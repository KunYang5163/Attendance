#include "maindialog.h"
#include <QApplication>
#include <QTextCodec>
#include <QDebug>
#include <QDesktopWidget>
#include <QMainWindow>
#include <QFontDatabase>
#include "fingerprint.h"
#include <QThread>
#ifdef Q_OS_ANDROID
#include <QtAndroidExtras/QAndroidJniObject>
#include <QtAndroid>
#endif


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    QTextCodec *codec=QTextCodec::codecForName("utf-8");//windows上一般是gbk，其他平台一般utf-8
//    QTextCodec::setCodecForLocale(codec);

//    QTextCodec::setCodecForTr(QTextCodec::codecForLocale());
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

//    QTextCodec::setCodecForTr(QTextCodec::codecForName("GB2312"));
//    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GB2312"));
//    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GB2312"));
//    QDesktopWidget* desktopWidget = QApplication::desktop();
//    //获取可用桌面大小
//    QRect deskRect = desktopWidget->availableGeometry();
//    //获取设备屏幕大小
//    QRect screenRect = desktopWidget->screenGeometry();
//    qDebug()<<deskRect<<screenRect;

    int fontId = QFontDatabase::addApplicationFont("assets:/msyh.ttf");//(":/font/simfang.ttf");
    QString fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
    QFont font(fontFamily,24,QFont::Normal);
    QApplication::setFont(font);

     mainDialog *md = &mainDialog::instance();

     md->setObjectName("mainDialog");
     md->setStyleSheet("QDialog#mainDialog{background-image:url(:/images/main_bg.png);}");
     md->move(0,0);
     md->resize(1280,800);

     md->show();

    return a.exec();
}
