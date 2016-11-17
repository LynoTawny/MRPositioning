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
DriveTestItem * curItem = NULL;

extern int    pos_val;
extern double prev_x;
extern double prev_y;

void initPositionCal(void)
{
    curItem = NULL;
    pos_val = 0;
    prev_x = 0;
    prev_y = 0;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //设置代理服务器
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::HttpProxy);//设置类型
    proxy.setHostName("127.0.0.1");//设置代理服务器地址
    proxy.setPort(8087);//设置端口
    //QNetworkProxy::setApplicationProxy(proxy);
    qDebug() << getNetIP();

    ui->webView->setUrl(QUrl("qrc:/html/map.html"));
    QWebPage *page = ui->webView->page();
    connect(page, SIGNAL(loadStarted()), this, SLOT(doLoad()));


    //QString dbPath("D:/MR Sample/DriveTest/base.db");
    QString dbPath("./base.db");
    this->dbHandler = new BaseDBHandler(dbPath);
    this->dbHandler->openBaseDB();

    ui->ourPosBtn->setEnabled(false);
    ui->outVsTrueBtn->setEnabled(false);

//    ui->preprocessBtn->hide();
//    ui->clearBtn->hide();

    this->statusLabel = new QLabel;
    this->statusLabel->setMinimumSize(this->statusLabel->sizeHint());
    this->statusLabel->setAlignment(Qt::AlignHCenter);
    this->statusLabel->setText(tr("点击\"打开路测\"，选择文件"));
    ui->statusBar->addWidget(this->statusLabel);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_clearBtn_clicked()
{
    QWebFrame * webFrame = ui->webView->page()->mainFrame();
    webFrame->evaluateJavaScript(QString("clearChoosedBases()"));
}

void MainWindow::on_drTstRsltBtn_clicked()
{
    initPositionCal();
    QWebFrame * webFrame = ui->webView->page()->mainFrame();
    webFrame->evaluateJavaScript(QString("clear()"));
    itemList.clear();

    QString drTstFilePath = QFileDialog::getOpenFileName(this, tr("Open File"), "D:/MR Sample/DriveTest");
    //QString drTstFilePath = QFileDialog::getOpenFileName(this, tr("Open File"), "./DriveTest");
    qDebug() << drTstFilePath;
    QFile file(drTstFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "fail to open drive test file.";
        return;
    }
    this->statusLabel->setText(tr("读文件中。。。"));

    QTextStream in(&file);
    QString line = in.readLine();
    int n = 1;
    while(!line.isEmpty())
    {
        qDebug() << "LINE:" << __LINE__ << "num" << n;
        DriveTestItem *item = new DriveTestItem(n, line);
        item->setBaseDBHandler(this->dbHandler);
        itemList.append(item);
        line = in.readLine();
        n++;

//        QList<base_meas_t> &results = item->getResults();
//        foreach (base_meas_t meas, results)
//        {
//            qDebug() << "LINE:" << __LINE__ << "|"
//                     << meas.base_meas_rslt.lac << ","
//                     << meas.base_meas_rslt.ci << ","
//                     << meas.base_meas_rslt.arfcn << ","
//                     << meas.base_meas_rslt.bsic << ","
//                     << meas.base_meas_rslt.rxlev;
//        }
//        if(n > 50)
//            break;
    }

    qDebug() << "LINE:" << __LINE__ << "; read drive test file done.";

//    QWebFrame * webFrame = ui->webView->page()->mainFrame();
//    webFrame->evaluateJavaScript(QString("showTheLocaltion(\"114.426,34.527|117.554,36.446|119.852,30.554\",1);"));

    this->statusLabel->setText(tr("读文件完成！"));
    on_showBasePosBtn_clicked();
    ui->ourPosBtn->setEnabled(true);
}

typedef struct
{
    base_info_t base_info;//基站信息
    int lac;
    int ci;
    QVector<int> meas_point_set;//保存接收到该基站的测量点序号
}super_base_info_t;

void MainWindow::on_showBasePosBtn_clicked()
{
    QMap<QString, super_base_info_t> bases;
    this->statusLabel->setText(tr("正在查询基站经纬度，稍后。。。"));

    int meas_point_index = 1;
    foreach (DriveTestItem *item, itemList) {

        if(!item->isBaseInfoReady())
        {
            item->queryBaseInformation();
        }

        QList<base_info_t> & base_infos = item->getBaseInfos();
        QList<base_meas_t> & results = item->getResults();

        int count = results.count();
        for(int i = 0; i < count; i++)
        {
            QString key;
            key.append(QString::number(results.at(i).base_meas_rslt.lac));
            key.append(",");
            key.append(QString::number(results.at(i).base_meas_rslt.ci));

            if(!bases.contains(key))
            {
                //如果Map中没有该基站
                super_base_info_t su_base_info;
                su_base_info.base_info = base_infos.at(i);
                su_base_info.meas_point_set.append(meas_point_index);

                base_meas_t result = results.at(i);
                su_base_info.ci = result.base_meas_rslt.ci;
                su_base_info.lac = result.base_meas_rslt.lac;
                bases.insert(key, su_base_info);
            }
            else
            {
                //如果Map中有该基站
                super_base_info_t su_base_info = bases[key];
                su_base_info.meas_point_set.append(meas_point_index);
                bases[key] = su_base_info;
            }
        }

        meas_point_index ++;
    }
    this->statusLabel->setText(tr("查询基站位置完成！点击\"定位位置\"。"));

    //显示所有基站位置
    QString points;
    for(QMap<QString, super_base_info_t>::iterator it = bases.begin(); it != bases.end(); it++)
    {
        points.append(QString::number(it.value().base_info.lng, 'g', 9));
        points.append(",");
        points.append(QString::number(it.value().base_info.lat, 'g', 9));
        points.append(",");
        points.append(QString::number(it.value().lac));
        points.append(",");
        points.append(QString::number(it.value().ci));
        points.append(",");
        foreach (int index, it.value().meas_point_set) {
            points.append(QString::number(index));
            points.append("/");
        }
        points.chop(1);
        points.append("|");
    }
    points.chop(1);

    qDebug() << __FILE__ << __LINE__ << points;
    QString str = QString("showAllBaseLocation(\"%1\")").arg(points);
    QWebFrame * webFrame = ui->webView->page()->mainFrame();
    webFrame->evaluateJavaScript(str);
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
    points.chop(1);
    QWebFrame * webFrame = ui->webView->page()->mainFrame();
    webFrame->evaluateJavaScript(QString("showTheLocaltion(\"%1\",0)").arg(points));

    on_truePosBtn_clicked();

    //qDebug() << QString("showTheLocaltion(\"%1\",0)").arg(points);
}

void MainWindow::on_ourPosBtn_clicked()
{
    QString points;

    this->statusLabel->setText(tr("正在计算位置，稍等。。。"));
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
            curItem = item;
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
    points.chop(1);
    this->statusLabel->setText(tr("计算位置完成！"));

//    qDebug() << "FILE:" << __FILE__ << "LINE:" << __LINE__ << ":" << points;
    QWebFrame * webFrame = ui->webView->page()->mainFrame();
    webFrame->evaluateJavaScript(QString("showTheLocaltion(\"%1\",0)").arg(points));

    on_truePosBtn_clicked();

    ui->outVsTrueBtn->setEnabled(true);
}

void MainWindow::on_truePosBtn_clicked()
{
    QString points;
    int i = 1;
    foreach (DriveTestItem *item, itemList)
    {
        double lat, lng;

        if(item->isTruePosReady())
        {
            item->getTrueBD09Coord(&lng, &lat);
            //qDebug("LINE:%d; lng:%.9g; lat:%.9g;", __LINE__, lng, lat);
            points.append(QString::number(lng, 'g', 9));
            points.append(",");
            points.append(QString::number(lat, 'g', 9));
            points.append(",");
            points.append(QString::number(i));
            points.append("|");
        }
        i ++;
    }

    points.chop(1);
    QWebFrame * webFrame = ui->webView->page()->mainFrame();
    webFrame->evaluateJavaScript(QString("showTheLocaltion(\"%1\",1)").arg(points));
    //qDebug() << QString("showTheLocaltion(\"%1\",1)").arg(points);

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
    QString str;
    str.append("序号\t真实经度\t真实纬度\t定位经度\t定位纬度\t距离");
    ui->textBrowser->append(str);

    int i = 1;
    double sum = 0;
    foreach (DriveTestItem *item, itemList  )
    {
        str.clear();

        double true_lat, true_lng;
        double true_x, true_y;
        double api_lat, api_lng, api_rad;
        item->getTrueBD09Coord(&true_lng, &true_lat);
        item->getApiPositioningResult(&api_lng, &api_lat, &api_rad);
        convertLonLat2XY(true_lng, true_lat, &true_x, &true_y);

        str.append(QString("%1\t").arg(i));

        str.append(QString::number(true_lng, 'g', 9));
        str.append("\t");
        str.append(QString::number(true_lat, 'g', 9));
        str.append("\t");

        str.append(QString::number(api_lng, 'g', 9));
        str.append("\t");
        str.append(QString::number(api_lat, 'g', 9));
        str.append("\t");

//        str.append(QString::number(our_x, 'g', 9));
//        str.append("\t");
//        str.append(QString::number(our_y, 'g', 9));
//        str.append("\t");

//        double distance = GetDistance(true_lng, true_lat, our_lng, our_lat);
        double distance = GetDistance(true_lng, true_lat, api_lng, api_lat);
        str.append(QString::number(distance, 'g', 9));
        sum += distance;

        ui->textBrowser->append(str);
        i++;
    }
    ui->textBrowser->append("平均精度：" + QString::number(sum / (i - 1), 'g', 9));
    ui->textBrowser->append("**************************************************************************************");
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
    str.append("序号\t真实经度\t真实纬度\t定位经度\t定位纬度\t距离");
    ui->textBrowser->append(str);

    int i = 1;
    double sum = 0;
    foreach (DriveTestItem *item, itemList  )
    {
        str.clear();

        double true_lat, true_lng;
        double true_x, true_y;
        double our_lat, our_lng, our_x, our_y;
        item->getTrueBD09Coord(&true_lng, &true_lat);
        item->getOurPositioningResult(&our_lng, &our_lat, &our_x, &our_y);
        convertLonLat2XY(true_lng, true_lat, &true_x, &true_y);

        str.append(QString("%1\t").arg(i));

        str.append(QString::number(true_lng, 'g', 9));
        str.append("\t");
        str.append(QString::number(true_lat, 'g', 9));
        str.append("\t");

        str.append(QString::number(our_lng, 'g', 9));
        str.append("\t");
        str.append(QString::number(our_lat, 'g', 9));
        str.append("\t");

//        str.append(QString::number(our_x, 'g', 9));
//        str.append("\t");
//        str.append(QString::number(our_y, 'g', 9));
//        str.append("\t");

//        double distance = GetDistance(true_lng, true_lat, our_lng, our_lat);
        double distance = GetDistanceByXY(our_x, our_y, true_x, true_y);
        str.append(QString::number(distance, 'g', 9));
        sum += distance;

        ui->textBrowser->append(str);
        i++;
    }
    ui->textBrowser->append("平均精度：" + QString::number(sum / (i - 1), 'g', 9));
    ui->textBrowser->append("**************************************************************************************");

#if 0
    //打印基站xy
    int index_point = 1, index_cell = 1;
    foreach (DriveTestItem *item, itemList  )
    {
        QList<base_info_t> &base_infos = item->getBaseInfos();
        foreach (base_info_t base, base_infos) {
            str.clear();
            str.append(QString("%1|%2|").arg(index_point).arg(index_cell));

            str.append(QString::number(base.lng, 'g', 9));
            str.append(",");
            str.append(QString::number(base.lat, 'g', 9));
            str.append("|");

            str.append(QString::number(base.pos_x, 'g', 9));
            str.append(",");
            str.append(QString::number(base.pos_y, 'g', 9));
            str.append("|");
            ui->textBrowser->append(str);
            index_cell ++;
        }

        index_point ++;
        index_cell = 1;
    }
    ui->textBrowser->append("******************************************");
#endif
}

void MainWindow::on_preprocessBtn_clicked()
{
    QString rawDataFilePath = QFileDialog::getOpenFileName(this, tr("Open File"), "D:/MR Sample/DriveTest");
    if(rawDataFilePath.isEmpty())
    {
        qDebug() << __FILE__ << "LINE:" << __LINE__ << "no file selected.";
        return;
    }

    rawDataRead(rawDataFilePath);

    int oldCnt = 0;
    while(1)
    {
        rawDataFiltrate();
        if(oldCnt == measPointList.count())
        {
            break;
        }
        else
        {
            oldCnt = measPointList.count();
        }
    }
    rawDataFill();
    rawDataOutputNewFile(rawDataFilePath);
}


void MainWindow::rawDataRead(QString rawDataFilePath)
{
    QFile file(rawDataFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "fail to open raw dta file.";
        return;
    }

    QDate date = QDate::currentDate();
    QString dateStr = date.toString("yyyy-MM-dd");

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
        QString time = strList.at(0);
        time.remove('"');
        QString dateTime = dateStr + " " + time;

        strncpy(p_data->time, dateTime.toLatin1().data(), TIME_BUF_LEN - 1);
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
//    bool isPointValid = true;

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

//        for(int i = 0; i < p->cell_count; i++)
//        {
//            if(!isCellRawDataCorrect(&p->cell_list[i]))
//            {
//                isPointValid = false;
//                break;
//            }
//        }
//        if(!isPointValid)
//        {
//            free(p);
//            measPointList.erase(it);
//            continue;
//        }


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

void MainWindow::rawDataFill(void)
{
    for(QList<meas_point_raw_data_t *>::iterator it = measPointList.begin();
        it != measPointList.end(); it++)
    {
        meas_point_raw_data_t *p = *it;
        bool isPointValid = true;
        //删掉数据残缺的测量点
        for(int i = 0; i < p->cell_count; i++)
        {
            if(!isCellRawDataCorrect(&p->cell_list[i]))
            {
                isPointValid = false;
                break;
            }
        }
        if(!isPointValid)
        {
            free(p);
            measPointList.erase(it);
            continue;
        }


        //先保存测量点中有效的基站测量结果到vector
//        QVector<cell_raw_data_t> vector;
//        vector.append(p->cell_list[0]);
//        for(int i = 1; i < p->cell_count; i++)
//        {
//            if(isCellRawDataValid(&p->cell_list[i]))
//            if(isCellRawDataCorrect(&p->cell_list[i]))
//            {
//                vector.append(p->cell_list[i]);
//            }
//        }

//        //测量点中基站数目不够，删掉测量点
//        if(vector.count() < 3)
//        {
//            free(p);
//            measPointList.erase(it);
//            continue;
//        }

//        //分配空间保存有效的基站测量结果
//        meas_point_raw_data_t * p_data = (meas_point_raw_data_t *)malloc(sizeof(meas_point_raw_data_t) +
//                                                                     vector.count() * sizeof(cell_raw_data_t));
//        p_data->no = p->no;
//        strncpy(p_data->time, p->time, TIME_BUF_LEN - 1);
//        p_data->cell_count = vector.count();
//        for(int i = 0; i < vector.count(); i++)
//        {
//            p_data->cell_list[i] = vector.at(i);
//        }

//        //替换链表中的元素
//        *it = p_data;
//        free(p);
    }

}

void MainWindow::rawDataOutputNewFile(QString rawDataFilePath)
{
    QString fileName = rawDataFilePath.split(".").at(0) + "_PreprcsOK_" + QDate::currentDate().toString("yyyy-MM-dd") + ".txt";
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "fail to open write only file.";
        return;
    }

    QTextStream out(&file);
//    out << "preprocess complete\n";
    for(QList<meas_point_raw_data_t *>::iterator it = measPointList.begin();
        it != measPointList.end(); it ++)
    {
        meas_point_raw_data_t *p = *it;
        QString str;

        //str.append(QString("%1|").arg(p->no));
        str.append(QString(p->time));
        str.append("|");
        str.append("361,361|");
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

void MainWindow::on_queryBtn_clicked()
{
}

void MainWindow::on_baseVsTrueBtn_clicked()
{
    QMap<QString, base_info_t> baseMap;

    foreach (DriveTestItem *item, itemList) {
        QList<base_info_t> &base_infos = item->getBaseInfos();
        QList<base_meas_t> &results = item->getResults();

        if(results.count() != base_infos.count())
        {
            qDebug() << "FILE:" << __FILE__ << "LINE:" << __LINE__ << "count error";
            continue;
        }

        int count = results.count();
        for(int i = 0; i < count; i++)
        {
            QString str = QString("%1,%2").arg(results.at(i).base_meas_rslt.lac).arg(results.at(i).base_meas_rslt.ci);
            baseMap.insert(str, base_infos.at(i));
        }
    }

    QString points;
    for(QMap<QString, base_info_t>::const_iterator it = baseMap.constBegin();
        it != baseMap.constEnd(); it++)
    {
        points.append(QString::number(it.value().lng, 'g', 9));
        points.append(",");
        points.append(QString::number(it.value().lat, 'g', 9));
        points.append("|");

    }
    points.chop(1);

    QWebFrame * webFrame = ui->webView->page()->mainFrame();
    webFrame->evaluateJavaScript(QString("showTheLocaltion(\"%1\",0)").arg(points));

    on_truePosBtn_clicked();
}

void MainWindow::doLoad()
{
    QUrl url = ui->webView->page()->mainFrame()->requestedUrl();
    QString urlStr = url.toString();

    int index = urlStr.split('/', QString::SkipEmptyParts).last().toInt() - 1;
    //qDebug() << __FILE__ << __LINE__ << "测量点序号" << index;

    QString points;

    DriveTestItem *item = this->itemList.at(index);
    if(item->isBaseInfoReady())
    {
        QList<base_info_t> &base_infos = item->getBaseInfos();
        QList<base_meas_t> &meas_results = item->getResults();

        int i = 0;
        foreach (base_info_t base_info, base_infos) {
            points.append(QString::number(base_info.lng, 'g', 9));
            points.append(",");
            points.append(QString::number(base_info.lat, 'g', 9));
            points.append(",");
            points.append(QString::number(base_info.radius));
            points.append(",");
            points.append(QString::number(meas_results.at(i).base_meas_rslt.lac));
            points.append(",");
            points.append(QString::number(meas_results.at(i).base_meas_rslt.ci));
            points.append(",");
            points.append(QString::number(meas_results.at(i).base_meas_rslt.rxlev));
            points.append("|");
            i++;
        }
    }
    points.chop(1);

//    qDebug() << __FILE__ << __LINE__ << points;
    QString str = QString("addBaseLocation(\"%1\")").arg(points);
    QWebFrame * webFrame = ui->webView->page()->mainFrame();
    webFrame->evaluateJavaScript(str);
}

