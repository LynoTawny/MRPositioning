#include "drivetestitem.h"
#include <QDebug>
#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QApplication>

#define AOA_TO_DEGREE(aoa)         ((double)aoa / 2 + 0.25)  //AOA上报值转角度

//TADV上报值对应ts单位的值
const int tadv_ts[45] = {   8, 24, 40, 56, 72, 88, 104, 120, 136, 152, 168, 184,        //TADV 0~11
                            208, 240, 272, 304, 336, 368, 400, 432, 464, 496, 528, 560, //TADV 12~23
                            592, 624, 656, 688, 720, 752, 784, 816, 848, 880, 912, 944, //TADV 24~35
                            976, 1008, 1152, 1408, 1664, 1920, 2560, 3584, 4096};       //TADV 36~44
#define METER_PER_TS                (4.89)  //一个ts相当于4.89米
#define TADV_TO_METER(tadv)         (METER_PER_TS * tadv_ts[(tadv>44)? 44 : tadv])//TADV上报值转距离，单位米
#define RSRP_TO_DBM(rsrp)           ((double)rsrp - 140 - 0.5)//RSRP上报值转dbm
#define EARFCN_TO_MHZ(earfcn)       ((double)(earfcn - 38250)/10 + 1880)//lte频点转MHz
#define ARFCN_TO_MHZ(arfcn)         ((arfcn > 124) ? (1805+0.2*(arfcn-511)) : (935+0.2*arfcn))//GSM频点转MHz
#define UARFCN_TO_MHZ(uarfcn)       ((double)uarfcn/5)//3g频点计算

const int TIMEOUT = (30 * 1000);

DriveTestItem::DriveTestItem(int line, QString &lineStr)
{
    this->p_xml_data = NULL;

    QStringList strList = lineStr.split('|', QString::SkipEmptyParts);
    this->time = strList.at(0);

    QString truePosStr = strList.at(1);
    QStringList truePosList = truePosStr.split(',', QString::SkipEmptyParts);
    this->true_wgs84_lat_str = truePosList.at(1);
    this->true_wgs84_lat = this->true_wgs84_lat_str.toDouble();
    this->true_wgs84_lng_str = truePosList.at(0);
    this->true_wgs84_lng = this->true_wgs84_lng_str.toDouble();
//    this->true_bd09_lat = this->true_wgs84_lat;
//    this->true_bd09_lng = this->true_wgs84_lng;

    this->line = line;
    this->result_cnt = strList.count() - 2;

    qDebug("%.15lg,%.15lg", this->true_wgs84_lat, this->true_wgs84_lng);
    for(int i = 0; i < this->result_cnt; i++)
    {
        QString str = strList.at(i + 2);
        base_meas_t result;
        for(int j = 0; j < 4; j++)
        {
            result.buf[j] = str.section(',', j, j).toInt();
        }
        result.buf[4] = str.section(',', 4, 4).toFloat();
        results.append(result);
        //qDebug() << str.section(',', 4, 4) << "|" << result.buf[4] << result.base_meas_rslt.rxlev;
    }

    // start post方式调用百度api，异步，wgs84坐标转换为bd09坐标
    QByteArray postArray;
    this->postStr = QString("coords=%1,%2").arg(this->true_wgs84_lng_str).arg(this->true_wgs84_lat_str);
    this->postStr.append("&from=1&to=5&ak=ojST3jwr9LGTWTIwxak4z6hkE11N6yMx");
    postArray.append(this->postStr);

    this->coord_trans_manager = new QNetworkAccessManager(this);
    connect(coord_trans_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(coordTransformFinishedSlot(QNetworkReply*)));

    QNetworkRequest request(QUrl("http://api.map.baidu.com/geoconv/v1/"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
    request.setHeader(QNetworkRequest::ContentLengthHeader,postArray.size());
    this->coord_trans_manager->post(request, postArray);
    //end

    // start mcc+mnc+lac+ci查询基站信息
    this->base_query_manager = new QNetworkAccessManager(this);
    //queryBaseInformation();//同步查询一个测量点的多个基站
    //end

    //start 调用api，多基站定位
    this->positioning_manager = new QNetworkAccessManager(this);
    //multiBasePositioning();
    //end

    baseInfoReadyFlag = false;
    apiPosReadyFlag = false;
    ourPosReadyFlag = false;
    isTruePosValid = false;
}

DriveTestItem::~DriveTestItem()
{
    delete this->coord_trans_manager;
    delete this->base_query_manager;
    delete this->positioning_manager;

    if(p_xml_data)
        free(p_xml_data);
}

void DriveTestItem::coordTransformFinishedSlot(QNetworkReply *reply)
{
    QByteArray result;

    if(reply->error() == QNetworkReply::NoError)
    {
        result = reply->readAll();
    }

    reply->deleteLater();

    //qDebug() << result;
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(result, &json_error);
    if(json_error.error == QJsonParseError::NoError)
    {
        if(parse_doucment.isObject())
        {
            QJsonObject obj = parse_doucment.object();
            if(obj.contains("status"))
            {
                QJsonValue status_value = obj.take("status");
                if(status_value.isDouble())
                {
                    if(0 != status_value.toVariant().toInt())
                    {
                        qDebug() << "FILE " << __FILE__ << "; LINE " << __LINE__ << "; Status error";
                        return;
                    }
                }
            }

            if(obj.contains("result"))
            {
                QJsonValue result_value = obj.take("result");
                if(result_value.isArray())
                {
                    QJsonArray result_array = result_value.toArray();
                    QJsonValue array_item = result_array.at(0);
                    if(array_item.isObject())
                    {
                        QJsonObject result_obj = array_item.toObject();
                        QJsonValue x_value = result_obj.take("x");
                        QJsonValue y_value = result_obj.take("y");
                        this->true_bd09_lng = x_value.toVariant().toDouble();
                        this->true_bd09_lat = y_value.toVariant().toDouble();
                        this->isTruePosValid = true;
                        //qDebug("LINE:%d %.15lg,%.15lg",__LINE__, this->true_bd09_lat, this->true_bd09_lng);
                    }
                }
                else
                {
                    qDebug() << "FILE " << __FILE__ << "; LINE " << __LINE__ << "; No Result Array";
                }
            }
        }
    }
    else
    {
        qDebug() << "FILE " << __FILE__ << "; LINE " << __LINE__ <<"; Parse json failed";
    }
}

void DriveTestItem::queryBaseInfoFinishedSlot(QNetworkReply *reply)
{
    QByteArray result;

    if(reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "FILE:" << __FILE__  << "; LINE:" << __LINE__ << "; network error code:" << reply->errorString();
        return;
    }

    result = reply->readAll();
    reply->deleteLater();

//    qDebug() << result;

    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(result, &json_error);
    if(json_error.error == QJsonParseError::NoError)
    {
        if(parse_doucment.isObject())
        {
            QJsonObject obj = parse_doucment.object();
            if(obj.contains("errcode"))
            {
                QJsonValue errcode_value = obj.take("errcode");
                if(errcode_value.isDouble())
                {
                    if(0 != errcode_value.toVariant().toInt())
                    {
                        qDebug() << "FILE " << __FILE__ << "; LINE " << __LINE__ << "; errcode error";
                        return;
                    }
                }
            }

            QJsonValue lat_value = obj.take("lat");
            QJsonValue lon_value = obj.take("lon");
            QJsonValue radius_value = obj.take("radius");

            base_info_t base_info;
            base_info.lat = lat_value.toVariant().toString().toDouble();
            base_info.lng = lon_value.toVariant().toString().toDouble();
            base_info.radius = radius_value.toVariant().toString().toDouble();

            //qDebug() << "LINE" << __LINE__ << " : " << base_info.lat << ":" << base_info.lng << ":" << base_info.radius;
            base_infos.append(base_info);
        }
    }
}

xml_data_t *DriveTestItem::get_xml_data(void)
{
    p_xml_data = (xml_data_t *)malloc(sizeof(xml_data_t) + this->result_cnt * sizeof(bs_data_t));

    for(int i = 0; i < result_cnt; i++)
    {
        p_xml_data->bs_data[i].freq = ARFCN_TO_MHZ(results.at(i).base_meas_rslt.arfcn);
        p_xml_data->bs_data[i].pos_y = base_infos.at(i).pos_y;
        p_xml_data->bs_data[i].pos_x = base_infos.at(i).pos_x;
        p_xml_data->bs_data[i].rxlevel = results.at(i).base_meas_rslt.rxlev;

//        qDebug() << "LINE:" << __LINE__ << "; "
//                 << "i:" << i << "; freq:" << p_xml_data->bs_data[i].freq
//                 << "; rxlev:" << p_xml_data->bs_data[i].rxlevel
//                 << "; lat:" << p_xml_data->bs_data[i].latitude
//                 << "; lng:" << p_xml_data->bs_data[i].longitude;
    }

    p_xml_data->bs_num = this->result_cnt;
    p_xml_data->aoa = -1;
    p_xml_data->toa = -1;
//    p_xml_data->true_lat = this->true_bd09_lat;
//    p_xml_data->true_lng = this->true_bd09_lng;

    return p_xml_data;

}

void DriveTestItem::queryBaseInformation(void)
{
    QNetworkReply *reply;
    for(int i = 0; i < this->result_cnt; i++)
    {
        int mcc = 460;
        int mnc = 1;
        int lac = this->results.at(i).base_meas_rslt.lac;
        int ci = this->results.at(i).base_meas_rslt.ci;
        double lng, lat;
        if(dbHandler->queryBaseInfo(mcc,mnc,lac,ci,&lng,&lat,NULL))
        {
            base_info_t base_info;
            base_info.lat = lat;
            base_info.lng = lng;
            base_info.radius = -1;

            base_infos.append(base_info);

            qDebug() << "FILE:" << __FILE__ << "; LINE:" << __LINE__ << "; query data base success.";
            continue;
        }

        QString urlStr;
        urlStr.append("http://api.cellocation.com/cell/");
        urlStr.append(QString("?mcc=%1").arg("460"));
        urlStr.append(QString("&mnc=%1").arg("1"));
        urlStr.append(QString("&lac=%1").arg(lac));
        urlStr.append(QString("&ci=%1").arg(ci));
        urlStr.append(QString("&coord=%1&output=json").arg("bd09"));

//        qDebug() << urlStr;

        QNetworkRequest query_base_info(QUrl(urlStr.toLatin1()));
        reply = this->base_query_manager->get(query_base_info);

        qDebug() << "FILE:" << __FILE__ << "; LINE:" << __LINE__ << "; waiting reply.";
        QEventLoop eventLoop;
        QObject::connect(this->base_query_manager, SIGNAL(finished(QNetworkReply *)), &eventLoop, SLOT(quit()));
        eventLoop.exec();

        queryBaseInfoFinishedSlot(reply);
        qDebug() << "FILE:" << __FILE__ << "; LINE:" << __LINE__ << "; query base info done.";

        base_info_t tmp = base_infos.last();
        BaseInfo base;
        base.setBaseID(mcc, mnc, lac, ci);
        QString type("bd09");
        base.setBasePos(tmp.lng, tmp.lat, type);
        dbHandler->insertBaseInfo(base);
//        break;
    }

    baseInfoReadyFlag = true;
}

QList<base_meas_t> & DriveTestItem::getResults(void)
{
    return this->results;
}

QList<base_info_t> & DriveTestItem::getBaseInfos(void)
{
    return this->base_infos;
}

void DriveTestItem::getTrueWGS84Coord(double *p_lng, double *p_lat)
{
    *p_lng = this->true_wgs84_lng;
    *p_lat = this->true_wgs84_lat;
}

void DriveTestItem::getTrueBD09Coord(double *p_lng, double *p_lat)
{
    *p_lng = this->true_bd09_lng;
    *p_lat = this->true_bd09_lat;
}

void DriveTestItem::getApiPositioningResult(double *p_lng, double *p_lat, double *p_radius)
{
    *p_lng = this->api_lng;
    *p_lat = this->api_lat;
    *p_radius = this->api_radius;
}

void DriveTestItem::multiBasePositioning(void)
{
    QNetworkReply *reply;
    QString urlStr;
    urlStr.append("http://api.cellocation.com/loc/?");
    urlStr.append("cl=");
    for(int i = 0; i < this->result_cnt; i++)
    {
        urlStr.append(QString("460,1,%1,%2,%3;").\
                      arg(results.at(i).base_meas_rslt.lac).\
                      arg(results.at(i).base_meas_rslt.ci).\
                      arg(results.at(i).base_meas_rslt.rxlev));
    }
    urlStr.chop(1);
    urlStr.append("&coord=bd09&output=json");

    //qDebug() << "LINE:" << __LINE__ << " : " << urlStr;

    QNetworkRequest query_pos(QUrl(urlStr.toLatin1()));
    reply = this->positioning_manager->get(query_pos);

    qDebug() << "LINE:" << __LINE__ << "; waiting reply.";

    QEventLoop eventLoop;
    QObject::connect(this->positioning_manager, SIGNAL(finished(QNetworkReply *)), &eventLoop, SLOT(quit()));
    eventLoop.exec();

    QByteArray result;

    if(reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "LINE:" << __LINE__ << "; network error code:" << reply->errorString();
        return;
    }

    result = reply->readAll();
    reply->deleteLater();

//    qDebug() << "LINE:" << __LINE__ << " : " << result;

    /*
        {   "errcode":0,
            "lat":"30.46433",
            "lon":"114.433728",
            "radius":"100",
            "address":"湖北省武汉市江夏区流芳街道当代国际花园C座;金融港中路与金融港二路路口东280米"}
    */
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(result, &json_error);
    if(json_error.error == QJsonParseError::NoError)
    {
        if(parse_doucment.isObject())
        {
            QJsonObject obj = parse_doucment.object();
            if(obj.contains("errcode"))
            {
                QJsonValue errcode_value = obj.take("errcode");
                if(errcode_value.isDouble())
                {
                    if(0 != errcode_value.toVariant().toInt())
                    {
                        qDebug() << "FILE " << __FILE__ << "; LINE " << __LINE__ << "; errcode error";
                        return;
                    }
                }
            }

            QJsonValue lat_value = obj.take("lat");
            QJsonValue lon_value = obj.take("lon");
            QJsonValue radius_value = obj.take("radius");

            this->api_lat = lat_value.toVariant().toString().toDouble();
            this->api_lng = lon_value.toVariant().toString().toDouble();
            //this->api_radius = radius_value.toVariant().toString().toDouble();

            apiPosReadyFlag = true;
            //qDebug("LINE:%d; lng:%.6g; lat:%.6g; radius:%.6g", __LINE__, this->api_lng, this->api_lat, this->api_radius);
            qDebug() << "LINE:" << __LINE__ << "; multi base positioning done.";
        }
    }
}

bool DriveTestItem::isAPIPosReady(void)
{
    return this->apiPosReadyFlag;
}

bool DriveTestItem::isBaseInfoReady(void)
{
    return this->baseInfoReadyFlag;
}

bool DriveTestItem::isOurPosReady(void)
{
    return this->ourPosReadyFlag;
}

void DriveTestItem::setOurPositioningResult(double lng, double lat, double x, double y)
{
    this->our_lat = lat;
    this->our_lng = lng;
    this->our_pos_x = x;
    this->our_pos_y = y;
    this->ourPosReadyFlag = true;
}

void DriveTestItem::getOurPositioningResult(double *lng, double *lat, double *x, double *y)
{
    *lng = this->our_lng;
    *lat = this->our_lat;
    *x = this->our_pos_x;
    *y = this->our_pos_y;
}

void DriveTestItem::setBaseDBHandler(BaseDBHandler *handler)
{
    this->dbHandler = handler;
}

void DriveTestItem::setBaseRadius(int i, double radius)
{
    base_info_t base = this->base_infos.at(i);
    base.radius = radius;
    base_infos[i] = base;
}
