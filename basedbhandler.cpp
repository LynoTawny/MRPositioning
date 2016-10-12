#include "basedbhandler.h"
#include <QStringList>
#include <QDebug>

BaseDBHandler::BaseDBHandler()
{
    //查看系统的data base driver
//    QStringList drivers = QSqlDatabase::drivers();
//    qDebug() << drivers;

    this->db = QSqlDatabase::addDatabase("QSQLITE");  //采用QSQLITE数据库

    db.open();//打开数据库连接
}

BaseDBHandler::~BaseDBHandler()
{
    db.close();//释放数据库
}

