#ifndef BASEDBHANDLER_H
#define BASEDBHANDLER_H

#include <QSqlDatabase>

class BaseDBHandler
{
public:
    BaseDBHandler();
    ~BaseDBHandler();

private:
    QSqlDatabase db;
};

#endif // BASEDBHANDLER_H
