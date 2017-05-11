#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QtXml>
#include <QList>
#include "stm32comm.h"

#if defined _WIN32 && !defined(Q_OS_SYMBIAN)
    #define  DEV_FILE "D:/devinfo.xml"
#elif defined(Q_OS_ANDROID)
    #define DEV_FILE "devinfo.xml"
#else
    #define DEV_FILE "devinfo.xml"
#endif

struct DevNode
{
public:
    QString name;
    QString value;
};


class DeviceManager : public QObject
{
    Q_OBJECT
public:
    explicit DeviceManager(QObject *parent = 0);

public:
    void getDevInfo(QList<DevNode *> &nodeList);
    bool getServerInfo(QList<DevNode *> &nodeList);
    void createDefaultDevInfo();
    int updateDevInfo(QList<DevNode *> nodeList);

public slots:
    void checkDevice(int &ret, char &status);

signals:

public slots:

};

#endif // DEVICEINFO_H
