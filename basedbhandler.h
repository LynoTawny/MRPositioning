#ifndef BASEDBHANDLER_H
#define BASEDBHANDLER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>

class BaseInfo
{
public:
    int mcc;
    int mnc;
    int lac;
    int ci;
    double lat;
    double lng;
    QString coordType;

    void setBaseID(int mcc, int mnc, int lac, int ci)
    {
        this->mcc = mcc;
        this->mnc = mnc;
        this->ci = ci;
        this->lac = lac;
    }

    void setBasePos(double lng, double lat, QString &type)
    {
        this->lat = lat;
        this->lng = lng;
        this->coordType = type;
    }

    QString getPos(double *lng, double *lat)
    {
        *lng = this->lng;
        *lat = this->lat;
        return coordType;
    }
};

class BaseDBHandler
{
public:
    BaseDBHandler(QString dbPath);
    ~BaseDBHandler();
    bool openBaseDB(void);
    bool insertBaseInfo(BaseInfo &base);
    bool updateBaseInfo(BaseInfo &base);
    bool queryBaseInfo(int mcc, int mnc, int lac, int ci, double *lng, double *lat, QString *str);

private:
    QSqlDatabase db;
};

#endif // BASEDBHANDLER_H
