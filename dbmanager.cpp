#include "dbmanager.h"
#include "common.h"


DBManager::DBManager(QObject *parent) : QObject(parent)
{
    db = QSqlDatabase::addDatabase("QSQLITE"); // add sqlite3 driver
    db.setDatabaseName("o2o.db"); //set db name
    if(db.open()) // open
    {
        // create tables if they don't exist
        QSqlQuery query(db);
        /*    QString userName;
        QString userNumber;
        QString department;
        QString org;
        QString userID;
        bool    bLogin;
        HealthInfo health;
        QString alcoholTest;
        QPixmap userPic;
        */
        bool err = query.exec("CRTEAT TABLE IF NOT EXISTS userInfo(id CHAR(8) primary key,name TEXT, number TEXT,"
                   "department TEXT, org TEXT, pic BLOB DEFAULT NULL, fp BLOB DEFAULT NULL, uploaded INTEGER DEFAULT 0)");
//        query.exec("CRTEAT TABLE IF NOT EXISTS userInfo(id CHAR(8) primary key,name TEXT, number TEXT,"
//                   "department TEXT, org TEXT, pic BLOB, uploaded INTEGER)");
        qDebug() <<  db.lastError().text();
        qDebug() << "create table:" << err;

        db.close();
    }


}

DBManager::~DBManager()
{
//    db.close();
}

bool DBManager::AddUserInfo(UserInfo userInfo)
{
    qDebug() << __func__;
    if(!db.isOpen())
    {
        db = QSqlDatabase::addDatabase("QSQLITE"); // add sqlite3 driver
        db.setDatabaseName("o2o.db"); //set db name
        if(!db.open())
        {
            return false;
        }
    }

    qDebug() << "db open:" << db.isOpen();
    // sql
    QSqlQuery query(db);

    QImage image = userInfo.userPic;
    QByteArray ba;
    QBuffer buf(&ba);
    image.save(&buf, "JPG");

    QByteArray compressed = qCompress(ba, 1); // better just open file with QFile, load data, compress and toHex?
    QByteArray hexed = compressed.toHex();
    // save to a file
    QString strPic(hexed);

    QString statement = QString("INSERT INTO userInfo(id, name, number, department, org) "
                                "VALUES(%1, %2, %3, %4, %5)")
            .arg(userInfo.userID).arg(userInfo.userName).arg(userInfo.userNumber)
            .arg(userInfo.department).arg(userInfo.org); //.arg(strPic).arg(userInfo.userFP) , pic, fp

    qDebug() << statement;
    bool err = query.exec(statement);

    qDebug() <<  db.lastError().text();
    qDebug() << "err:" << err;

    err = db.commit();

    qDebug() << "err:" << err;

    db.close();
    return err;
}


/*
 * 删除工号列表中对应的人员指纹特征
 */
bool DBManager::DelFingerFeature(QStringList empList)
{
    QFile fpFile(FILE_FP);

    if(!fpFile.open(QIODevice::ReadOnly))
    {
        return false;
    }


    QByteArray dataAll = fpFile.readAll();

    QString strAll = QString(dataAll);

    QStringList itemList = strAll.split(";;;");


}

