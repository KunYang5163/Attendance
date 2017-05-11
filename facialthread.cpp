#include "facialthread.h"
#include "facial.h"

#include <QFile>
#include <QTextCodec>
#include <QTextStream>
#include <QDebug>
#include <QImage>
#include <QPixmap>

FacialThread::FacialThread(int userNum)
{
    userNumber = userNum;
}


void FacialThread :: run()
{
    emit sigChangeMsg("请进行人脸识别");

    QThread::yieldCurrentThread();

    QString str = QString::number(userNumber);
    QByteArray baNum = str.toLatin1();

    char *pUserNumber = baNum.data();

    char nameBuff[128] = {0};
    char cScore = 0;

    qDebug("userNum :%s", pUserNumber);
    int ret = dealFaceVerifyCmd(pUserNumber, nameBuff, &cScore);
    if(1 == ret)
    {
        emit sigChangeMsg("识别成功");
        QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
        QTextCodec *codec = QTextCodec::codecForName("GBK");

        QFile file(FILE_USER_INFO);
        file.open(QIODevice::ReadOnly);
        QString userInfo;

        QTextStream inputStream(&file);

        QByteArray ba;
        QString codeStr;
        while(1)
        {
            inputStream >> userInfo;
            qDebug() << userInfo;
            if(userInfo.isEmpty())
                break;

            ba = userInfo.toLocal8Bit();
            codeStr = codec->toUnicode(ba);
            QStringList strList = codeStr.split(":::");



            qDebug() << strList.value(1) << ":" << str;

            if(strList.value(1) == str)
            {
                currentUser.userName = strList.value(0);
                currentUser.userNumber = strList.value(1);
                currentUser.department = strList.value(2);
                currentUser.org = strList.value(3);
                currentUser.userID = strList.value(4);

//                bzero(facePicBuf, sizeof(facePicBuf));

                // get user picture
                QString fileName = QString(QLatin1String(pUserNumber)) + QString(".jpg");

                if(!currentUser.userPic.load(fileName)) //加载图像
                {
                    char facePicBuf[20 * 1024] = {0};
                    int iPicSize = 0;
                    int ret =  dealGetUserPicCmd(pUserNumber, facePicBuf, &iPicSize);

                    if(0 == ret)
                    {
                        currentUser.userPic.loadFromData((const uchar*)facePicBuf, iPicSize, "JPG");
                    }
                }


#if 0
                ret =  dealGetUserPicCmd(pUserNumber, facePicBuf, &iPicSize);
                if(0 == ret)
                {
                    qDebug() << "pic size:" << iPicSize;
//                    for(int i =0; i < iPicSize;i++)
//                    {
//                        qDebug("%02x ", facePicBuf[i]);
//                    }
                    QByteArray baPic(facePicBuf, iPicSize);
                    currentUser.userPic = baPic;
                    qDebug("size of ba:%d",baPic.length());
//                    QImage image;

////                    if(!image.loadFromData(QByteArray::baPic(base64Data), "PNG"))
//                    if(!image.loadFromData(baPic, "JPG"))
//                    {
//                        qDebug() << "failed to load image";
//                    }

//                    QPixmap p;
//                    // fill array with image
//                    if(!p.loadFromData(baPic,"JPG"))
//                    {
//                        qDebug() << "failed to load pixmap";
//                    }



//                    QFile file("111.jpg");
//                    file.open(QIODevice::WriteOnly);
//                    p.save(&file, "JPG");
//                    file.close();
//                    ui->lblFaceRecogInfo->setPixmap(p);
//                    ui->lblFaceRecogInfo->setText("");
//                    ui->lblFaceRecogInfo->show();

        //            ui->lblFaceRecogInfo->setVisible(true);
//                    faceRecogShow(true);
                }
                else if(1 == ret)
                {
                    qDebug() << "user not found " << str;

//                    for (int i = 0; i < iPicSize; i++)
//                    {
//                        qDebug("%02x ", recvBuf[i]);
//                    }
                }
                else if(-3 == ret)
                {
                    qDebug() << "bad package: " << iPicSize;
                }
                else
                {
                    qDebug() << "failed to get pic for : " << str;
                }
#endif
                emit sigUpdateUserInfo(currentUser);
                break;
            }

        }

    }
    else if(2 == ret)
    {
        emit sigChangeMsg("未找到用户");
    }
    else
    {
        QString str = QString("人脸识别失败") + "["+ QString::number(ret) + "]";
        emit sigChangeMsg(str);

        if(-2 == ret)
        {
            for(int j = 0; j < 128; ++j)
                qDebug("%02x", nameBuff[j]);
        }
    }

}
