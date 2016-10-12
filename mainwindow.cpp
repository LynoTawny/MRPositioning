#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QDebug>
#include <QWebFrame>
#include <QFileDialog>
#include <stdio.h>
#include <position_cal/field_func.h>
#include <QtMath>
#include <QNetworkProxy>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

const double EARTH_RADIUS = 6371.004;//地球半径

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::HttpProxy);//设置类型
    proxy.setHostName("119.6.136.122");//设置代理服务器地址
    proxy.setPort(80);//设置端口
//    proxy.setUser("xxx");//设置用户名,可以不填写
//    proxy.setPassword("xxx");//设置，可以不填写
//    QNetworkProxy::setApplicationProxy(proxy);

    ui->webView->setUrl(QUrl("qrc:/html/map.html"));

    qDebug() << getNetIP();

    //BaseDBHandler * dbHandler = new BaseDBHandler;

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_clearBtn_clicked()
{
    QWebFrame * webFrame = ui->webView->page()->mainFrame();
    webFrame->evaluateJavaScript(QString("clear()"));
}

void MainWindow::on_drTstRsltBtn_clicked()
{
    QString drTstFilePath = QFileDialog::getOpenFileName(this, tr("Open File"), "D:/MR Sample/DriveTest");
    qDebug() << drTstFilePath;
    QFile file(drTstFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "fail to open drive test file.";
        return;
    }

    QTextStream in(&file);
    QString line = in.readLine();
    int n = 1;
    while(!line.isEmpty())
    {
        DriveTestItem *item = new DriveTestItem(n, line);
        itemList.append(item);
        line = in.readLine();
        n++;

//        break;
    }

    qDebug() << "LINE:" << __LINE__ << "; read drive test file done.";
}

void MainWindow::on_showBasePosBtn_clicked()
{
    QString points;
    foreach (DriveTestItem *item, itemList)
    {
        if(!item->isBaseInfoReady())
        {
            item->queryBaseInformation();
        }

        QList<base_info_t> & base_infos = item->getBaseInfos();

        //画基站位置
        foreach (base_info_t base, base_infos)
        {
            points.append(QString::number(base.lng, 'g', 9));
            points.append(",");
            points.append(QString::number(base.lat, 'g', 9));
            points.append("|");
        }
    }

    QWebFrame * webFrame = ui->webView->page()->mainFrame();
    webFrame->evaluateJavaScript(QString("showTheLocaltion(\"%1\",0)").arg(points));

    //qDebug() << QString("showTheLocaltion(\"%1\",0)").arg(points);
}

void MainWindow::on_apiPosBtn_clicked()
{
    QString points;
    foreach (DriveTestItem *item, itemList)
    {
        if(!item->isAPIPosReady())
        {
            item->multiBasePositioning();
            if(!item->isAPIPosReady())
            {
                qDebug() << "LINE:" << __LINE__ << "; multi base positioning failed.";
                return;
            }
        }

        double lat, lng, radius;
        //xml_data_t *p_data = item->get_xml_data();
        //QList<base_meas_t> & result = item->getResults();
        //调接口 TODO

        //画基站位置
//        foreach (base_info_t base, item->base_infos)
//        {
//            basePoints.append(QString::number(base.lng, 'g', 9));
//            basePoints.append(",");
//            basePoints.append(QString::number(base.lat, 'g', 9));
//            basePoints.append("|");
//        }

        item->getApiPositioningResult(&lng, &lat, &radius);
        //qDebug("LINE:%d; lng:%.9g; lat:%.9g;", __LINE__, lng, lat);
        points.append(QString::number(lng, 'g', 9));
        points.append(",");
        points.append(QString::number(lat, 'g', 9));
        points.append("|");
    }

    QWebFrame * webFrame = ui->webView->page()->mainFrame();
    webFrame->evaluateJavaScript(QString("showTheLocaltion(\"%1\",0)").arg(points));

    on_truePosBtn_clicked();

    //qDebug() << QString("showTheLocaltion(\"%1\",0)").arg(points);
}

void MainWindow::on_ourPosBtn_clicked()
{
    QString points;

    foreach (DriveTestItem *item, itemList)
    {
        if(item->isOurPosReady()!= true)
        {
            QList<base_info_t> &base_infos = item->getBaseInfos();
            int count = base_infos.count();
            for(int i = 0; i < count; i++)
            {
                base_info_t base_info = base_infos.at(i);
                convertLonLat2XY(base_info.lng, base_info.lat, &base_info.pos_x, &base_info.pos_y);
                base_infos.replace(i, base_info);
            }

            xml_data_t *p_data = item->get_xml_data();
            ms_pos_t ms_pos;
            get_ms_pos(p_data, &ms_pos, 0);

            double lng, lat;
            convertXY2LonLat(ms_pos.pos_x, ms_pos.pos_y, &lng, &lat);
            item->setOurPositioningResult(lng, lat, ms_pos.pos_x, ms_pos.pos_y);
        }

        double lng, lat, x, y;
        item->getOurPositioningResult(&lng,&lat, &x, &y);

        points.append(QString::number(lng, 'g', 9));
        points.append(",");
        points.append(QString::number(lat, 'g', 9));
        points.append("|");
    }

    QWebFrame * webFrame = ui->webView->page()->mainFrame();
    webFrame->evaluateJavaScript(QString("showTheLocaltion(\"%1\",0)").arg(points));

    on_truePosBtn_clicked();
}

void MainWindow::on_truePosBtn_clicked()
{
    QString points;
    foreach (DriveTestItem *item, itemList)
    {
        double lat, lng;

        item->getTrueBD09Coord(&lng, &lat);
        //qDebug("LINE:%d; lng:%.9g; lat:%.9g;", __LINE__, lng, lat);
        points.append(QString::number(lng, 'g', 9));
        points.append(",");
        points.append(QString::number(lat, 'g', 9));
        points.append("|");
    }

    QWebFrame * webFrame = ui->webView->page()->mainFrame();
    webFrame->evaluateJavaScript(QString("showTheLocaltion(\"%1\",1)").arg(points));

}

double MainWindow::GetDistance(double lng1, double lat1, double lng2, double lat2)
{
    double radLat1 = qDegreesToRadians(lat1);
    double radLat2 = qDegreesToRadians(lat2);
    double a = radLat1 - radLat2;
    double b = qDegreesToRadians(lng1) - qDegreesToRadians(lng2);

    double s = 2 * qAsin(qSqrt(qPow(qSin(a/2),2) +
    qCos(radLat1) * qCos(radLat2) * qPow(qSin(b/2),2)));
    s = s * EARTH_RADIUS;

    return s;
}

double MainWindow::GetDistanceByXY(double x1, double y1, double x2, double y2)
{
    return qSqrt(qPow(x1-x2, 2) + qPow(y1-y2, 2));
}

void MainWindow::on_apiVsTrueBtn_clicked()
{
    //    double tst = GetDistance(114.430557,30.461733,114.430557,30.461241);
    //    qDebug("114.430557,30.461733|114.430557,30.461241 distance: %.9g", tst);

    QString str;
    ui->textBrowser->append("******************************************");
    str.append("i|true lng,true lat|api lng,api lat,api rad|distance");
    ui->textBrowser->append(str);

    int i = 1;
    foreach (DriveTestItem *item, itemList  )
    {
        str.clear();

        double true_lat, true_lng;
        double api_lat, api_lng, api_rad;
        item->getTrueBD09Coord(&true_lng, &true_lat);
        item->getApiPositioningResult(&api_lng, &api_lat, &api_rad);

        str.append(QString("%1|").arg(i));

        str.append(QString::number(true_lng, 'g', 9));
        str.append(",");
        str.append(QString::number(true_lat, 'g', 9));
        str.append("|");

        str.append(QString::number(api_lng, 'g', 9));
        str.append(",");
        str.append(QString::number(api_lat, 'g', 9));
        str.append(",");
        str.append(QString::number(api_rad, 'g', 9));
        str.append("|");

        double distance = GetDistance(true_lng, true_lat, api_lng, api_lat);
        str.append(QString::number(distance, 'g', 9));

        ui->textBrowser->append(str);
        i++;
    }
    ui->textBrowser->append("******************************************");
}

QString MainWindow::getNetIP(void)
{
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    QNetworkReply *reply = manager->get(QNetworkRequest(QUrl("http://whois.pconline.com.cn/")));
    QByteArray responseData;
    QEventLoop eventLoop;
    QObject::connect(manager, SIGNAL(finished(QNetworkReply *)), &eventLoop, SLOT(quit()));
    eventLoop.exec();
    responseData = reply->readAll();

    QString responseStr(responseData);

    QString web = responseStr.replace(" ", "");
    web = web.replace("\r", "");
    web = web.replace("\n", "");
    QStringList list = web.split("<br/>");
    QString tar = list[3];
    QStringList ip = tar.split("=");

    reply->deleteLater();
    manager->deleteLater();

    return ip[1];
}

//void MainWindow::convertLonLat2XY(base_info_t * base_info)
void MainWindow::convertLonLat2XY(double lng, double lat, double *pos_x, double *pos_y)
{
//    QString str = QString("%1,%2").arg(base_info->lng, 0, 'g', 9).arg(base_info->lat, 0, 'g', 9);
//    qDebug() << "FILE:" << __FILE__ << ";LINE:" << __LINE__ << "|" << str;
    QString str = QString("%1,%2").arg(lng, 0, 'g', 9).arg(lat, 0, 'g', 9);

    QWebFrame * webFrame = ui->webView->page()->mainFrame();
    QVariant ret = webFrame->evaluateJavaScript(QString("toFixPoint(\"%1\",0)").arg(str));
//    qDebug() << "FILE:" << __FILE__ << ";LINE:" << __LINE__ << "|" << ret.toString();

    QStringList strList = ret.toString().split(',');
    *pos_x = strList.at(0).toDouble();
    *pos_y = strList.at(1).toDouble();
//    qDebug("%.15g : %.15g", base_info->pos_x, base_info->pos_y);
}

//void MainWindow::convertXY2LonLat(ms_pos_t &ms_pos, double *lng, double *lat)
void MainWindow::convertXY2LonLat(double pos_x, double pos_y, double *lng, double *lat)
{
    QString str = QString("%1,%2").arg(pos_x, 0, 'g', 12).arg(pos_y, 0, 'g', 12);
//  qDebug() << "FILE:" << __FILE__ << ";LINE:" << __LINE__ << "|" << str;

    QWebFrame * webFrame = ui->webView->page()->mainFrame();
    QVariant ret = webFrame->evaluateJavaScript(QString("toFixPoint(\"%1\",1)").arg(str));

    QStringList strList = ret.toString().split(',');
    *lng = strList.at(0).toDouble();
    *lat = strList.at(1).toDouble();

//  qDebug("%.15g : %.15g", *lng, *lat);
}

void MainWindow::on_outVsTrueBtn_clicked()
{
    QString str;
    ui->textBrowser->append("******************************************");
    str.append("i|true lng,true lat|our lng,our lat|distance");
    ui->textBrowser->append(str);

    int i = 1;
    foreach (DriveTestItem *item, itemList  )
    {
        str.clear();

        double true_lat, true_lng;
        double true_x, true_y;
        double our_lat, our_lng, our_x, our_y;
        item->getTrueBD09Coord(&true_lng, &true_lat);
        item->getOurPositioningResult(&our_lng, &our_lat, &our_x, &our_y);
        convertLonLat2XY(true_lng, true_lat, &true_x, &true_y);

        str.append(QString("%1|").arg(i));

        str.append(QString::number(true_lng, 'g', 9));
        str.append(",");
        str.append(QString::number(true_lat, 'g', 9));
        str.append("|");

        str.append(QString::number(our_lng, 'g', 9));
        str.append(",");
        str.append(QString::number(our_lat, 'g', 9));
        str.append("|");

//        double distance = GetDistance(true_lng, true_lat, our_lng, our_lat);
        double distance = GetDistanceByXY(our_x, our_y, true_x, true_y);
        str.append(QString::number(distance, 'g', 9));

        ui->textBrowser->append(str);
        i++;
    }
    ui->textBrowser->append("******************************************");
}

void MainWindow::on_preprocessBtn_clicked()
{
    rawDataRead();
    rawDataFiltrate();
    rawDataFill();
    rawDataOutputNewFile();
}

void MainWindow::rawDataFill(void)
{

}

void MainWindow::rawDataRead(void)
{
    QString rawDataFilePath = QFileDialog::getOpenFileName(this, tr("Open File"), "D:/MR Sample/DriveTest");

    QFile file(rawDataFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "fail to open raw dta file.";
        return;
    }

    QTextStream in(&file);
    QString line = in.readLine();
    int no = 1;
    while(!line.isEmpty())
    {
        QStringList strList = line.split('\t', QString::SkipEmptyParts);
        int cellCnt = (strList.count() - 1) / 5;
        meas_point_raw_data_t * p_data = (meas_point_raw_data_t *)malloc(sizeof(meas_point_raw_data_t) +
                                                                     cellCnt * sizeof(cell_raw_data_t));
        p_data->no = no;
        QString time = strList.at(0).remove('"');
        strncpy(p_data->time, time.toLatin1().data(), TIME_BUF_LEN - 1);
        p_data->cell_count = cellCnt;
        for(int i = 0; i < cellCnt; i++)
        {
            int *tmp = (int *)&p_data->cell_list[i];
            for(int j = 0; j < 5; j++)
            {
                QString s_value = strList.at(5*i + j + 1);
                s_value.remove('"');
                int value = s_value.isEmpty() ? -1 : s_value.toInt();
                *tmp ++ = value;

//                qDebug() << "line:" << no << "; value:" << value;
            }
        }

        measPointList.append(p_data);
        line = in.readLine();
        no ++;

//        if(no > 50) break;
    }

    file.close();
    qDebug() << "LINE:" << __LINE__ << "; read raw data file done.";

//    meas_point_raw_data_t *test = measPointList.last();
//    qDebug() << "line:" << test->no;
//    for(int i = 0; i < test->cell_count; i++)
//    {
//        qDebug() << "cell" << i << ":" << test->cell_list[i].lac << ","
//                 << test->cell_list[i].ci << ","
//                 << test->cell_list[i].arfcn << ","
//                 << test->cell_list[i].bsic << ","
//                 << test->cell_list[i].rxlev;
//    }
}

bool isCellRawDataValid(cell_raw_data_t *p)
{
    if(-1 == p->rxlev)
    {
        return false;
    }

    if(((-1 == p->lac) || (-1 == p->ci)) && ((-1 == p->arfcn) || (-1 == p->bsic)))
    {
        return false;
    }

    return true;
}

bool isCellRawDataCorrect(cell_raw_data_t *p)
{
    int *tmp = (int *)p;
    for(int i = 0; i < sizeof(cell_raw_data_t)/sizeof(int); i++)
    {
        if(*tmp ++ == -1)
        {
            return false;
        }
    }

    return true;
}

void MainWindow::rawDataFiltrate(void)
{
    for(QList<meas_point_raw_data_t *>::iterator it = measPointList.begin();
        it != measPointList.end(); it++)
    {
        meas_point_raw_data_t *p = *it;

        //如果该测量点主服务小区数据残缺，扔掉该测量点
//        if(!isCellRawDataValid(&p->cell_list[0]))
        if(!isCellRawDataCorrect(&p->cell_list[0]))
        {
            free(p);
            measPointList.erase(it);
            continue;
        }

        //先保存测量点中有效的基站测量结果到vector
        QVector<cell_raw_data_t> vector;
        vector.append(p->cell_list[0]);
        for(int i = 1; i < p->cell_count; i++)
        {
//            if(isCellRawDataValid(&p->cell_list[i]))
            if(isCellRawDataCorrect(&p->cell_list[i]))
            {
                vector.append(p->cell_list[i]);
            }
        }

        //测量点中基站数目不够，删掉测量点
        if(vector.count() < 3)
        {
            free(p);
            measPointList.erase(it);
            continue;
        }

        //分配空间保存有效的基站测量结果
        meas_point_raw_data_t * p_data = (meas_point_raw_data_t *)malloc(sizeof(meas_point_raw_data_t) +
                                                                     vector.count() * sizeof(cell_raw_data_t));
        p_data->no = p->no;
        strncpy(p_data->time, p->time, TIME_BUF_LEN - 1);
        p_data->cell_count = vector.count();
        for(int i = 0; i < vector.count(); i++)
        {
            p_data->cell_list[i] = vector.at(i);
        }

        //替换链表中的元素
        *it = p_data;
        free(p);
    }
}

void MainWindow::rawDataOutputNewFile(void)
{
    QFile file(QString("D:/MR Sample/DriveTest/PreprocessOK.txt"));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "fail to open write only file.";
        return;
    }

    QTextStream out(&file);
    out << "preprocess complete\n";
    for(QList<meas_point_raw_data_t *>::iterator it = measPointList.begin();
        it != measPointList.end(); it ++)
    {
        meas_point_raw_data_t *p = *it;
        QString str;

        str.append(QString("%1|").arg(p->no));
        str.append(QString(p->time));
        str.append("|");
        for(int i = 0; i < p->cell_count; i++)
        {
            str.append(QString("%1,").arg(p->cell_list[i].lac));
            str.append(QString("%1,").arg(p->cell_list[i].ci));
            str.append(QString("%1,").arg(p->cell_list[i].arfcn));
            str.append(QString("%1,").arg(p->cell_list[i].bsic));
            str.append(QString("%1|").arg(p->cell_list[i].rxlev));
        }
        str.chop(1);
        str.append("\n");
        out << str;
    }

    file.close();
    qDebug() << "write file done";
}



void MainWindow::rawDataFinalyCheck(void)
{
}
