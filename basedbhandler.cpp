#include "basedbhandler.h"
#include <QStringList>
#include <QDebug>
#include <QSqlError>

#define SQL_CREATE_TABLE    "create table base (mcc int, mnc int, lac int, ci int, lng real, lat real, type text)"
#define SQL_INSERT
#define SQL_SELECT

BaseDBHandler::BaseDBHandler(QString dbPath)
{
    //查看系统的data base driver
//    QStringList drivers = QSqlDatabase::drivers();
//    qDebug() << drivers;

    this->db = QSqlDatabase::addDatabase("QSQLITE");  //采用QSQLITE数据库
    this->db.setDatabaseName(dbPath);
}

BaseDBHandler::~BaseDBHandler()
{
    if(db.isOpen())
        db.close();//释放数据库
}

bool BaseDBHandler::openBaseDB(void)
{
    if(!this->db.open())
    {
        qDebug() << "file:" << __FILE__ << "line" << __LINE__ << "Database Error!";
        return false;
    }

    QSqlQuery query;
    if (!query.exec(SQL_CREATE_TABLE))
    {
        qDebug() << "file:" << __FILE__ << "line" << __LINE__ << "Create Table Failed!";
        return false;
    }

    return true;
}

bool BaseDBHandler::insertBaseInfo(BaseInfo &base)
{
    QSqlQuery query;
    query.prepare("insert into base(mcc, mnc, lac, ci, lng, lat, type)"
                      "values(:mcc, :mnc, :lac, :ci, :lng, :lat, :type)");

    //query.bindValue(":id", 1);
    query.bindValue(":mcc", base.mcc);
    query.bindValue(":mnc", base.mnc);
    query.bindValue(":lac", base.lac);
    query.bindValue(":ci", base.ci);
    query.bindValue(":lng", base.lng);
    query.bindValue(":lat", base.lat);
    query.bindValue(":type", base.coordType);

    if(!query.exec())
    {
        qDebug() << "file:" << __FILE__ << "line" << __LINE__ << query.lastError();
        return false;
    }
    else
    {
        qDebug() << "file:" << __FILE__ << "line" << __LINE__ << "inserted!";
        return true;
    }
}

bool BaseDBHandler::updateBaseInfo(BaseInfo &base)
{
    return false;
}

bool BaseDBHandler::queryBaseInfo(int mcc, int mnc, int lac, int ci, double *lng, double *lat, QString *str)
{
    QString sql("select * from base where lac=? and ci=? and mnc=? and mcc=? ");

    QSqlQuery query;
    query.prepare(sql);
    query.addBindValue(lac);
    query.addBindValue(ci);
    query.addBindValue(mnc);
    query.addBindValue(mcc);

    if(!query.exec())
    {
        qDebug() << "file:" << __FILE__ << "line" << __LINE__ << query.lastError();
        return false;
    }
    else
    {
        while(query.next())
        {
            *lng = query.value("lng").toDouble();
            *lat = query.value("lat").toDouble();
            qDebug() << "file:" << __FILE__ << "; line:" << __LINE__ << "query done";
            return true;
        }

        return false;
    }
}
