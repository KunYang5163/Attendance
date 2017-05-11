#include "uploadthread.h"
#include "devicemanager.h"
#include "common.h"
#include <QHostAddress>
#include "JQChecksum.h"
#include "common.h"
#include <QCryptographicHash>
#include <QJsonObject>
#include <QJsonDocument>

UploadThread::UploadThread(QObject *parent):
    QThread(parent)
{
    socket = NULL;
}


void UploadThread :: run()
{
    qDebug() << __func__;
    DeviceManager dManager;
    QList<DevNode *> nodeList;
    dManager.getServerInfo(nodeList);

    QString serverIP = nodeList.at(0)->value;
    QString serverPort = nodeList.at(1)->value;

    nodeList.clear();

    if(NULL == socket)
    {
        socket = new QTcpSocket();
    }
    if(serverIP.length() > 0 && serverPort.length() > 0)
    {
        qDebug() << __LINE__;
        socket->connectToHost(QHostAddress(serverIP), serverPort.toInt());
        if(!socket->waitForConnected(2000))
        {
            qDebug() << "can not connect to server";
            sigShowMsg("无法连接到服务器"+serverIP+":"+serverPort);
            return;
        }
         qDebug() << __LINE__;
    }

    if(!socket->isValid())
    {
        // display warning message
        sigShowMsg("无法连接到服务器"+serverIP+":"+serverPort);

        qDebug() << "无法连接到服务器"+serverIP+":"+serverPort;
        return;
    }

    // send local dev info to server
    devAuthentication();

    exec();
}

/*
 * 向认证服务器发送设备信息
 */
void UploadThread :: devAuthentication()
{
    qDebug(__func__);
    MSG_HEADER header;
    header.mHFlag = PACKAGE_HEADER_FLAG;
    header.mFFlag = COMMAND_IDENTIFICATION;
    header.mVersion = PACKAGE_VERSION;


    DeviceManager dManager;
    QList<DevNode *> nodeList;
    dManager.getDevInfo(nodeList);

    QString digId = nodeList.at(3)->value;
    QString devId = nodeList.at(1)->value;

    nodeList.clear();
    QByteArray byte_array;
    byte_array.append(digId.toLatin1());
    QByteArray hash_byte_array = QCryptographicHash::hash(byte_array, QCryptographicHash::Sha1);
    QString sha1 = hash_byte_array.toHex();
    QJsonObject json;
    json.insert("设备ID", devId);
    json.insert("数字签名", digId);
    json.insert("校验值", sha1);

    QJsonDocument document;
    document.setObject(json);
    QByteArray byteArray = document.toJson(QJsonDocument::Compact);


    header.mCrc = JQChecksum :: crc32(byteArray);
    header.mLength = byteArray.length();
    int ret = 0;
    ret =  socket->write((char *)&header, sizeof(header));
    qDebug() << ret;
    qDebug() << "header.length:" << header.mLength;
    qDebug() << "header.mCrc:" << header.mCrc;
    if(sizeof(header) != ret)
    {
        qDebug() << "write header error";
        return ;
    }

    qDebug() << byteArray;
    ret = socket->write(byteArray);
    qDebug() << ret;
    if(byteArray.length() != ret)
    {
        return ;
    }

//    QThread::yieldCurrentThread();
//    socket->waitForReadyRead();
//    memset(&header, 0x0, sizeof(header));
//    ret =  socket->read((char *)&header, sizeof(header));

//    qDebug("response:");
//    qDebug("mHFlag: %x",header.mHFlag);
//    qDebug("mFFlag: %x",header.mFFlag);
//    qDebug() << "mVersion:" << header.mVersion;
//    qDebug() << "mCrc:" << header.mCrc;
//    qDebug() << "mLength:" << header.mLength;

//    if(header.mLength > 0)
//    {
//        getData = socket->read(header.mLength);
//    }

}
