#ifndef DRIVETESTITEM_H
#define DRIVETESTITEM_H

#include <QString>
#include <stdlib.h>
#include <stdio.h>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <position_cal/field_func.h>

#define TIME_BUF_LEN 20

struct _base_meas
{
    int arfcn;
    int lac;
    int ci;
    int rxlev;
};

typedef union
{
    struct _base_meas base_meas_rslt;
    int buf[4];
}base_meas_t;

typedef struct
{
    double lng;
    double lat;
    double radius;
    double pos_x;
    double pos_y;
}base_info_t;

typedef struct
{
    int lac;
    int ci;
    int arfcn;
    int bsic;
    int rxlev;
}cell_raw_data_t;

typedef struct
{
    int no;
    char time[TIME_BUF_LEN];
    int cell_count;
    cell_raw_data_t cell_list[];
}meas_point_raw_data_t;

class DriveTestItem : public QObject
{
    Q_OBJECT

public:
    DriveTestItem(int line, QString &lineStr);
    ~DriveTestItem();

    xml_data_t * get_xml_data(void);
    QList<base_meas_t>& getResults(void);
    QList<base_info_t>& getBaseInfos(void);
    void getTrueWGS84Coord(double *p_lng, double *p_lat);
    void getTrueBD09Coord(double *p_lng, double *p_lat);
    void getApiPositioningResult(double *p_lng, double *p_lat, double *p_radius);
    void queryBaseInformation(void);
    void multiBasePositioning(void);
    bool isBaseInfoReady(void);
    bool isAPIPosReady(void);
    bool isOurPosReady(void);

    void setOurPositioningResult(double lng, double lat, double x, double y);
    void getOurPositioningResult(double *lng, double *lat, double *x, double *y);

public slots:
    void coordTransformFinishedSlot(QNetworkReply *reply);//将wgs84经纬度转换为bd09经纬度完成槽函数
    void queryBaseInfoFinishedSlot(QNetworkReply *reply);//查询基站信息完成槽函数

private:
    QString postStr;
    QString time;
    int line;
    QString true_wgs84_lat_str;//真实的gps纬度，wgs84坐标系，字符串
    QString true_wgs84_lng_str;//真实的gps经度，wgs84坐标系，字符串
    double true_wgs84_lat;//真实的gps纬度，wgs84坐标系
    double true_wgs84_lng;//真实的gps经度，wgs84坐标系
    double true_bd09_lat;//真实的gps纬度，bd09坐标系
    double true_bd09_lng;//真实的gps经度，bd09坐标系
    double api_lat;//调用网路api定位的纬度
    double api_lng;//调用网络api定位的经度
    double api_pos_x;
    double api_pos_y;
    double api_radius;//调用网络api定位结果的经度半径
    double our_lat;
    double our_lng;
    double our_pos_x;
    double our_pos_y;
    QList<base_meas_t> results;
    QList<base_info_t> base_infos;
    int result_cnt;
    xml_data_t * p_xml_data;
    QNetworkAccessManager *coord_trans_manager;//用于坐标转换
    QNetworkAccessManager *base_query_manager;//用于查询基站
    QNetworkAccessManager *positioning_manager;//用于多基站定位查询

    bool baseInfoReadyFlag;
    bool apiPosReadyFlag;
    bool ourPosReadyFlag;
    //bool truePosReadFlag;
};

#endif // DRIVETESTITEM_H
