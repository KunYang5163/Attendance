#include "clientthread.h"
#include "common.h"
#include "qobjecthelper.h"
#include "parser.h"
#include "devicemanager.h"
#include <serializer.h>
#include "JQChecksum.h"
#include <QTcpSocket>
#include "alcoholtestthread.h"
#include "facial.h"

#ifdef Q_OS_ANDROID
#include <QtAndroidExtras/QAndroidJniObject>
#endif

ClientThread::ClientThread(int socketDes,QLabel *label, QObject *parent) :
    QThread(parent), socketDescriptor(socketDes), textLabel(label)
{
    isCameraStarted = false;
}


void ClientThread::run()
{
    qDebug(__func__);
    socket = new QTcpSocket();

    if (!socket->setSocketDescriptor(socketDescriptor)) {
        qDebug() << "failed to setSocketDescriptor";
        return;
    }

    connect(socket, SIGNAL(readyRead()), this, SLOT(readData()), Qt::DirectConnection );
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));

    // start timer to send dev info
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(reportDevStatusPer10min()));
    timer->start(1000 * 60 * 10);

    exec();

//    int count = 0;
//    while(!tcpSocket.waitForReadyRead(1000) && count < 20)
//    {
//        qDebug() << "before waitForReadyRead";
//        QThread::yieldCurrentThread();
//        count++;
//        qDebug() << "after waitForReadyRead " << count;
//    }

//    MSG_HEADER header;

//    memset(&header, 0x0, sizeof(MSG_HEADER));
//    tcpSocket.read((char *)&header, sizeof(MSG_HEADER));

//    qDebug() << __func__;
//    qDebug("mHFlag: %x",header.mHFlag);
//    qDebug("mFFlag: %x",header.mFFlag);
//    //    qDebug() << "mFFlag:" << header.mFFlag;
//    qDebug() << "mVersion:" << header.mVersion;
//    qDebug() << "mCrc:" << header.mCrc;
//    qDebug() << "mLength:" << header.mLength;

//    switch(header.mFFlag)
//    {
//    case COMMAND_GET_DEVICE_INFO:
//        packageHandler_devinfo(&tcpSocket);
//        break;
//    case COMMAND_DISPLAY_MESSAGE:
//        packageHandler_displayText(&tcpSocket, header.mLength);
//        break;
//    case COMMAND_PLAY_SOUND:
//        packageHandler_playSound(&tcpSocket,  header.mLength);
//        break;
//    case COMMAND_CHECK_DEVICE:
//        packageHandler_checkDev(&tcpSocket);
//        break;

//    case 0x50:
//        packageHandler_identification(&tcpSocket, header.mLength);
//        break;

//    default:
//        break;
//    }
//    qDebug() << "before waitForDisconnected";
//    tcpSocket.waitForDisconnected();
//    qDebug() << "after waitForDisconnected";
//    tcpSocket.close();

}

void ClientThread::disconnected()
{
    qDebug() << socketDescriptor << " Disconnected";


    socket->deleteLater();
    exit(0);
}


bool ClientThread :: sendPackage(unsigned char command, QByteArray data)
{
    qDebug(__func__);


    MSG_HEADER header;
    header.mHFlag = PACKAGE_HEADER_FLAG;
    header.mFFlag = command;
    header.mVersion = PACKAGE_VERSION;

    header.mCrc = JQChecksum :: crc32(data);
    header.mLength = data.length();
    if(sizeof(header) != socket->write((char *)&header, sizeof(header)))
    {
        return false;
    }

    if(header.mLength > 0)
    {
        if(data.length() != socket->write(data))
        {
            return false;
        }
    }

    return true;
}

void ClientThread :: readData()
{
    MSG_HEADER header;

    int recv = 0;
    qDebug() << "sizeof(MSG_HEADER)" << sizeof(MSG_HEADER);
    memset(&header, 0x0, sizeof(MSG_HEADER));
    recv = socket->read((char *)&header, sizeof(MSG_HEADER));

    qDebug() << __func__;
    qDebug() << "sizeof(header)" << sizeof(MSG_HEADER);
    qDebug() << "recv ::" << recv ;
    qDebug("mHFlag: %x",header.mHFlag);
    qDebug("mFFlag: %x",header.mFFlag);
    //    qDebug() << "mFFlag:" << header.mFFlag;
    qDebug() << "mVersion:" << header.mVersion;
    qDebug() << "mCrc:" << header.mCrc;
    qDebug() << "mLength:" << header.mLength;

    switch(header.mFFlag)
    {
    case COMMAND_GET_DEVICE_INFO:
        // 获取设备信息
        packageHandler_devinfo();
        break;
    case COMMAND_ALCOHOL_DETECTION:
        // 开始测酒
        packageHandler_alcoholDetection(header.mLength, header.mCrc);
        break;
    case COMMAND_DISPLAY_MESSAGE:
        // 显示文字
        packageHandler_displayText(header.mLength, header.mCrc);
        break;
    case COMMAND_PLAY_SOUND:
        // 播报语音
        packageHandler_playSound(header.mLength, header.mCrc);
        break;
    case COMMAND_CHECK_DEVICE:
        // 设备自检
        packageHandler_checkDev();
        break;


    case COMMAND_DEL_FINGER_PRINT:
        packageHandler_delFingerPrint(header.mLength, header.mCrc);
        break;

    case COMMAND_DEL_FACIAL:
        packageHandler_delFacial(header.mLength, header.mCrc);
        break;
    case COMMAND_ADD_FACIAL:
        packageHandler_addFacial(header.mLength, header.mCrc);
        break;

    case COMMAND_ADD_FINGER_PRINT:
        packageHandler_addFingerPrint(header.mLength, header.mCrc);
        break;


    case COMMAND_DEV_IDENTIFY:
        packageHandler_identification();
        break;

    case COMMAND_TRANS_VIDEO:
        packageHandler_transportVideo();
        break;

    case COMMAND_STOP_VIDEO:
        break;

    case COMMAND_DEV_REGISTER:
        if(header.mLength > 0)
        {
            socket->read(header.mLength);
        }

        break;

    case COMMAND_FACIAL_REGISTER:
        packageHandler_facialRegister(header.mLength, header.mCrc);
        break;

    case COMMAND_FINGER_REGISTER:
        packageHandler_fingerRegister(header.mLength, header.mCrc);
        break;

    case COMMAND_DOWNLOAD_USER:
        packageHandler_downloadUser(header.mLength, header.mCrc);
        break;

    case COMMAND_DEL_USER:
        packageHandler_delUser(header.mLength, header.mCrc);
        break;

    case COMMAND_GET_TEMPERATURE:
        packageHandler_getTemp(header.mLength, header.mCrc);
        break;

    default:
        if(header.mLength > 0)
        {
            QByteArray getData = socket->read(header.mLength);
            qDebug() << getData;
        }
        break;
    }
//    qDebug() << "before waitForDisconnected";
//    tcpSocket.waitForDisconnected();
//    qDebug() << "after waitForDisconnected";
//    tcpSocket.close();
}


void ClientThread :: packageHandler_devinfo()
{
    qDebug(__func__);

    MSG_HEADER header;
    header.mHFlag = PACKAGE_HEADER_FLAG;
    header.mFFlag = COMMAND_GET_DEVICE_INFO;
    header.mVersion = PACKAGE_VERSION;


    QList<DevNode *> nodeList;

    DeviceManager devinfo;

    devinfo.getDevInfo(nodeList);


    QVariantMap devjson;
    devjson.insert("devStatus", nodeList.at(2)->value);
    devjson.insert("devName", nodeList.at(0)->value);
    devjson.insert("devID", nodeList.at(1)->value);
    devjson.insert("signature", nodeList.at(3)->value);
    devjson.insert("version", nodeList.at(4)->value);

    QJson::Serializer serializer;
    QByteArray json = serializer.serialize(devjson);
    qDebug() << json;

    header.mCrc = JQChecksum :: crc32(json);
    header.mLength = json.length();
    socket->write((char *)&header, sizeof(header));
    socket->write(json);
    socket->waitForBytesWritten();
//    socket->close();

//    nodeList.clear();
}

void ClientThread :: packageHandler_displayText(quint32 len, quint32 crc)
{
    qDebug(__func__);

    QByteArray getData = socket->read(len);
    QString err;


    if(!getData.isEmpty())
    {
        qDebug() << "data:" << getData;

        if(JQChecksum :: crc32(getData) != crc)
        {
            qDebug() << "crc not match, get " << JQChecksum :: crc32(getData) << "but we expect " << crc;
            err = "CRC校验错误";
            goto response;
        }

        bool ok;
        QJson::Parser parser;
        QVariantMap result = parser.parse(getData, &ok).toMap();

        if (!ok) {
            qFatal("An error occurred during parsing");
            return;
        }

        textLabel->setText(result["text"].toString());
    }
    else
    {
        err = "缺少显示内容"
    }

response:
    MSG_HEADER header;
    header.mHFlag = PACKAGE_HEADER_FLAG;
    header.mFFlag = COMMAND_DISPLAY_MESSAGE;
    header.mVersion = PACKAGE_VERSION;

    QVariantMap devjson;


    if(err.isEmpty())
    {
        devjson.insert("result", "1");
    }
    else
    {
        devjson.insert("result", "0");
    }
    devjson.insert("data", err);

    QJson::Serializer serializer;
    QByteArray json = serializer.serialize(devjson);
    qDebug() << json;

    header.mCrc = JQChecksum :: crc32(json);
    header.mLength = json.length();
    socket->write((char *)&header, sizeof(header));
    socket->write(json);

    socket->waitForBytesWritten();
}


void ClientThread :: packageHandler_playSound(quint32 len, quint32 crc)
{
    qDebug(__func__);

    QByteArray getData = socket->read(len);
    QString err;

    if(!getData.isEmpty())
    {
        qDebug() << "data:" << getData;
        if(JQChecksum :: crc32(getData) != crc)
        {
            qDebug() << "crc not match, get " << JQChecksum :: crc32(getData) << "but we expect " << crc;
            err = "CRC校验错误";
            goto response;
        }

        bool ok;
        QJson::Parser parser;
        QVariantMap result = parser.parse(getData, &ok).toMap();

        if (!ok) {
            qFatal("An error occurred during parsing");
            return;
        }

        qDebug() << "playSound:" << result["text"];
        if(result["text"].toString().length() > 0)
        {
#ifdef Q_OS_ANDROID

            QAndroidJniObject textToSpeak = QAndroidJniObject::fromString(result["text"].toString());

            QAndroidJniObject::callStaticMethod<void>("o2oAttendance/QtFullscreenActivity",
                                                      "speak",
                                                      "(Ljava/lang/String;)V",
                                                      textToSpeak.object<jstring>());
#endif
            textLabel->setText(result["text"].toString());
        }
        else
        {
            err = "播报文字错误";
        }
    }
    else
    {
        err = "缺少播报内容"
    }


response:
    MSG_HEADER header;
    header.mHFlag = PACKAGE_HEADER_FLAG;
    header.mFFlag = COMMAND_PLAY_SOUND;
    header.mVersion = PACKAGE_VERSION;

    QVariantMap devjson;

    if(err.isEmpty())
    {
        devjson.insert("result", "1");
    }
    else
    {
        devjson.insert("result", "0");
    }
    devjson.insert("data", err);

    QJson::Serializer serializer;
    QByteArray json = serializer.serialize(devjson);
    qDebug() << json;

    header.mCrc = JQChecksum :: crc32(json);
    header.mLength = json.length();
    socket->write((char *)&header, sizeof(header));
    socket->write(json);

    socket->waitForBytesWritten();
}

/*
 * 下位机自检
 */
void ClientThread :: packageHandler_checkDev()
{
    qDebug(__func__);

    MSG_HEADER header;
    header.mHFlag = PACKAGE_HEADER_FLAG;
    header.mFFlag = COMMAND_CHECK_DEVICE;
    header.mVersion = PACKAGE_VERSION;

    // todo
    // 设备检查
    int ret = 0;
    QString msg;
    DeviceManager dManager;
    char status = 0;
    dManager.checkDevice(ret, status);

    if(!(status & STM_BIT_STATUS_SUCCESS))
    {
        // 读取状态失败
        msg = QString("读取设备状态失败");
    }
    else
    {
        if(!(status & STM_BIT_STATUS_TH))
        {
            msg += QString("[温湿度模块]");
        }
        if(!(status & STM_BIT_STATUS_ALCO))
        {
            msg += QString("[测酒模块]");
        }
        if(!(status & STM_BIT_STATUS_BAT))
        {
            msg += QString("[电池]");
        }
        if(!(status & STM_BIT_STATUS_BP))
        {
            msg += QString("[血压模块]");
        }
        if(!(status & STM_BIT_STATUS_INFRA))
        {
            msg += QString("[红外体温模块]");
        }

        if(msg.length() > 0)
        {
            msg += QString("异常");
        }
    }

    QList<DevNode *> nodeList;

    DeviceManager devinfo;

    devinfo.getDevInfo(nodeList);
    QVariantMap devjson;
    devjson.insert("devID", nodeList.at(1)->value);

    if(msg.length() > 0)
    {
        devjson.insert("result", "0");
    }
    else
    {
        devjson.insert("result", "1");
    }
    devjson.insert("data", msg);

    QJson::Serializer serializer;
    QByteArray json = serializer.serialize(devjson);
    qDebug() << json;

    header.mCrc = JQChecksum :: crc32(json);
    header.mLength = json.length();
    socket->write((char *)&header, sizeof(header));
    socket->write(json);
    socket->waitForBytesWritten();

    nodeList.clear();
}

void ClientThread :: packageHandler_identification()
{
    qDebug(__func__);

    MSG_HEADER header;
    header.mHFlag = PACKAGE_HEADER_FLAG;
    header.mFFlag = COMMAND_DEV_IDENTIFY;
    header.mVersion = PACKAGE_VERSION;

    // todo
    // 设备检查

//    QList<DevNode *> nodeList;

//    DeviceInfo devinfo;

//    devinfo.getDevInfo(nodeList);
    QVariantMap devjson;
//    devjson.insert("设备ID", nodeList.at(1)->value);
    devjson.insert("结果", "成功");

    QJson::Serializer serializer;
    QByteArray json = serializer.serialize(devjson);
    qDebug() << json;

    header.mCrc = JQChecksum :: crc32(json);
    header.mLength = json.length();
    socket->write((char *)&header, sizeof(header));
    socket->write(json);
    socket->waitForBytesWritten();
//    socket->close();
}


/*
 * 处理上位机开始测酒指令
 *
 *
 * 调用测酒线程进行测酒，成功则返回测酒结果，失败返回错误描述
 */
void ClientThread :: packageHandler_alcoholDetection(quint32 len, quint32 crc)
{

    qDebug(__func__);

    QByteArray getData = socket->read(len);
    QString EmployeeNumber;
    QString err;

    alcValue = 0;
    qDebug() << "alcValue:" << alcValue;
    if(!getData.isEmpty())
    {
        qDebug() << "data:" << getData;
        if(JQChecksum :: crc32(getData) != crc)
        {
            qDebug() << "crc not match, get " << JQChecksum :: crc32(getData) << "but we expect " << crc;
            err = "失败,CRC校验错误";
        }
        else
        {
            bool ok;
            QJson::Parser parser;
            QVariantMap result = parser.parse(getData, &ok).toMap();

            if (!ok) {
                qFatal("An error occurred during parsing");
                err = "数据解析错误";
            }
            else
            {
                EmployeeNumber = result["employeeID"].toString();
                if(EmployeeNumber.isEmpty())
                {
                    err = "工号错误";
                }
            }
        }
    }
    else
    {
        err = "缺少工号信息";
    }

    if(err.isEmpty())
    {
        // create thread to test alcohol
        AlcoholTestThread *att = new AlcoholTestThread();
        connect(att,SIGNAL(sigChangeMsg(QString)),this->parent(),SLOT(updateMessage(QString)),Qt::AutoConnection);
        connect(att,SIGNAL(sigChangeAlcoholResult(int)),this,SLOT(getAlcValue(int)),Qt::AutoConnection);
        connect(att, SIGNAL(finished()), att, SLOT(deleteLater()));
        att->start();

        att->wait(30 * 1000);
    }

    // send response to client
    MSG_HEADER header;
    header.mHFlag = PACKAGE_HEADER_FLAG;
    header.mFFlag = COMMAND_ALCOHOL_DETECTION;
    header.mVersion = PACKAGE_VERSION;

    QVariantMap devjson;

    qDebug() << "alcValue:" << alcValue;
    if(!err.isEmpty())
    {
        devjson.insert("employeeID", "");
        devjson.insert("result", "0");
        devjson.insert("data", err);
        devjson.insert("picture", "");
    }
    else
    {
        devjson.insert("employeeID", EmployeeNumber);
        devjson.insert("result", "1");
        devjson.insert("data", QString::number(alcValue));
        devjson.insert("picture", "");
    }


    QJson::Serializer serializer;
    QByteArray json = serializer.serialize(devjson);
    qDebug() << json;

    header.mCrc = JQChecksum :: crc32(json);
    header.mLength = json.length();
    socket->write((char *)&header, sizeof(header));
    socket->write(json);
    socket->waitForBytesWritten();
//    socket->close();
}


/*
 *  接收测酒线程返回的测酒值。
 *
 *  绑定sigChangeAlcoholResult信号
 */
void ClientThread :: getAlcValue(int v)
{
    alcValue = v;
}


void ClientThread :: packageHandler_delFingerPrint(quint32 len, quint32 crc)
{
    Q_UNUSED(crc);
    QByteArray getData = socket->read(len);
    QString EmployeeNumber;
    QString err;

    if(!getData.isEmpty())
    {
//        if(JQChecksum :: crc32(getData) != crc)
//        {
//            qDebug() << "crc not match, get " << JQChecksum :: crc32(getData) << "but we expect " << crc;
//            err = "失败,CRC校验错误";
//            goto response;
//        }

        qDebug() << "data:" << getData;
        bool ok;
        QJson::Parser parser;
        QVariant result = parser.parse(getData, &ok).toMap();

        if (!ok) {
            qFatal("An error occurred during parsing");
            err = "失败,JSON解析错误";
            goto response;
        }

        QVariantList mylist = result.toList();

          foreach (QVariant plugin, mylist) {

           QVariantMap mymap = plugin.toMap();

           qDebug() << "[" << mymap["employeeID"].toString() << "]";

//        EmployeeNumber = result["工号"].toString();
//        if(EmployeeNumber.isEmpty())
//        {
//            err = "工号错误";
//        }
          }
    }

response:
    // send response to client
    MSG_HEADER header;
    header.mHFlag = PACKAGE_HEADER_FLAG;
    header.mFFlag = COMMAND_DEL_FINGER_PRINT;
    header.mVersion = PACKAGE_VERSION;

    QVariantMap devjson;

    if(err.isEmpty())
    {
       err = "成功";
    }

    devjson.insert("结果", err);


    QJson::Serializer serializer;
    QByteArray json = serializer.serialize(devjson);
    qDebug() << json;

    header.mCrc = JQChecksum :: crc32(json);
    header.mLength = json.length();
    socket->write((char *)&header, sizeof(header));
    socket->write(json);
    socket->waitForBytesWritten();
//    socket->close();
}


void ClientThread :: packageHandler_addFacial(quint32 len, quint32 crc)
{
    QByteArray getData = socket->read(len);
    QString EmployeeNumber;
    QString err;

    if(!getData.isEmpty())
    {
        qDebug() << "data:" << getData;
        if(JQChecksum :: crc32(getData) != crc)
        {
            qDebug() << "crc not match, get " << JQChecksum :: crc32(getData) << "but we expect " << crc;
            err = "失败,CRC校验错误";
            goto response;
        }

        bool ok;
        QJson::Parser parser;
        QVariant result = parser.parse(getData, &ok).toMap();

        if (!ok) {
            qFatal("An error occurred during parsing");
            err = "失败,JSON解析错误";
            goto response;
        }

        QVariantList mylist = result.toList();

          foreach (QVariant plugin, mylist) {

           QVariantMap mymap = plugin.toMap();

           qDebug() << "[" << mymap["工号"].toString() << "]";

//        EmployeeNumber = result["工号"].toString();
//        if(EmployeeNumber.isEmpty())
//        {
//            err = "工号错误";
//        }
          }
    }

response:
    // send response to client
    MSG_HEADER header;
    header.mHFlag = PACKAGE_HEADER_FLAG;
    header.mFFlag = COMMAND_ADD_FACIAL;
    header.mVersion = PACKAGE_VERSION;

    QVariantMap devjson;

    if(err.isEmpty())
    {
       err = "成功";
    }

    devjson.insert("结果", err);


    QJson::Serializer serializer;
    QByteArray json = serializer.serialize(devjson);
    qDebug() << json;

    header.mCrc = JQChecksum :: crc32(json);
    header.mLength = json.length();
    socket->write((char *)&header, sizeof(header));
    socket->write(json);
    socket->waitForBytesWritten();
//    socket->close();
}


void ClientThread :: packageHandler_addFingerPrint(quint32 len, quint32 crc)
{
    QByteArray getData = socket->read(len);
    QString EmployeeNumber;
    QString err;

    if(!getData.isEmpty())
    {
        qDebug() << "data:" << getData;
        if(JQChecksum :: crc32(getData) != crc)
        {
            qDebug() << "crc not match, get " << JQChecksum :: crc32(getData) << "but we expect " << crc;
            err = "失败,CRC校验错误";
            goto response;
        }

        bool ok;
        QJson::Parser parser;
        QVariant result = parser.parse(getData, &ok).toMap();

        if (!ok) {
            qFatal("An error occurred during parsing");
            err = "失败,JSON解析错误";
            goto response;
        }

        QVariantList mylist = result.toList();

          foreach (QVariant plugin, mylist) {

           QVariantMap mymap = plugin.toMap();

           qDebug() << "[" << mymap["工号"].toString() << "]";

//        EmployeeNumber = result["工号"].toString();
//        if(EmployeeNumber.isEmpty())
//        {
//            err = "工号错误";
//        }
          }
    }

response:
    // send response to client
    MSG_HEADER header;
    header.mHFlag = PACKAGE_HEADER_FLAG;
    header.mFFlag = COMMAND_ADD_FINGER_PRINT;
    header.mVersion = PACKAGE_VERSION;

    QVariantMap devjson;

    if(err.isEmpty())
    {
       err = "成功";
    }

    devjson.insert("结果", err);


    QJson::Serializer serializer;
    QByteArray json = serializer.serialize(devjson);
    qDebug() << json;

    header.mCrc = JQChecksum :: crc32(json);
    header.mLength = json.length();
    socket->write((char *)&header, sizeof(header));
    socket->write(json);
    socket->waitForBytesWritten();
//    socket->close();
}


void ClientThread :: packageHandler_delFacial(quint32 len, quint32 crc)
{
    QByteArray getData = socket->read(len);
    QString EmployeeNumber;
    QString err;

    if(!getData.isEmpty())
    {
        qDebug() << "data:" << getData;
        if(JQChecksum :: crc32(getData) != crc)
        {
            qDebug() << "crc not match, get " << JQChecksum :: crc32(getData) << "but we expect " << crc;
            err = "失败,CRC校验错误";
            goto response;
        }


        QString text;
        QJsonParseError jsonError;
        QJsonDocument doucment = QJsonDocument::fromJson(getData, &jsonError);  // 转化为 JSON 文档
        if (!doucment.isNull() && (jsonError.error == QJsonParseError::NoError)) { // 解析未发生错误
            if (doucment.isArray()) { // JSON 文档为数组
                QJsonArray array = doucment.array();  // 转化为数组
                int nSize = array.size();  // 获取数组大小
                for (int i = 0; i < nSize; ++i) {  // 遍历数组
                    QJsonValue value = array.at(i);
                    text.append( "工号:");
                    text.append(value.toString());
                    text.append("\n");

                    qDebug() << "value: " << value.toString();;  // 获取指定 key 对应的 value

                }
            }
        }

         textLabel->setText(text);
    }

response:
    // send response to client
    MSG_HEADER header;
    header.mHFlag = PACKAGE_HEADER_FLAG;
    header.mFFlag = COMMAND_DEL_FACIAL;
    header.mVersion = PACKAGE_VERSION;

    QVariantMap devjson;

    if(err.isEmpty())
    {
       err = "成功";
    }

    devjson.insert("结果", err);


    QJson::Serializer serializer;
    QByteArray json = serializer.serialize(devjson);
    qDebug() << json;

    header.mCrc = JQChecksum :: crc32(json);
    header.mLength = json.length();
    socket->write((char *)&header, sizeof(header));
    socket->write(json);
    socket->waitForBytesWritten();
//    socket->close();
}


void ClientThread :: packageHandler_transportVideo()
{
    // init camera
    setupCamera();

    // set a timer to caputre frame
    if(captureTimer->isActive())
    {
        // send response to client

        return;
    }

    takeImage();
//    captureTimer= new QTimer();
//    connect(captureTimer, SIGNAL(timeout()), this, SLOT(takeImage()));
//    captureTimer->start(1000);
}

void ClientThread :: setupCamera()
{
    if(!isCameraStarted)
    {
        if(camera != NULL)
        delete camera;
        if(imageCapture != NULL)
//        delete imageCapture;
        camera = new QCamera;

        imageCapture = new QCameraImageCapture(camera);

        connect(imageCapture, SIGNAL(imageCaptured(int,QImage)), this, SLOT(processCapturedImage(int,QImage)));
//        connect(imageCapture, SIGNAL(error(int,QCameraImageCapture::Error,QString)), this,
//                SLOT(displayCaptureError(int,QCameraImageCapture::Error,QString)));
        camera->start();
        isCameraStarted = true;
    }

}

void ClientThread::takeImage()
{
    imageCapture->capture();
}

void ClientThread::processCapturedImage(int requestId, const QImage& img)
{
    Q_UNUSED(requestId);
    Q_UNUSED(img);
    return ;

//    QByteArray ba;

//    QBuffer buffer(&ba);
//    buffer.open(QIODevice::WriteOnly);
//    bool ret = img.save(&buffer, "JPG"); // writes image into ba in PNG format
//    qDebug() << "image save to buffer ret:" << ret;
//    buffer.close();

//    qDebug() << "bytearray size:" << ba.length();
//    QByteArray hexed = ba.toBase64();
//    qDebug() << "base64 bytearray size:" << hexed.length();
//    QString str = QString::fromLatin1(hexed.data());

    // send response to client
     MSG_HEADER header;
     header.mHFlag = PACKAGE_HEADER_FLAG;
     header.mFFlag = COMMAND_TRANS_VIDEO;
     header.mVersion = PACKAGE_VERSION;

     QVariantMap jsonMap;

     bool ret = false;
     QString str;
     jsonMap.insert("success", ret ? "1":"0");
     jsonMap.insert("data", ret ? str : "");
     QJson::Serializer serializer;
     QByteArray json = serializer.serialize(jsonMap);
     qDebug() << json;

     header.mCrc = JQChecksum :: crc32(json);
     header.mLength = json.length();
     socket->write((char *)&header, sizeof(header));
     socket->write(json);
     socket->waitForBytesWritten();
}


void ClientThread :: packageHandler_stopVideo()
{
    // stop tiemr
    captureTimer->stop();
    // release camera
    camera->stop();
    isCameraStarted = false;
    delete camera;
    delete imageCapture;


}


void ClientThread:: reportDevStatusPer10min()
{
    qDebug(__func__);
    QByteArray getData;

    MSG_HEADER header;
    header.mHFlag = PACKAGE_HEADER_FLAG;
    header.mFFlag = COMMAND_DEV_REGISTER;
    header.mVersion = PACKAGE_VERSION;

    DeviceManager *dManager = new DeviceManager();
    QList<DevNode *> nodeList;
    dManager->getDevInfo(nodeList);

    QString digId = nodeList.at(3)->value;
    QByteArray byte_array;
    byte_array.append(digId.toLatin1());
    QByteArray hash_byte_array = QCryptographicHash::hash(byte_array, QCryptographicHash::Sha1);
    QString sha1 = hash_byte_array.toHex();
    QJsonObject json;
    json.insert("设备ID", nodeList.at(1)->value);
    json.insert("数字签名", digId);
    json.insert("校验值", sha1);
    json.insert("有线MAC", nodeList.at(5)->value);
    json.insert("无线MAC", nodeList.at(6)->value);

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

return;
//    QThread::yieldCurrentThread();
    socket->waitForReadyRead();
    memset(&header, 0x0, sizeof(header));
    ret =  socket->read((char *)&header, sizeof(header));

    qDebug("response:");
    qDebug("mHFlag: %x",header.mHFlag);
    qDebug("mFFlag: %x",header.mFFlag);
    qDebug() << "mVersion:" << header.mVersion;
    qDebug() << "mCrc:" << header.mCrc;
    qDebug() << "mLength:" << header.mLength;

    if(header.mLength > 0)
    {
        getData = socket->read(header.mLength);
    }

    if(getData.isEmpty())
    {
        return;
    }

    qDebug() << getData;
//    bool ok;
//    QJson::Parser parser;
//    qDebug() << getData;
//    QVariantMap result = parser.parse(getData, &ok).toMap();

//    if (!ok) {
//        qFatal("An error occurred during parsing");
//        return ;
//    }
}

//static char facialFeatureBuff[11 * 1024];

/*
 * 处理上位机人脸注册指令
 */
void ClientThread::packageHandler_facialRegister(quint32 len, quint32 crc)
{
    QByteArray getData = socket->read(len);
    QString err("数据格式错误");
    int success = 0;

    if(!getData.isEmpty())
    {
        qDebug() << "data:" << getData;
        if(JQChecksum :: crc32(getData) != crc)
        {
            qDebug() << "crc not match, get " << JQChecksum :: crc32(getData) << "but we expect " << crc;
            err = "CRC校验错误";
        }

        bool ok;
        QJson::Parser parser;
        QVariantMap result = parser.parse(getData, &ok).toMap();

        if (!ok) {
            qFatal("An error occurred during parsing");
            return;
        }

        if(result["姓名"].toString().length() <= 0 ||
                result["工号"].toString().length() <= 0 ||
                result["部门"].toString().length() <= 0 ||
                result["单位"].toString().length() <= 0 ||
                result["证件号"].toString().length() <= 0)
        {
            err = "员工信息不全";
        }
        else
        {
            int len = 0;

//            bzero(facialFeatureBuff, sizeof(facialFeatureBuff));
            // 姓名暂时不转，只用工号识别员工
            int ret = dealRegisterCmd(result["工号"].toString().toLatin1().data(),
                    "", NULL, NULL);

            if(0 == ret)
            {
                dealGetUserFeatureCmd(result["工号"].toString().toLatin1().data(),
                        NULL, &len);
                qDebug() << "len :" << len;

                if(len > 0)
                {
                    success = 1;
                }
            }
        }


    }

    // send response to client
     MSG_HEADER header;
     header.mHFlag = PACKAGE_HEADER_FLAG;
     header.mFFlag = COMMAND_FACIAL_REGISTER;
     header.mVersion = PACKAGE_VERSION;

     QVariantMap jsonMap;


     QByteArray fileData;
     if(len > 0)
     {
         QFile file("FeatureTest.dat");
         file.open(QIODevice::ReadOnly);

         fileData = file.readAll();
         qDebug() << "fileData len:" << fileData.length();
     }

     QString str;
     if(len > 0)
     {
         str = QString::fromLatin1(fileData.data(), fileData.length());
     }

     qDebug() << "str len:" << str.length();
     jsonMap.insert("结果", success == 1 ? "1":"0");
     jsonMap.insert("数据", success == 1 ? str : err);
     QJson::Serializer serializer;
     QByteArray json = serializer.serialize(jsonMap);
     qDebug() << "json len:" << json.length();

     header.mCrc = JQChecksum :: crc32(json);
     header.mLength = json.length();
     socket->write((char *)&header, sizeof(header));
     socket->write(json);
     socket->waitForBytesWritten();
}


void ClientThread::packageHandler_fingerRegister(quint32 len, quint32 crc)
{
    QByteArray getData = socket->read(len);
    QString err("数据格式错误");
    int success = 0;

    if(!getData.isEmpty())
    {
        qDebug() << "data:" << getData;
        if(JQChecksum :: crc32(getData) != crc)
        {
            qDebug() << "crc not match, get " << JQChecksum :: crc32(getData) << "but we expect " << crc;
            err = "CRC校验错误";
        }

        bool ok;
        QJson::Parser parser;
        QVariantMap result = parser.parse(getData, &ok).toMap();

        if (!ok) {
            qFatal("An error occurred during parsing");
            return;
        }

        if(result["姓名"].toString().length() <= 0 ||
                result["工号"].toString().length() <= 0 ||
                result["部门"].toString().length() <= 0 ||
                result["单位"].toString().length() <= 0 ||
                result["证件号"].toString().length() <= 0)
        {
            err = "员工信息不全";
        }
        else
        {
            success = 1;
        }

    }

    // send response to client
     MSG_HEADER header;
     header.mHFlag = PACKAGE_HEADER_FLAG;
     header.mFFlag = COMMAND_FINGER_REGISTER;
     header.mVersion = PACKAGE_VERSION;

     QVariantMap jsonMap;

     jsonMap.insert("结果", success == 1 ? "1":"0");
     jsonMap.insert("数据", success == 1 ? "4D 53 47 5F 48 45 41 44 45 52 20 68 65 61 64 65 72 3B 0D 0A 20 20 20 20 20 68 65 61 64 65 72 2E 6D 48 46 6C 61 67 20 3D 20 50 41 43 4B 41 47 45 5F 48 45 41 44 45 52 5F 46 4C 41 47 3B 0D 0A 20 20 20 20 20 68 65 61 64 65 72 2E 6D 46 46 6C 61 67 20 3D 20 43 4F 4D 4D 41 4E 44 5F 46 49 4E 47 45 52 5F 52 45 47 49 53 54 45 52 3B 0D 0A 20 20 20 20 20 68 65 61 64 65 72 2E 6D 56 65 72 73 69 6F 6E 20 3D 20 50 41 43 4B 41 47 45 5F 56 45 52 53 49 4F 4E 3B "
                                       : err);
     QJson::Serializer serializer;
     QByteArray json = serializer.serialize(jsonMap);
     qDebug() << json;

     header.mCrc = JQChecksum :: crc32(json);
     header.mLength = json.length();
     socket->write((char *)&header, sizeof(header));
     socket->write(json);
     socket->waitForBytesWritten();
}


void ClientThread::packageHandler_downloadUser(quint32 len, quint32 crc)
{
    QByteArray getData = socket->read(len);
    QString err("数据格式错误");
    int success = 0;

    if(!getData.isEmpty())
    {
        qDebug() << "data:" << getData;
        if(JQChecksum :: crc32(getData) != crc)
        {
            qDebug() << "crc not match, get " << JQChecksum :: crc32(getData) << "but we expect " << crc;
            err = "CRC校验错误";
        }

        bool ok;
        QJson::Parser parser;
        QVariantMap result = parser.parse(getData, &ok).toMap();

        if (!ok) {
            qFatal("An error occurred during parsing");
            return;
        }

        if(result["姓名"].toString().length() <= 0 ||
                result["工号"].toString().length() <= 0 ||
                result["部门"].toString().length() <= 0 ||
                result["单位"].toString().length() <= 0 ||
                result["证件号"].toString().length() <= 0 ||
                result["人脸特征"].toString().length() <= 0 ||
                result["指纹特征"].toString().length() <= 0)
        {
            err = "员工信息不全";
        }
        else
        {
            success = 1;
        }
    }

    // send response to client
     MSG_HEADER header;
     header.mHFlag = PACKAGE_HEADER_FLAG;
     header.mFFlag = COMMAND_DOWNLOAD_USER;
     header.mVersion = PACKAGE_VERSION;

     QVariantMap jsonMap;

     jsonMap.insert("结果", success == 1 ? "1":"0");
     jsonMap.insert("数据", success == 1 ? "": err);
     QJson::Serializer serializer;
     QByteArray json = serializer.serialize(jsonMap);
     qDebug() << json;

     header.mCrc = JQChecksum :: crc32(json);
     header.mLength = json.length();
     socket->write((char *)&header, sizeof(header));
     socket->write(json);
     socket->waitForBytesWritten();
}


void ClientThread::packageHandler_delUser(quint32 len, quint32 crc)
{
    QByteArray getData = socket->read(len);
    QString err("数据格式错误");
    int success = 0;

    if(!getData.isEmpty())
    {
        qDebug() << "data:" << getData;
        if(JQChecksum :: crc32(getData) != crc)
        {
            qDebug() << "crc not match, get " << JQChecksum :: crc32(getData) << "but we expect " << crc;
            err = "CRC校验错误";
        }

        bool ok;
        QJson::Parser parser;
        QVariantMap result = parser.parse(getData, &ok).toMap();

        if (!ok) {
            qFatal("An error occurred during parsing");
            return;
        }

        if(result["工号"].toString().length() <= 0 )
        {
            err = "工号不能为空";
        }
        else
        {
            success = 1;
        }
    }

    // send response to client
     MSG_HEADER header;
     header.mHFlag = PACKAGE_HEADER_FLAG;
     header.mFFlag = COMMAND_DEL_USER;
     header.mVersion = PACKAGE_VERSION;

     QVariantMap jsonMap;

     jsonMap.insert("结果", success == 1 ? "1":"0");
     jsonMap.insert("数据", success == 1 ? "" : err);
     QJson::Serializer serializer;
     QByteArray json = serializer.serialize(jsonMap);
     qDebug() << json;

     header.mCrc = JQChecksum :: crc32(json);
     header.mLength = json.length();
     socket->write((char *)&header, sizeof(header));
     socket->write(json);
     socket->waitForBytesWritten();
}


void ClientThread::packageHandler_getTemp(quint32 len, quint32 crc)
{
    QByteArray getData = socket->read(len);
    QString err("数据格式错误");
    int success = 0;

    if(!getData.isEmpty())
    {
        qDebug() << "data:" << getData;
        if(JQChecksum :: crc32(getData) != crc)
        {
            qDebug() << "crc not match, get " << JQChecksum :: crc32(getData) << "but we expect " << crc;
            err = "CRC校验错误";
        }
        else
        {
            bool ok;
            QJson::Parser parser;
            QVariantMap result = parser.parse(getData, &ok).toMap();

            if (!ok) {
                qFatal("An error occurred during parsing");
                return;
            }

            success =1;
        }
    }

    // send response to client
     MSG_HEADER header;
     header.mHFlag = PACKAGE_HEADER_FLAG;
     header.mFFlag = COMMAND_GET_TEMPERATURE;
     header.mVersion = PACKAGE_VERSION;

     QVariantMap jsonMap;

     jsonMap.insert("结果", success == 1 ? "1":"0");
     jsonMap.insert("数据", success == 1 ? "36.2" : err);
     QJson::Serializer serializer;
     QByteArray json = serializer.serialize(jsonMap);
     qDebug() << json;

     header.mCrc = JQChecksum :: crc32(json);
     header.mLength = json.length();
     socket->write((char *)&header, sizeof(header));
     socket->write(json);
     socket->waitForBytesWritten();
}

