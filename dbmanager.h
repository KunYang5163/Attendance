#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QObject>
#include <QtSql>
#include "common.h"

class DBManager : public QObject
{
    Q_OBJECT
public:
    explicit DBManager(QObject *parent = 0);
    ~DBManager();

signals:

public slots:
    bool AddUserInfo(UserInfo userInfo);
    bool DelFingerFeature(QStringList empList);

private:
    QSqlDatabase db;
};

#endif // DBMANAGER_H
