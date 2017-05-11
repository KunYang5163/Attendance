#include "maindialog.h"
#include "ui_maindialog.h"
#include <QPixmap>
#include <QTextCodec>
#include <QDebug>
#ifdef Q_OS_ANDROID
#include <QtAndroidExtras/QAndroidJniObject>
#include <QAndroidJniEnvironment>
#include <QtAndroid>
#endif
#include <QMessageBox>
#include <QTimer>
#include <QFile>
#include <QMouseEvent>
#include "fingerprint.h"
#include "stm32comm.h"
#include "healthtestthread.h"
#include "alcoholtestthread.h"
#include "alcoholadjustdialog.h"
#include "facialthread.h"
#include "devicemanager.h"


#include "JQChecksum.h"




// facial device
#include "facial.h"


using namespace cv;

//#pragma execution_character_set("utf-8")

#ifdef __cplusplus
extern "C" {
#endif
int start(void);
#ifdef __cplusplus
}
#endif

mainDialog::mainDialog(QWidget *parent) :
    QDialog(parent,Qt::FramelessWindowHint),
    ui(new Ui::mainDialog)
{
    ui->setupUi(this);
    tcpForClient.setParent(this);
//    initView();
    initNaviButtons();
    initUserInfoLabels();

    // open facial dev
    if(0 != openFacialDev())
    {
        showMessage("打开人脸设备失败");
    }

    //test
    //人脸识别
    initFaceRecogWidget();
    faceRecogShow(false);
    //指纹识别
    initFPRecogWidget();
    fpRecogShow(false);
    //酒精测试
    initAlcoholTestWidget1();
//    initAlcoholTestWidget2();
//    initAlcoholTestWidget3();
    alcoholTestShow(false);
    displayAlcoholTestresult(0);
    //健康测试
    initHealthTestWidget();
    healthTestShow(false, false);
    //注册1
    initUserReg1Widget();
    userReg1Show(true);
    userReg1ShowInputSign(InputName,InputSignNone);
    userReg1ShowInputSign(InputNumber,InputSignNone);
    userReg1ShowInputSign(InputDpt,InputSignNone);
    userReg1ShowInputSign(InputOrg,InputSignNone);
    userReg1ShowInputSign(InputId,InputSignNone);
    //注册2
    initUserReg2Widget();
    userReg2Show(false);
    //注册3
    initUserReg3Widget();
    userReg3Show(false);
    //设置按钮
    initSettingButtonsWidget();
    settingButtonsShow(false);
    //登录方式选择
    initLoginWayWidget();
    loginWayShow(false);
    //数据上传
    initDataUploadWidget1();
//    initDataUploadWidget2();
//    initDataUploadWidget3();
    dataUploadShow(false);
    //有线网络
    initEthWidget();
    ethShow(false);
    //wifi
    initWifiWidget();
    wifiShow(false);
    //volume
    initVolume();
    volumeShow(false);
    ui->sliderVolume->setValue(50);
    displayVolume(50);
    //device test
    initDevTest();
    devTestShow(false);


    // start a tcp server for client request
    tcpForClient.setLabel(ui->lbMessage);
    if(tcpForClient.listen(QHostAddress::Any, 9000))
    {
        qDebug("startListenning success!");
    }
    else
    {
        qDebug("startListenning false!");
    }

    // create devinfo.xml if it doesn't exist
    DeviceManager *devM = new DeviceManager();
    devM->createDefaultDevInfo();
    int ret;
    char status;
    // check device status
    devM->checkDevice(ret, status);
    delete devM;


    // connect to data server for uploading
    uploadThread = new UploadThread(this);
    connect(uploadThread,SIGNAL(sigShowMsg(QString)),this,SLOT(updateMessage(QString)),Qt::AutoConnection);
    uploadThread->start();

//    start();
//   V.open("0");
//  cvCreateCameraCapture
//   VideoCapture* capture = VideoCapture(0);

//    QString str1 = "1";
    QByteArray getData;
    getData[0] = 0x31;
    getData[1] = 0x32;
    getData[2] = 0x33;

    qDebug() << getData << ":" << getData.length();
    qDebug("crc32: %x", JQChecksum :: crc32(getData));

//    VideoCapture cap;
//    qDebug() << cap.isOpened();
////    if(!cap.isOpened())

//        try
//        {
//            for(int i = 0; i < 10; i++)
//            {
//            cap.open(i ); // call OpenCV
//            qDebug() << cap.isOpened();
//            }
//        }
//        catch( cv::Exception& e )
//        {
//            const char* err_msg = e.what();
//            qDebug() << "exception caught: " << err_msg ;
//        }
//    if(cap.isOpened())
//   if(NULL == capture)
//   {
//       showMessage("打开摄像头又失败");
//   }
//   else
   {
        qDebug() << "camera is opened";
//       IplImage* frame = cvQueryFrame(capture);
//       Mat im(frame);
//       cvtColor(im,im,CV_BGR2RGB);

//       QImage img((unsigned char*)im.data,im.cols,im.rows,QImage::Format_RGB888);

//       QPixmap pixmap = QPixmap::fromImage(img);
//       QSize picSize = ui->lblPic->size();
//       QPixmap scaledPixmap = pixmap.scaled(picSize);
//       ui->lblPic->setPixmap(scaledPixmap);
   }
//   if(!V.isOpened())
//   {
//       showMessage("打开摄像头失败");
//   }
//   else
//   {
//           QTimer *captureTimer= new QTimer(this);
//           connect(captureTimer, SIGNAL(timeout()), this, SLOT(takeImage()));
//           captureTimer->start(1000);
//   }

}

void mainDialog::takeImage()
{
     Mat im;
    V >> im;
    //图像格式转换
    cvtColor(im,im,CV_BGR2RGB);

    QImage img((unsigned char*)im.data,im.cols,im.rows,QImage::Format_RGB888);

    QPixmap pixmap = QPixmap::fromImage(img);
    QSize picSize = ui->lblPic->size();
    QPixmap scaledPixmap = pixmap.scaled(picSize);
    ui->lblPic->setPixmap(scaledPixmap);
}

mainDialog::~mainDialog()
{
    closeFacialDev();
    delete ui;
}


mainDialog &mainDialog::instance(QWidget *parent)
{
    static mainDialog mDialog(parent);
    return mDialog;
}

void mainDialog::initView()
{
    this->setStyleSheet("QPushButton{background-color: rgb(88, 186, 236);"
                        "color: rgb(255, 255, 255);"
                        "QLabel{background-color: rgb(112, 187, 221);"
                        "color: rgb(184, 176,164);}");
}

void mainDialog::initNaviButtons()
{
    mainActiveBtn = ActiveButtonNone;
    ui->btnAlcoholTest->resize(90,100);
    ui->btnAlcoholTest->move(400,663);
    ui->btnAlcoholTest->setStyleSheet("border-image:url(:/images/alcohol_test2.png)");

    ui->btnHealthTest->resize(90,100);
    ui->btnHealthTest->move(630,663);
    ui->btnHealthTest->setStyleSheet("border-image:url(:/images/health_test2.png)");

    ui->btnUserReg->resize(90,100);
    ui->btnUserReg->move(860,663);
    ui->btnUserReg->setStyleSheet("border-image:url(:/images/user_reg2.png)");

    ui->btnUserSetting->resize(90,100);
    ui->btnUserSetting->move(1090,663);
    ui->btnUserSetting->setStyleSheet("border-image:url(:/images/user_setting2.png)");
}

void mainDialog::initUserInfoLabels()
{
    ui->lblPic->resize(200, 260);
    ui->lblPic->move(50,90);
    ui->lblPic->setAttribute(Qt::WA_TranslucentBackground, true);


    ui->lblName->resize(170,30);
    ui->lblName->move(105,395);
    ui->lblName->setStyleSheet("color:rgb(255,255,255)");

    ui->lblNo->resize(170,30);
    ui->lblNo->move(105,450);
    ui->lblNo->setStyleSheet("color:rgb(255,255,255)");

    ui->lblDpt->resize(170,30);
    ui->lblDpt->move(105,505);
    ui->lblDpt->setStyleSheet("color:rgb(255,255,255)");

    ui->lblOrg->resize(170,30);
    ui->lblOrg->move(105,562);
    ui->lblOrg->setStyleSheet("color:rgb(255,255,255)");

    ui->lblId->resize(170,30);
    ui->lblId->move(105,619);
    ui->lblId->setStyleSheet("color:rgb(255,255,255)");

    ui->lbMessage->resize(600, 50);
    ui->lbMessage->move(480,610);
    ui->lbMessage->setAlignment(Qt::AlignHCenter);
    ui->lbMessage->setAttribute(Qt::WA_TranslucentBackground, true);
}

//人脸识别
void mainDialog::initFaceRecogWidget()
{
    ui->wgFaceRecog->resize(980,625);
    ui->wgFaceRecog->move(300,40);

    ui->lblFaceRecogInfo->resize(980,30);
    ui->lblFaceRecogInfo->move(0,180);
    ui->lblFaceRecogInfo->setStyleSheet("color: rgb(171, 171,171)");
    ui->lblFaceRecogInfo->setAlignment(Qt::AlignHCenter);

    ui->btnFaceRecogComfirm->resize(150,45);
    ui->btnFaceRecogComfirm->move(415,415);
    ui->btnFaceRecogComfirm->setStyleSheet("background-color: rgb(88, 186, 236);color: rgb(255, 255, 255)");
}

void mainDialog::faceRecogShow(bool show)
{
    ui->wgFaceRecog->setVisible(show);
}

void mainDialog::showFaceRecogInfo(const QString &info)
{
    ui->lblFaceRecogInfo->setText(info);
}

void mainDialog::on_btnFaceRecogComfirm_clicked()
{
    //
}

//指纹识别
void mainDialog::initFPRecogWidget()
{
    ui->wgFPRecog->resize(980,625);
    ui->wgFPRecog->move(300,40);

    ui->lblFPRecogBg->resize(200,320);//(200, 320)
    ui->lblFPRecogBg->move(390,100);//(390, 350)
    ui->lblFPRecogBg->setStyleSheet("border-image:url(:/images/fp_test.png)");

    ui->lblFPRecogInfo->resize(980,30);
    ui->lblFPRecogInfo->move(0,275);
    ui->lblFPRecogInfo->setStyleSheet("color: rgb(255, 255,255)");
    ui->lblFPRecogInfo->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    ui->btnFPRecogComfirm->resize(150,45);
    ui->btnFPRecogComfirm->move(415,500);
    ui->btnFPRecogComfirm->setStyleSheet("background-color: rgb(88, 186, 236);color: rgb(255, 255, 255)");
}

void mainDialog::fpRecogShow(bool show)
{
    ui->wgFPRecog->setVisible(show);
}

void mainDialog::showFPRecogInfo(const QString &info)
{
    ui->lblFPRecogInfo->setText(info);
}

void mainDialog::on_btnFPRecogComfirm_clicked()
{

}
//酒精测试
void mainDialog::initAlcoholTestWidget1()
{
    ui->wgAlcoholTest->resize(980,625);
    ui->wgAlcoholTest->move(300,40);
    ui->wgAlcoholTest->setAttribute(Qt::WA_TranslucentBackground,true);

    //step1
    ui->lblAlcoholTestBg->setVisible(true);
    ui->lblAlcoholTestBg->resize(980,45);
    ui->lblAlcoholTestBg->move(0,180);
    ui->lblAlcoholTestBg->setStyleSheet("border-image:url(:/images/alcohol_testing.png)");

    ui->lcdAlcoholTestNum->setVisible(true);
    ui->lcdAlcoholTestNum->resize(120,75);
    ui->lcdAlcoholTestNum->move(285,305);
    ui->lcdAlcoholTestNum->setStyleSheet("color: rgb(88, 186, 236);border-style:none;");
    ui->lcdAlcoholTestNum->setSegmentStyle(QLCDNumber::Flat);
    ui->lcdAlcoholTestNum->setDigitCount(4);

    ui->lblAlcoholTestUnit->setVisible(true);
    ui->lblAlcoholTestUnit->setAttribute(Qt::WA_TranslucentBackground,true);
    ui->lblAlcoholTestUnit->resize(150,240);
    ui->lblAlcoholTestUnit->move(110,230);
    ui->lblAlcoholTestUnit->setStyleSheet("color:rgb(255,255,255)");
    QFont font = ui->lblAlcoholTestUnit->font();
    font.setPointSize(46);
    ui->lblAlcoholTestUnit->setFont(font);
    ui->lblAlcoholTestBg->setVisible(false);

    ui->btnStartAlcoholTest->resize(120,75);
    ui->btnStartAlcoholTest->move(600,310);
    ui->btnStartAlcoholTest->setStyleSheet("background-color: rgb(88, 186, 236);color: rgb(255, 255, 255)");
    ui->btnStartAlcoholTest->setVisible(true);

    ui->lblAlcoholTestInfo->setAttribute(Qt::WA_TranslucentBackground, true);

    //step2
    ui->lblAlcoholTestInfo->setVisible(false);//step3 too
    ui->btnAlcoholTestRetest->setVisible(false);
    ui->btnAlcoholTestHear->setVisible(false);
    ui->btnAlcoholTestCancel->setVisible(false);
    //step3
    ui->btnAlcoholTestComfirm->setVisible(false);
}

void mainDialog::initAlcoholTestWidget2()
{
    ui->wgAlcoholTest->resize(980,625);
    ui->wgAlcoholTest->move(300,40);
    ui->wgAlcoholTest->setAttribute(Qt::WA_TranslucentBackground,true);

    //step1
    ui->lblAlcoholTestBg->setVisible(true);
    ui->lblAlcoholTestBg->resize(980,45);
    ui->lblAlcoholTestBg->move(0,180);
    ui->lblAlcoholTestBg->setStyleSheet("border-image:url(:/images/alcohol_testing.png)");

    ui->lcdAlcoholTestNum->setVisible(false);
    ui->lblAlcoholTestUnit->setVisible(false);

    //step2
    ui->lblAlcoholTestInfo->setVisible(true);
    ui->btnAlcoholTestRetest->setVisible(true);
    ui->btnAlcoholTestHear->setVisible(true);
    ui->btnAlcoholTestCancel->setVisible(true);

    ui->lblAlcoholTestInfo->resize(980,30);
    ui->lblAlcoholTestInfo->move(0,290);
    ui->lblAlcoholTestInfo->setStyleSheet("color:rgb(255,255,255)");
    ui->lblAlcoholTestInfo->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->lblAlcoholTestInfo->setAttribute(Qt::WA_TintedBackground,true);

    ui->btnAlcoholTestRetest->resize(150,45);
    ui->btnAlcoholTestRetest->move(225,520);
    ui->btnAlcoholTestRetest->setStyleSheet("background-color: rgb(88, 186, 236);color: rgb(255, 255, 255)");

    ui->btnAlcoholTestHear->resize(150,45);
    ui->btnAlcoholTestHear->move(415,520);
    ui->btnAlcoholTestHear->setStyleSheet("background-color: rgb(88, 186, 236);color: rgb(255, 255, 255)");

    ui->btnAlcoholTestCancel->resize(150,45);
    ui->btnAlcoholTestCancel->move(605,520);
    ui->btnAlcoholTestCancel->setStyleSheet("background-color: rgb(88, 186, 236);color: rgb(255, 255, 255)");
    //step3
    ui->btnAlcoholTestComfirm->setVisible(false);

}

void mainDialog::initAlcoholTestWidget3()
{
    ui->wgAlcoholTest->resize(980,625);
    ui->wgAlcoholTest->move(300,40);

    //step1
    ui->lblAlcoholTestBg->setVisible(false);
    ui->lblAlcoholTestBg->resize(980,240);
    ui->lblAlcoholTestBg->move(0,180);
    ui->lblAlcoholTestBg->setStyleSheet("border-image:url(:/images/alcohol_hear.png)");

    ui->lcdAlcoholTestNum->setVisible(false);
    ui->lblAlcoholTestUnit->setVisible(false);

    //step2
    ui->lblAlcoholTestInfo->setVisible(false);
    ui->btnAlcoholTestRetest->setVisible(false);
    ui->btnAlcoholTestHear->setVisible(false);
    ui->btnAlcoholTestCancel->setVisible(false);
    //step3
    ui->btnAlcoholTestComfirm->setVisible(true);
    ui->btnAlcoholTestComfirm->resize(150,45);
    ui->btnAlcoholTestComfirm->move(415,520);
    ui->btnAlcoholTestComfirm->setStyleSheet("background-color: rgb(88, 186, 236);color: rgb(255, 255, 255)");
}

void mainDialog::alcoholTestStart()
{
    ui->lblAlcoholTestBg->setVisible(false);
    ui->lcdAlcoholTestNum->setVisible(true);
    ui->lblAlcoholTestUnit->setVisible(true);
    ui->btnAlcoholTestRetest->setVisible(false);
    ui->btnAlcoholTestHear->setVisible(false);
    ui->btnAlcoholTestCancel->setVisible(false);
    ui->btnAlcoholTestComfirm->setVisible(false);

    currentUser.alcoholTest.clear();
    currentUser.health.sys.clear();
    currentUser.health.dia.clear();
    currentUser.health.pulse.clear();
    currentUser.health.bbt.clear();
    currentUser.userName.clear();
    currentUser.userID.clear();
    currentUser.userNumber.clear();
    currentUser.userPic = QImage();
    currentUser.department.clear();
    currentUser.org.clear();
    on_userInfoChanged(currentUser);

    onHealthTestSYSChanged(0);

    onHealthTestDIAChanged(0);

    onHealthTestPUISEChanged(0);





    ui->wgAlcoholTest->update();

    showMessage("请吹气测酒");
//    HealthTestThread *htt = new HealthTestThread(1);

//    connect(htt,SIGNAL(sigChangeHealthTestBBT(int)),this,SLOT(onHealthTestBBTChanged(int)),Qt::AutoConnection);

//    connect(htt, SIGNAL(finished()), htt, SLOT(deleteLater()));
//    connect(htt, SIGNAL(finished()), this, SLOT(alcolAndLogin()));

//    htt->start();

    AlcoholTestThread *att = new AlcoholTestThread();
    connect(att,SIGNAL(sigShowMsg(QString)),this,SLOT(updateMessage(QString)),Qt::AutoConnection);
    connect(att,SIGNAL(sigChangeAlcoholResult(int)),this,SLOT(onAlcoholResultChanged(int)),Qt::AutoConnection);
    connect(att, SIGNAL(finished()), att, SLOT(deleteLater()));
    att->start();


    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(loginIn()));
    timer->start(3000);

}

void mainDialog::alcolAndLogin()
{
    AlcoholTestThread *att = new AlcoholTestThread();
    connect(att,SIGNAL(sigShowMsg(QString)),this,SLOT(updateMessage(QString)),Qt::AutoConnection);
    connect(att,SIGNAL(sigChangeAlcoholResult(int)),this,SLOT(onAlcoholResultChanged(int)),Qt::AutoConnection);
    connect(att, SIGNAL(finished()), att, SLOT(deleteLater()));
    att->start();


    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(loginIn()));
    timer->start(3000);
}


void mainDialog::alcoholTestShow(bool show)
{
    ui->lcdAlcoholTestNum->display(0);
    ui->wgAlcoholTest->setVisible(show);
}

void mainDialog::displayAlcoholTestresult(int testResult)
{
    ui->lcdAlcoholTestNum->display(testResult);
}

void mainDialog::showAlcoholTestInfo(const QString &info)
{
    ui->lblAlcoholTestInfo->setText(info);
}


void mainDialog::showMessage(const QString &info)
{
    ui->lbMessage->setText(info);
}

void mainDialog::on_btnAlcoholTestRetest_clicked()
{

}

void mainDialog::on_btnAlcoholTestHear_clicked()
{

}

void mainDialog::on_btnAlcoholTestCancel_clicked()
{

}

void mainDialog::on_btnAlcoholTestComfirm_clicked()
{

}

void mainDialog :: onAlcoholResultChanged(int testResult)
{
    ui->lcdAlcoholTestNum->display(testResult);

    showMessage("测酒结束");
    ui->btnStartAlcoholTest->clearFocus();
    ui->btnStartAlcoholTest->setDisabled(false);

    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(switchToHealth()));
    timer->start(2000);
}

void mainDialog::switchToHealth()
{
//    alcoholTestShow(false);
    healthTestShow(true, false);

    mainActiveBtn = HealthTest;
    ui->btnAlcoholTest->setStyleSheet("border-image:url(:/images/alcohol_test2.png)");
    ui->btnHealthTest->setStyleSheet("border-image:url(:/images/health_test1.png)");

    ui->btnStartAlcoholTest->setVisible(false);
    ui->lcdAlcoholTestNum->setVisible(true);
    ui->lcdAlcoholTestNum->resize(120,70);
    ui->lcdAlcoholTestNum->move(380,30);
    ui->lcdAlcoholTestNum->setStyleSheet("color: rgb(88, 186, 236);border-style:none;");
    ui->lcdAlcoholTestNum->setSegmentStyle(QLCDNumber::Flat);
    ui->lcdAlcoholTestNum->setDigitCount(3);

    ui->lblAlcoholTestUnit->setVisible(true);
    ui->lblAlcoholTestUnit->resize(150,240);
    ui->lblAlcoholTestUnit->move(110,-60);
    ui->lblAlcoholTestUnit->setStyleSheet("color:rgb(255,255,255)");
    QFont font = ui->lblAlcoholTestUnit->font();
    font.setPointSize(36);
    ui->lblAlcoholTestUnit->setFont(font);

    showMessage("请把手指轻按到血压仪上");
    HealthTestThread *htt = new HealthTestThread(0);
    connect(htt,SIGNAL(sigShowMsg(QString)),this,SLOT(updateMessage(QString)),Qt::AutoConnection);
    connect(htt,SIGNAL(sigChangeHealthTestSYS(int)),this,SLOT(onHealthTestSYSChanged(int)),Qt::AutoConnection);
    connect(htt,SIGNAL(sigChangeHealthTestDIA(int)),this,SLOT(onHealthTestDIAChanged(int)),Qt::AutoConnection);
    connect(htt,SIGNAL(sigChangeHealthTestPUISE(int)),this,SLOT(onHealthTestPUISEChanged(int)),Qt::AutoConnection);
//    connect(htt,SIGNAL(sigChangeHealthTestBBT(int)),this,SLOT(onHealthTestBBTChanged(int)),Qt::AutoConnection);

    connect(htt, SIGNAL(finished()), htt, SLOT(deleteLater()));

    htt->start();

    showAlcoholTestInfo("");
}

//健康测试
void mainDialog::initHealthTestWidget()
{
    ui->wgHealthTest->resize(980,625);
    ui->wgHealthTest->move(300,40);
    ui->wgHealthTest->setAttribute(Qt::WA_TranslucentBackground,true);

    ui->lblHealthTest->resize(210,340);
    ui->lblHealthTest->move(100,115);
    ui->lblHealthTest->setStyleSheet("border-image:url(:/images/health_info.png)");

    ui->lcdSys->resize(120,70);
    ui->lcdSys->move(380,110);
    ui->lcdSys->setStyleSheet("color: rgb(88, 186, 236);border-style:none;");
    ui->lcdSys->setSegmentStyle(QLCDNumber::Flat);
    ui->lcdSys->setDigitCount(4);

    ui->lcdDia->resize(120,70);
    ui->lcdDia->move(380,205);
    ui->lcdDia->setStyleSheet("color: rgb(88, 186, 236);border-style:none;");
    ui->lcdDia->setSegmentStyle(QLCDNumber::Flat);
    ui->lcdDia->setDigitCount(4);

    ui->lcdPuise->resize(120,70);
    ui->lcdPuise->move(380,300);
    ui->lcdPuise->setStyleSheet("color: rgb(88, 186, 236);border-style:none;");
    ui->lcdPuise->setSegmentStyle(QLCDNumber::Flat);
    ui->lcdPuise->setDigitCount(4);

    ui->lcdBbt->resize(120,70);
    ui->lcdBbt->move(380,395);
    ui->lcdBbt->setStyleSheet("color: rgb(88, 186, 236);border-style:none;");
    ui->lcdBbt->setSegmentStyle(QLCDNumber::Flat);
    ui->lcdBbt->setDigitCount(4);

    ui->pbStartBP->resize(120,70);
    ui->pbStartBP->move(600, 205);
    ui->pbStartBP->setStyleSheet("background-color: rgb(88, 186, 236);color: rgb(255, 255, 255)");

    ui->pbStartTemp->resize(120,70);
    ui->pbStartTemp->move(600, 400);
    ui->pbStartTemp->setStyleSheet("background-color: rgb(88, 186, 236);color: rgb(255, 255, 255)");

}

void mainDialog::healthTestShow(bool show, bool btnShow)
{
    ui->wgHealthTest->setVisible(show);

    ui->pbStartBP->setVisible(btnShow);
//    ui->pbStartTemp->setVisible(btnShow);

}

void mainDialog::displayHealthTestSYS(int sys)
{
    ui->lcdSys->display(sys);
}

void mainDialog::displayHealthTestDIA(int dia)
{
    ui->lcdDia->display(dia);
}

void mainDialog::displayHealthTestPUISE(int puise)
{
    ui->lcdPuise->display(puise);
}

void mainDialog::displayHealthTestBBT(int bbt)
{
    ui->lcdBbt->display(bbt);
}

//注册1
void mainDialog::initUserReg1Widget()
{
    ui->wgUserReg1->resize(980,625);
    ui->wgUserReg1->move(300,40);
    ui->wgUserReg1->setAttribute(Qt::WA_TranslucentBackground, true);

    ui->leName->resize(400,50);
    ui->leName->move(290,90);

    ui->leName->setPlaceholderText(nameDefaultString);
    ui->leName->setStyleSheet("QLineEdit{"
                              "border: 1px solid rgb(162, 155, 144);"
                              "border-radius: 3px;"
                              "background-color: rgb(255, 255, 255, 0);"
                              "color:gray;"
                              "font-size: 24px;}");
    ui->lblNameSign->resize(40,50);
    ui->lblNameSign->move(690,90);
    ui->lblNameSign->setText("");

    ui->leNumber->resize(400,50);
    ui->leNumber->move(290,170);
    ui->leNumber->setPlaceholderText(numberDefaultString);
    ui->leNumber->setStyleSheet("QLineEdit{"
                              "border: 1px solid rgb(162, 155, 144);"
                              "border-radius: 3px;"
                              "background-color: rgb(255, 255, 255, 0);"
                              "color:gray;"
                              "font-size: 24px;}");
    ui->lblNumberSign->resize(40,50);
    ui->lblNumberSign->move(690,170);
    ui->lblNumberSign->setText("");


    ui->leDpt->resize(400,50);
    ui->leDpt->move(290,250);
    ui->leDpt->setPlaceholderText(dptDefaultString);
    ui->leDpt->setStyleSheet("QLineEdit{"
                              "border: 1px solid rgb(162, 155, 144);"
                              "border-radius: 3px;"
                              "background-color: rgb(255, 255, 255, 0);"
                              "color:gray;"
                              "font-size: 24px;}");
    ui->lblDptSign->resize(40,50);
    ui->lblDptSign->move(690,250);
    ui->lblDptSign->setText("");


    ui->leOrg->resize(400,50);
    ui->leOrg->move(290,330);
    ui->leOrg->setPlaceholderText(orgDefaultString);
    ui->leOrg->setStyleSheet("QLineEdit{"
                              "border: 1px solid rgb(162, 155, 144);"
                              "border-radius: 3px;"
                              "background-color: rgb(255, 255, 255, 0);"
                              "color:gray;"
                              "font-size: 24px;}");
    ui->lblOrgSign->resize(40,50);
    ui->lblOrgSign->move(690,330);
    ui->lblOrgSign->setText("");


    ui->leId->resize(400,50);
    ui->leId->move(290,410);
    ui->leId->setPlaceholderText(idDefaultString);
    ui->leId->setStyleSheet("QLineEdit{"
                              "border: 1px solid rgb(162, 155, 144);"
                              "border-radius: 3px;"
                              "background-color: rgb(255, 255, 255, 0);"
                              "color:gray;"
                              "font-size: 24px;}");
    ui->lblIdSign->resize(40,50);
    ui->lblIdSign->move(690,410);
    ui->lblIdSign->setText("");

    ui->btnUserReg1Next->resize(150,45);
    ui->btnUserReg1Next->move(415,500);
    ui->btnUserReg1Next->setStyleSheet("background-color: rgb(88, 186, 236);color: rgb(255, 255, 255)");

    connect(ui->leName, SIGNAL(textChanged(const QString &)), this, SLOT(on_leNameTextChanged(const QString &)));
    connect(ui->leNumber, SIGNAL(textChanged(const QString &)), this, SLOT(on_leNumberTextChanged(const QString &)));
    connect(ui->leDpt, SIGNAL(textChanged(const QString &)), this, SLOT(on_leDptTextChanged(const QString &)));
    connect(ui->leOrg, SIGNAL(textChanged(const QString &)), this, SLOT(on_leOrgTextChanged(const QString &)));
    connect(ui->leId, SIGNAL(textChanged(const QString &)), this, SLOT(on_leIdTextChanged(const QString &)));
}

void mainDialog::userReg1Show(bool show)
{
    ui->wgUserReg1->setVisible(show);

    currentUser.alcoholTest.clear();
    currentUser.health.sys.clear();
    currentUser.health.dia.clear();
    currentUser.health.pulse.clear();
    currentUser.health.bbt.clear();
    currentUser.userName.clear();
    currentUser.userID.clear();
    currentUser.userNumber.clear();
    currentUser.userPic = QImage();
    currentUser.department.clear();
    currentUser.org.clear();
    on_userInfoChanged(currentUser);

    showMessage("");
}

bool mainDialog::userReg1CheckInput()
{
    if(0 == ui->leName->text().length())
    {
        ui->leName->setFocus();
        return false;
    }
    if(0 == ui->leNumber->text().length())
    {
        ui->leNumber->setFocus();
        return false;
    }
    if(0 == ui->leDpt->text().length())
    {
        ui->leDpt->setFocus();
        return false;
    }
    if(0 == ui->leOrg->text().length())
    {
        ui->leOrg->setFocus();
        return false;
    }
    if(0 == ui->leId->text().length())
    {
        ui->leId->setFocus();
        return false;
    }

    return true;
}

void mainDialog::on_btnUserReg1Next_clicked()
{
    if(userReg1CheckInput())
    {
        userReg1Show(false);
        userReg2Show(true);
//        userReg3Show(true);
    }
}

void mainDialog::on_leNameTextChanged(const QString &text)
{
    if(text.length() >= 1 && text.length() <= 8)
    {
        userReg1ShowInputSign(InputName,InputSignOk);
    }
    else
    {
        userReg1ShowInputSign(InputName,InputSignErr);
    }
}
void mainDialog::on_leIdTextChanged(const QString &text)
{
    if(text.length() >= 1 && text.length() <= 8)
    {
        userReg1ShowInputSign(InputId,InputSignOk);
    }
    else
    {
        userReg1ShowInputSign(InputId,InputSignErr);
    }
}

void mainDialog::on_leNumberTextChanged(const QString &text)
{
    if(text.length() >= 1 && text.length() <= 8)
    {
        userReg1ShowInputSign(InputNumber,InputSignOk);
    }
    else
    {
        userReg1ShowInputSign(InputNumber,InputSignErr);
    }
}

void mainDialog::on_leDptTextChanged(const QString &text)
{
    if(text.length() >= 1 && text.length() <= 8)
    {
        userReg1ShowInputSign(InputDpt,InputSignOk);
    }
    else
    {
        userReg1ShowInputSign(InputDpt,InputSignErr);
    }
}

void mainDialog::on_leOrgTextChanged(const QString &text)
{
    if(text.length() >= 1 && text.length() <= 8)
    {
        userReg1ShowInputSign(InputOrg,InputSignOk);
    }
    else
    {
        userReg1ShowInputSign(InputOrg,InputSignErr);
    }
}


void mainDialog::userReg1ShowInputSign(UserRegInput input, UserRegInputSign sign)
{
    QString borderImage;
    if(sign == InputSignOk)
        borderImage = "border-image:url(:/images/input_ok.png)";
    else if(sign == InputSignErr)
        borderImage = "border-image:url(:/images/input_err.png)";
    else
        borderImage = "";

    switch (input) {
    case InputName:
        if(borderImage == "")
        {
            ui->lblNameSign->setVisible(false);
        }
        else
        {
            ui->lblNameSign->setVisible(true);
            ui->lblNameSign->setStyleSheet(borderImage);
        }
        break;
    case InputNumber:
        if(borderImage == "")
        {
            ui->lblNumberSign->setVisible(false);
        }
        else
        {
            ui->lblNumberSign->setVisible(true);
            ui->lblNumberSign->setStyleSheet(borderImage);
        }

        break;
    case InputDpt:
        if(borderImage == "")
        {
            ui->lblDptSign->setVisible(false);
        }
        else
        {
            ui->lblDptSign->setVisible(true);
            ui->lblDptSign->setStyleSheet(borderImage);
        }

        break;
    case InputOrg:
        if(borderImage == "")
        {
            ui->lblOrgSign->setVisible(false);
        }
        else
        {
            ui->lblOrgSign->setVisible(true);
            ui->lblOrgSign->setStyleSheet(borderImage);
        }

        break;
    case InputId:
        if(borderImage == "")
        {
            ui->lblIdSign->setVisible(false);
        }
        else
        {
            ui->lblIdSign->setVisible(true);
            ui->lblIdSign->setStyleSheet(borderImage);
        }

        break;
    default:
        break;
    }
}

//注册2
void mainDialog::initUserReg2Widget()
{
    ui->wgUserReg2->resize(980,625);
    ui->wgUserReg2->move(300,40);
    ui->wgUserReg2->setAttribute(Qt::WA_TranslucentBackground, true);

    ui->lblUserReg2Info->resize(200,320);
    ui->lblUserReg2Info->move(390,100);
    ui->lblUserReg2Info->setStyleSheet("border-image:url(:/images/camera.png)");

    ui->btnUserReg2Next->resize(150,45);
    ui->btnUserReg2Next->move(415,500);
    ui->btnUserReg2Next->setStyleSheet("background-color: rgb(88, 186, 236);color: rgb(255, 255, 255)");
}

void mainDialog::userReg2Show(bool show)
{
    ui->wgUserReg2->setVisible(show);

    ui->btnUserReg2Next->setDisabled(true);
    if(show)
    {
//        on_facialReg();
        QTimer *timer = new QTimer(this);
        timer->setSingleShot(true);
        connect(timer, SIGNAL(timeout()), this, SLOT(on_facialReg()));
        timer->start(500);
    }
}

void mainDialog::on_facialReg()
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));


    char  *pUserNum, *pUserName;

    QByteArray ba = ui->leNumber->text().toLocal8Bit();

    pUserNum = ba.data();

    ba = ui->leName->text().toLocal8Bit();

    pUserName = ba.data();

    char buff[1024] = {0};
    int len = 0;
    int i = dealRegisterCmd(pUserNum, pUserName, buff, &len);

//    for(int j = 0; j < len; ++j)
//    {
//        qDebug("%02x ", buff[j]);
//    }

    if(0 == i)
    {
        qDebug() << "bf get pic";
        char facePicBuf[20 * 1024] = {0};
        int iPicSize = 0;
        int retry = 2;
getPic:
        int ret =  dealGetUserPicCmd(pUserNum, facePicBuf, &iPicSize);
        qDebug() << "ret:" << ret;
        if(0 == ret)
        {
            QByteArray baPic(facePicBuf, iPicSize);

            QPixmap p;
            // fill array with image
            if(p.loadFromData(baPic,"JPG"))
            {
                qDebug() << "failed to load pixmap" << "pic size:" << iPicSize;

                QString fileName = QString(ba) + QString(".jpg");
                qDebug() << "file Name:" << fileName;
                QFile file(fileName);
                file.open(QIODevice::WriteOnly);
                p.save(&file, "JPG");
                file.close();
            }
            else
            {
                if(retry-- > 0)
                    goto getPic;
            }
        }
        showMessage("人脸注册成功");
        ui->btnUserReg2Next->setDisabled(false);
    }
    else if(-2 == i)
    {
        showMessage("注册人数已满");
    }
    else if(-3 == i)
    {
        showMessage("此人已注册");
    }
    else if(-4 == i)
    {
        showMessage("注册人脸超时");
    }
    else if(-5 == i)
    {
        showMessage("人脸机未响应");
    }


    qDebug() << "i:" << i;
}

void mainDialog::on_btnUserReg2Next_clicked()
{
    userReg2Show(false);
    userReg3Show(true);
}

//注册3
void mainDialog::initUserReg3Widget()
{
    ui->wgUserReg3->resize(980,625);
    ui->wgUserReg3->move(300,40);
    ui->wgUserReg3->setAttribute(Qt::WA_TranslucentBackground, true);

    ui->lblUserReg3Info->resize(200,320);
    ui->lblUserReg3Info->move(390,100);
    ui->lblUserReg3Info->setStyleSheet("border-image:url(:/images/finger_print.png)");

    ui->btnUserReg3Next->resize(150,45);
    ui->btnUserReg3Next->move(415,500);
    ui->btnUserReg3Next->setStyleSheet("background-color: rgb(88, 186, 236);color: rgb(255, 255, 255)");
}

void mainDialog:: onNotifyEnroll(const int count, const QString temp)
{
    QString num =  QString::number(3 - count);
    qDebug("mainDialog onNotifyQt is invoked!!!!!!!!!!!!!!!!!%d, %s", count, num.toLatin1().data());
    qDebug() << "temp len:" << temp.length();
    QString str = "识别出错";
    if(count > -3 && count < 0)
    {
        str = "请按" + QString::number(3 + count) + "次指纹";
    }
    else if(-3 == count)
    {
        str = "用户注册成功";
        ui->btnUserReg3Next->setVisible(true);

        // save to db
        UserInfo userInfo;

        userInfo.userID = ui->leId->text();
        userInfo.userName = ui->leName->text();
        userInfo.userNumber = ui->leNumber->text();
        userInfo.org = ui->leOrg->text();
        userInfo.department = ui->leDpt->text();
        userInfo.userFP = temp;

        dbManager.AddUserInfo(userInfo);


        QFile file(FILE_USER_INFO);
        file.open(QIODevice::WriteOnly | QIODevice::Append);

        QTextStream txtOutput(&file);
        QString strToSave = ui->leName->text() + ":::" + ui->leNumber->text() + ":::" +
                ui->leDpt->text() + ":::" + ui->leOrg->text() + ":::" + ui->leId->text();
        txtOutput << strToSave << endl;
        file.close();

        ui->leName->setText("");
        ui->leId->setText("");
        ui->leOrg->setText("");
        ui->leDpt->setText("");
        ui->leNumber->setText("");
    }
    else if(count >= 0)
    {
        str = "已注册为" +  QString::number(count);
         ui->btnUserReg3Next->setVisible(true);
    }
    showMessage(str);

}

void mainDialog:: onNotifyVerify(const int userNum)
{
    qDebug("mainDialog onNotifyVerify is invoked!!!!!!!!!!!!!!!!!%d", userNum);

    if(userNum > 0)
    {
        qRegisterMetaType<UserInfo>("UserInfo");
        FacialThread *ft = new FacialThread(userNum);

        connect(ft,SIGNAL(sigShowMsg(const QString)),this,SLOT(showAlcoholTestInfo(QString)),Qt::AutoConnection);
        connect(ft,SIGNAL(sigUpdateUserInfo(const UserInfo)),this,SLOT(on_userInfoChanged(const UserInfo)),Qt::AutoConnection);
        connect(ft, SIGNAL(finished()), ft, SLOT(deleteLater()));

        ft->start();
    }
}

void mainDialog :: startFingerPrintEnroll(int userNumber)
{
    ui->btnUserReg3Next->setVisible(false);
    showMessage("请按3次指纹");

    QtAndroid::androidActivity().callMethod<void>("OnEnroll", "(I)V", userNumber);

}

void mainDialog::userReg3Show(bool show)
{
    ui->wgUserReg3->setVisible(show);

    if(show)
    {
        int userNumber = ui->leNumber->text().toInt();
        startFingerPrintEnroll(userNumber);
    }

}

void mainDialog::on_btnUserReg3Next_clicked()
{
    userReg3Show(false);
    userReg1Show(true);
}


//设置按钮
void mainDialog::initSettingButtonsWidget()
{
    ui->wgSettingButtons->resize(160,415);
    ui->wgSettingButtons->move(300,90);

//    ui->wgSettingButtons->setStyleSheet("border-image:url(:/images/setting_buttons.png)");

//    signalMapper = new QSignalMapper();

    ui->btnLoginWay->resize(160,45);
    ui->btnLoginWay->move(0,0);
    ui->btnLoginWay->setStyleSheet("background-color: rgb(255, 255, 255,0);color: rgb(88, 186, 236);"
                                   "border-image:url(:/images/setting_button.png)");
//    connect(ui->btnLoginWay,SIGNAL(clicked()),signalMapper,SLOT(map()));
//    signalMapper->setMapping(ui->btnLoginWay,SettingLoginWay);

    currentSettingType = SettingLoginWay;

    ui->btnDataUpload->resize(160,45);
    ui->btnDataUpload->move(0,74);
    ui->btnDataUpload->setStyleSheet("background-color: rgb(255, 255, 255,0);color:rgb(255, 255, 255);"
                                     "border-image:url(:/images/setting_button.png)");
//    connect(ui->btnDataUpload,SIGNAL(clicked()),signalMapper,SLOT(map()));
//    signalMapper->setMapping(ui->btnDataUpload,SettingDataUpload);

    ui->btnEth->resize(160,45);
    ui->btnEth->move(0,148);
    ui->btnEth->setStyleSheet("background-color: rgb(255, 255, 255,0);color:rgb(255, 255, 255);"
                              "border-image:url(:/images/setting_button.png)");
//    connect(ui->btnEth,SIGNAL(clicked()),signalMapper,SLOT(map()));
//    signalMapper->setMapping(ui->btnEth,SettingEth);

    ui->btnWifi->resize(160,45);
    ui->btnWifi->move(0,222);
    ui->btnWifi->setStyleSheet("background-color: rgb(255, 255, 255,0);color:rgb(255, 255, 255);"
                               "border-image:url(:/images/setting_button.png)");
//    connect(ui->btnWifi,SIGNAL(clicked()),signalMapper,SLOT(map()));
//    signalMapper->setMapping(ui->btnWifi,SettingWifi);

    ui->btnVolume->resize(160,45);
    ui->btnVolume->move(0,296);
    ui->btnVolume->setStyleSheet("background-color: rgb(255, 255, 255,0);color:rgb(255, 255, 255);"
                                 "border-image:url(:/images/setting_button.png)");
//    connect(ui->btnVolume,SIGNAL(clicked()),signalMapper,SLOT(map()));
//    signalMapper->setMapping(ui->btnVolume,SettingVolume);

    ui->btnSelfTest->resize(160,45);
    ui->btnSelfTest->move(0,370);
    ui->btnSelfTest->setStyleSheet("background-color: rgb(255, 255, 255,0);color:rgb(255, 255, 255);"
                                   "border-image:url(:/images/setting_button.png)");
//    connect(ui->btnSelfTest,SIGNAL(clicked()),signalMapper,SLOT(map()));
//    signalMapper->setMapping(ui->btnSelfTest,SettingSelfTest);

//    connect(signalMapper,SIGNAL(mapped(const SettingType)),this,SLOT(on_settingButtons_clicked(const SettingType)));

}

void mainDialog::settingButtonsShow(bool show)
{
    ui->wgSettingButtons->setVisible(show);
    if(show)
    {
        clearPreActiveButtonColor();
        currentSettingType = SettingLoginWay;
        showSettingWidget();
    }
}

void mainDialog::clearPreActiveButtonColor()
{
    switch (currentSettingType)
    {
    case SettingLoginWay:
        ui->btnLoginWay->setStyleSheet("background-color: rgb(255, 255, 255,0);color:rgb(255, 255, 255);"
                               "border-image:url(:/images/setting_button.png)");
        loginWayShow(false);

        break;
    case SettingDataUpload:
        dataUploadShow(false);
        ui->btnDataUpload->setStyleSheet("background-color: rgb(255, 255, 255,0);color:rgb(255, 255, 255);"
                               "border-image:url(:/images/setting_button.png)");

        break;
    case SettingEth:
        ethShow(false);
        ui->btnEth->setStyleSheet("background-color: rgb(255, 255, 255,0);color:rgb(255, 255, 255);"
                               "border-image:url(:/images/setting_button.png)");

        break;
    case SettingWifi:
        wifiShow(false);
        ui->btnWifi->setStyleSheet("background-color: rgb(255, 255, 255,0);color:rgb(255, 255, 255);"
                               "border-image:url(:/images/setting_button.png)");

        break;
    case SettingVolume:
        volumeShow(false);
        ui->btnVolume->setStyleSheet("background-color: rgb(255, 255, 255,0);color:rgb(255, 255, 255);"
                               "border-image:url(:/images/setting_button.png)");

        break;
    case SettingSelfTest:
        ui->btnSelfTest->setStyleSheet("background-color: rgb(255, 255, 255,0);color:rgb(255, 255, 255);"
                               "border-image:url(:/images/setting_button.png)");
        devTestShow(false);

        break;
    default:
        break;
    }
}

void mainDialog::showSettingWidget()
{
    switch (currentSettingType)
    {
    case SettingLoginWay:
        loginWayShow(true);

        break;
    case SettingDataUpload:
        dataUploadShow(true);

        break;
    case SettingEth:
        ethShow(true);

        break;
    case SettingWifi:


        break;
    case SettingVolume:
        volumeShow(true);

        break;
    case SettingSelfTest:
        devTestShow(true);

        break;
    default:
        break;
    }
}

void mainDialog::on_btnLoginWay_clicked()
{
    if(currentSettingType == SettingLoginWay)
        return;

    ui->btnLoginWay->setStyleSheet("background-color: rgb(255, 255, 255,0);color: rgb(88, 186, 236);"
                                   "border-image:url(:/images/setting_button.png)");
    clearPreActiveButtonColor();

    currentSettingType = SettingLoginWay;
    showSettingWidget();
}

void mainDialog::on_btnDataUpload_clicked()
{
    if(currentSettingType == SettingDataUpload)
        return;

    ui->btnDataUpload->setStyleSheet("background-color: rgb(255, 255, 255,0);color: rgb(88, 186, 236);"
                                   "border-image:url(:/images/setting_button.png)");
    clearPreActiveButtonColor();

    currentSettingType = SettingDataUpload;
    showSettingWidget();
}

void mainDialog::on_btnEth_clicked()
{
    if(currentSettingType == SettingEth)
        return;

    ui->btnEth->setStyleSheet("background-color: rgb(255, 255, 255,0);color: rgb(88, 186, 236);"
                                   "border-image:url(:/images/setting_button.png)");
    clearPreActiveButtonColor();

    currentSettingType = SettingEth;
    showSettingWidget();
}

void mainDialog::on_btnWifi_clicked()
{
    return ;
    if(currentSettingType == SettingWifi)
        return;

    ui->btnWifi->setStyleSheet("background-color: rgb(255, 255, 255,0);color: rgb(88, 186, 236);"
                                   "border-image:url(:/images/setting_button.png)");
    clearPreActiveButtonColor();

    currentSettingType = SettingWifi;
    showSettingWidget();

}

void mainDialog::on_btnVolume_clicked()
{
    if(currentSettingType == SettingVolume)
        return;

    ui->btnVolume->setStyleSheet("background-color: rgb(255, 255, 255,0);color: rgb(88, 186, 236);"
                                   "border-image:url(:/images/setting_button.png)");
    clearPreActiveButtonColor();

    currentSettingType = SettingVolume;
    showSettingWidget();
}

void mainDialog::on_btnSelfTest_clicked()
{
    if(currentSettingType == SettingSelfTest)
        return;

//    hideAll();
    ui->btnSelfTest->setStyleSheet("background-color: rgb(255, 255, 255,0);color: rgb(88, 186, 236);"
                                   "border-image:url(:/images/setting_button.png)");
    clearPreActiveButtonColor();

    currentSettingType = SettingSelfTest;
    showSettingWidget();
}

//登录方式
void mainDialog::initLoginWayWidget()
{
    ui->wgLoginWay->resize(820,625);
    ui->wgLoginWay->move(460,40);
    ui->wgLoginWay->setAttribute(Qt::WA_TranslucentBackground, true);

    ui->lblLoginWaySelect->resize(660,40);
    ui->lblLoginWaySelect->move(0,100);
    ui->lblLoginWaySelect->setStyleSheet("border-image:url(:/images/login_way_select.png)");

    ui->rbLoginWayFace->resize(160,40);
    ui->rbLoginWayFace->move(100,220);
    ui->rbLoginWayFace->setStyleSheet("QRadioButton {color: rgb(255, 255, 255);font-size: 26px}"
                                      "QRadioButton::indicator {width: 35px;height: 40px}"
                                      "QRadioButton::indicator:checked {image: url(:/images/radio_checked.png)}"
                                      "QRadioButton::indicator:unchecked {image: url(:/images/radio_unchecked.png)}"
                                      );
    ui->rbLoginWayFace->setChecked(true);

    ui->rbLoginWayFp->resize(160,40);
    ui->rbLoginWayFp->move(260,220);
    ui->rbLoginWayFp->setStyleSheet("QRadioButton {color: rgb(255, 255, 255);font-size: 26px}"
                                    "QRadioButton::indicator {width: 35px;height: 40px}"
                                    "QRadioButton::indicator:checked {image: url(:/images/radio_checked.png)}"
                                    "QRadioButton::indicator:unchecked {image: url(:/images/radio_unchecked.png)}"
                                    );

    ui->rbLoginWayFaceFp->resize(160,40);
    ui->rbLoginWayFaceFp->move(420,220);
    ui->rbLoginWayFaceFp->setStyleSheet("QRadioButton {color: rgb(255, 255, 255);font-size: 26px}"
                                        "QRadioButton::indicator {width: 35px;height: 40px}"
                                        "QRadioButton::indicator:checked {image: url(:/images/radio_checked.png)}"
                                        "QRadioButton::indicator:unchecked {image: url(:/images/radio_unchecked.png)}"
                                        );

    ui->btnLoginWayComfirm->resize(150,45);
    ui->btnLoginWayComfirm->move(255,500);
    ui->btnLoginWayComfirm->setStyleSheet("background-color: rgb(88, 186, 236);color: rgb(255, 255, 255)");
}

void mainDialog::loginWayShow(bool show)
{
    ui->wgLoginWay->setVisible(show);
}


//数据上传
void mainDialog::initDataUploadWidget1()
{
    ui->wgDataUpload->resize(820,625);
    ui->wgDataUpload->move(460,40);
    ui->wgDataUpload->setAttribute(Qt::WA_TranslucentBackground, true);

    ui->lblDataUploadServerIP->resize(180,50);
    ui->lblDataUploadServerIP->move(140,50);
    ui->lblDataUploadServerIP->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->leServerIP->resize(220,50);
    ui->leServerIP->move(350,50);

    ui->lblDataUploadServerPort->resize(180,50);
    ui->lblDataUploadServerPort->move(140,110);
    ui->lblDataUploadServerPort->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->leServerPort->resize(220,50);
    ui->leServerPort->move(350,110);

    ui->btnDataUploadData->resize(150,45);
    ui->btnDataUploadData->move(200,500);
    ui->btnDataUploadData->setStyleSheet("background-color: rgb(88, 186, 236);color: rgb(255, 255, 255)");

    ui->btnDataUploadLog->resize(150,45);
    ui->btnDataUploadLog->move(400,500);
    ui->btnDataUploadLog->setStyleSheet("background-color: rgb(88, 186, 236);color: rgb(255, 255, 255)");
}
//上传成功
void mainDialog::initDataUploadWidget2()
{
#if 0
    ui->wgDataUpload->resize(820,625);
    ui->wgDataUpload->move(460,40);

    ui->lblDataUploadInfo->resize(660,45);
    ui->lblDataUploadInfo->move(0,180);
    ui->lblDataUploadInfo->setStyleSheet("color:rgb(185,177,165);font-size: 26px");
    ui->lblDataUploadInfo->setAlignment(Qt::AlignHCenter);
    ui->lblDataUploadInfo->setText("数据上传成功!");

    ui->btnDataUploadComfirm->setVisible(true);
    ui->btnDataUploadComfirm->resize(150,45);
    ui->btnDataUploadComfirm->move(255,415);
    ui->btnDataUploadComfirm->setStyleSheet("background-color: rgb(88, 186, 236);color: rgb(255, 255, 255)");

    ui->btnDataUploadCancel->setVisible(false);
#endif
}
//上传失败
void mainDialog::initDataUploadWidget3()
{
#if 0
    ui->wgDataUpload->resize(820,625);
    ui->wgDataUpload->move(460,40);

    ui->lblDataUploadInfo->resize(660,45);
    ui->lblDataUploadInfo->move(0,180);
    ui->lblDataUploadInfo->setStyleSheet("color:rgb(185,177,165);font-size: 26px");
    ui->lblDataUploadInfo->setAlignment(Qt::AlignHCenter);
    ui->lblDataUploadInfo->setText("数据上传失败,请重新上传或检查网络状况!");

    ui->btnDataUploadComfirm->setVisible(true);
    ui->btnDataUploadComfirm->resize(150,45);
    ui->btnDataUploadComfirm->move(255,415);
    ui->btnDataUploadComfirm->setStyleSheet("background-color: rgb(88, 186, 236);color: rgb(255, 255, 255)");

    ui->btnDataUploadCancel->setVisible(false);
#endif

}

void mainDialog::dataUploadShow(bool show)
{
    if(show)
    {
        DeviceManager dm;
        QList<DevNode *> nodeList;

        dm.getServerInfo(nodeList);

        ui->leServerIP->setText(nodeList.at(0)->value);
        ui->leServerPort->setText(nodeList.at(1)->value);
    }
    ui->wgDataUpload->setVisible(show);
}



//有线网络
void mainDialog::initEthWidget()
{
    ui->wgEth->resize(820,625);
    ui->wgEth->move(460,40);
//    ui->wgEth->setStyleSheet("background-image:url(:/images/eth_bg.png);background-repeat: no-repeat");
    QPalette pal;
    pal.setBrush(ui->wgEth->backgroundRole(), QBrush(QPixmap(":/images/eth_bg.png")));
    ui->wgEth->setPalette(pal);
    ui->wgEth->setAutoFillBackground(true);

    ui->rbDhcp->resize(125,40);
    ui->rbDhcp->move(95,35);
    ui->rbDhcp->setStyleSheet("QRadioButton {color: rgb(255, 255, 255);font-size: 26px}"
                              "QRadioButton::indicator {width: 35px;height: 40px}"
                              "QRadioButton::indicator:checked {image: url(:/images/radio_checked.png)}"
                              "QRadioButton::indicator:unchecked {image: url(:/images/radio_unchecked.png)}"
                              );

    ui->rbStaticIp->resize(125,40);
    ui->rbStaticIp->move(295,35);
    ui->rbStaticIp->setStyleSheet("QRadioButton {color: rgb(255, 255, 255);font-size: 26px}"
                              "QRadioButton::indicator {width: 35px;height: 40px}"
                              "QRadioButton::indicator:checked {image: url(:/images/radio_checked.png)}"
                              "QRadioButton::indicator:unchecked {image: url(:/images/radio_unchecked.png)}"
                              );

    ui->rbDns->resize(300,40);
    ui->rbDns->move(495,35);
    ui->rbDns->setStyleSheet("QRadioButton {color: rgb(255, 255, 255);font-size: 26px}"
                              "QRadioButton::indicator {width: 35px;height: 40px}"
                              "QRadioButton::indicator:checked {image: url(:/images/radio_checked.png)}"
                              "QRadioButton::indicator:unchecked {image: url(:/images/radio_unchecked.png)}"
                              );

    ui->leIp1->resize(77,28);
    ui->leIp1->move(102,141);
//    ui->leIp1->setStyleSheet("QLineEdit {"
//                             "border: 1px solid rgb(41, 57, 85);"
//                             "border-radius: 3px;"
//                             "background: white;"
//                             "selection-background-color: green;"
//                             "font-size: 14px ;}");
    ui->leIp2->resize(77,28);
    ui->leIp2->move(206,141);
//    ui->leIp2->setStyleSheet("QLineEdit{background:rgba(255, 255, 255, 0)}");
    ui->leIp3->resize(77,28);
    ui->leIp3->move(310,141);
//    ui->leIp3->setStyleSheet("QLineEdit{background-color: rgba(255, 255, 255, 0)}");
    ui->leIp4->resize(77,28);
    ui->leIp4->move(414,141);
//    ui->leIp4->setStyleSheet("QLineEdit{background-color: rgba(255, 255, 255, 0)}");

    ui->leSubnet1->resize(77,28);
    ui->leSubnet1->move(102,211);
    ui->leSubnet2->resize(77,28);
    ui->leSubnet2->move(206,211);
    ui->leSubnet3->resize(77,28);
    ui->leSubnet3->move(310,211);
    ui->leSubnet4->resize(77,28);
    ui->leSubnet4->move(414,211);

    ui->leGateway1->resize(77,28);
    ui->leGateway1->move(102,281);
    ui->leGateway2->resize(77,28);
    ui->leGateway2->move(206,281);
    ui->leGateway3->resize(77,28);
    ui->leGateway3->move(310,281);
    ui->leGateway4->resize(77,28);
    ui->leGateway4->move(414,281);

    ui->lePredns1->resize(77,28);
    ui->lePredns1->move(102,371);
    ui->lePredns2->resize(77,28);
    ui->lePredns2->move(206,371);
    ui->lePredns3->resize(77,28);
    ui->lePredns3->move(310,371);
    ui->lePredns4->resize(77,28);
    ui->lePredns4->move(414,371);

    ui->leAltdns1->resize(77,28);
    ui->leAltdns1->move(102,441);
    ui->leAltdns2->resize(77,28);
    ui->leAltdns2->move(206,441);
    ui->leAltdns3->resize(77,28);
    ui->leAltdns3->move(310,441);
    ui->leAltdns4->resize(77,28);
    ui->leAltdns4->move(414,441);

    ui->btnEthComfirm->resize(150,45);
    ui->btnEthComfirm->move(255,518);
    ui->btnEthComfirm->setStyleSheet("background-color: rgb(88, 186, 236);color: rgb(255, 255, 255)");
}

void mainDialog::ethShow(bool show)
{
    ui->wgEth->setVisible(show);
}

void mainDialog::initWifiWidget()
{
    ui->wgWifi->resize(820,625);
    ui->wgWifi->move(460,40);

    ui->cbWifl->resize(385,28);
    ui->cbWifl->move(135,120);
    ui->cbWifl->setStyleSheet(
                "QCheckBox::indicator { width: 70px;height: 20px;}"
                "QCheckBox::indicator::unchecked {image: url(:/images/checkbox_unchecked.png);}"
                "QCheckBox::indicator::checked {image: url(:/images/checkbox_checked.png);}"
                "QCheckBox{font-size: 20px;color: rgb(255, 255, 255);}"
                );
    ui->cbWifl->setChecked(false);
    ui->cbWifl->setText("无线局域网已关闭                        ");

    ui->lblWifiCloseInfo->resize(380,28);
    ui->lblWifiCloseInfo->move(140,190);
    ui->lblWifiCloseInfo->setStyleSheet("font-size: 16px;color: gray;");
    ui->lblWifiCloseInfo->setVisible(true);

    ui->twWifiConnectInfo->setVisible(false);
    ui->twWifiConnectInfo->resize(380,28);
    ui->twWifiConnectInfo->move(140,190);
    ui->twWifiConnectInfo->setColumnCount(3);
    ui->twWifiConnectInfo->setFrameShape(QFrame::NoFrame); //设置无边框
    ui->twWifiConnectInfo->setShowGrid(false); //设置不显示格子线
    ui->twWifiConnectInfo->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->twWifiConnectInfo->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行为时每次选择一行
    ui->twWifiConnectInfo->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置不可编辑
    ui->twWifiConnectInfo->setStyleSheet("background-color:rgba(0,0,0,0);" //背景透明
                              "color:rgb(255,255,255);"
                              "font-size: 16px;"
                              );
    ui->twWifiConnectInfo->setRowCount(1);
    ui->twWifiConnectInfo->setRowHeight(0,28);
    //horizontal
    ui->twWifiConnectInfo->horizontalHeader()->setVisible(false);
    ui->twWifiConnectInfo->horizontalHeader()->resizeSection(0,320);
    ui->twWifiConnectInfo->horizontalHeader()->resizeSection(1,30);
    ui->twWifiConnectInfo->horizontalHeader()->resizeSection(2,30);
    //vertical
    ui->twWifiConnectInfo->verticalHeader()->setVisible(false);

    QFrame *line = new QFrame(ui->wgWifi);
    line->setGeometry(QRect(140, 225, 400, 3));
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setStyleSheet("border-top: 2px solid gray;");// background-color: green;");
    line->raise();

    ui->lblWifiSelect->resize(180,25);
    ui->lblWifiSelect->move(140,275);
    ui->lblWifiSelect->setStyleSheet("image: url(:/images/wifi_select.png);font-size: 24px;");
    ui->lblWifiSelect->setVisible(false);

    ui->twWifi->setVisible(false);
    ui->twWifi->resize(380,290);
    ui->twWifi->move(140,315);
    ui->twWifi->setColumnCount(3);
    ui->twWifi->setFrameShape(QFrame::NoFrame); //设置无边框
    ui->twWifi->setShowGrid(false); //设置不显示格子线
    ui->twWifi->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->twWifi->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选择行为时每次选择一行
    ui->twWifi->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置不可编辑
    ui->twWifi->setStyleSheet("background-color:rgba(0,0,0,0);" //背景透明
                              "color:rgb(255,255,255);"
                              "font-size: 16px;"
                              );
    //horizontal
    ui->twWifi->horizontalHeader()->setVisible(false);
    ui->twWifi->horizontalHeader()->resizeSection(0,320);
    ui->twWifi->horizontalHeader()->resizeSection(1,30);
    ui->twWifi->horizontalHeader()->resizeSection(2,30);
    //vertical
    ui->twWifi->verticalHeader()->setVisible(false);
//    ui->twWifi->verticalHeader()->setDefaultSectionSize(30);

    appendWifiAP("TP_link123",true,WSSMiddle);
    appendWifiAP("tp_link01",false,WSSStrong);
}

void mainDialog::insertWifiAP(QTableWidget *tableWidget, int itemIndex, QString wifiSSID, bool locked, WifiSignalStrength wss)
{
    tableWidget->setItem(itemIndex,0,new QTableWidgetItem(wifiSSID));
    QLabel *lblLocked = new QLabel("");

    if(locked)
        lblLocked->setPixmap(QPixmap(":/images/wifi_locked.png").scaled(30,28));
    else
        lblLocked->setPixmap(QPixmap("").scaled(30,28));

    tableWidget->setCellWidget(itemIndex,1,lblLocked);

    QLabel *lblWss = new QLabel("");
    switch(wss)
    {
    case WSSStrong:
        lblWss->setPixmap(QPixmap(":/images/wifi_strong.png").scaled(30,28));
        break;
    case WSSMiddle:
        lblWss->setPixmap(QPixmap(":/images/wifi_middle.png").scaled(30,28));
        break;
    case WSSWeak:
        lblWss->setPixmap(QPixmap(":/images/wifi_weak.png").scaled(30,28));
        break;
    default:
        lblWss->setPixmap(QPixmap("").scaled(30,28));
        break;
    }
    tableWidget->setCellWidget(itemIndex,2,lblWss);
}

void mainDialog::appendWifiAP(QString wifiSSID, bool locked, WifiSignalStrength wss)
{
    int rowIndex = ui->twWifi->rowCount();
    ui->twWifi->setRowCount(rowIndex + 1);
    ui->twWifi->setRowHeight(rowIndex,28);
    insertWifiAP(ui->twWifi,rowIndex,wifiSSID,locked,wss);
}

void mainDialog::updateWifiConnectInfo(QString wifiSSID, bool locked, WifiSignalStrength wss)
{
    insertWifiAP(ui->twWifiConnectInfo,0,"您当前链接的网络: " +  wifiSSID,locked,wss);
}

void mainDialog::wifiShow(bool show)
{
    ui->wgWifi->setVisible(show);
}

void mainDialog::resetWifi()
{
    ui->twWifi->clear();
    ui->twWifi->setRowCount(0);
    updateWifiConnectInfo("",false,WSSNone);
}

void mainDialog::on_cbWifl_clicked()
{
    if(ui->cbWifl->isChecked())
    {
        ui->lblWifiCloseInfo->setVisible(false);
        ui->cbWifl->setText("无线局域网已开启                        ");
        ui->lblWifiSelect->setVisible(true);
        ui->twWifi->setVisible(true);
        ui->twWifiConnectInfo->setVisible(true);
        updateWifiConnectInfo("",false,WSSNone);
        //test
//        updateWifiConnectInfo("TP_link123",true,WSSMiddle);
//        resetWifi();
    }
    else
    {
        ui->lblWifiCloseInfo->setVisible(true);
        ui->cbWifl->setText("无线局域网已关闭                        ");
        ui->lblWifiSelect->setVisible(false);
        ui->twWifi->setVisible(false);
        ui->twWifiConnectInfo->setVisible(false);
    }
}

void mainDialog::on_twWifi_cellClicked(int row, int column)
{
    Q_UNUSED(column);

    inputWifiKeyDlg = new InputKeyDialog();
//    inputWifiKeyDlg->initView(QString("cell clicked: %1, %2").arg(row).arg(column));
    QString ssid = ui->twWifi->item(row,0)->data(Qt::DisplayRole).toString();
    inputWifiKeyDlg->initView(ssid);
    inputWifiKeyDlg->exec();
    QString key = inputWifiKeyDlg->getKey();
    delete inputWifiKeyDlg;

    //test
    qDebug()<<QString("WiFi key = %1").arg(key);
    updateWifiConnectInfo(ssid,true,WSSStrong);
}

void mainDialog::initVolume()
{
    ui->wgVolume->resize(820,625);
    ui->wgVolume->move(460,40);
    ui->wgVolume->setAttribute(Qt::WA_TranslucentBackground, true);

    ui->lcdVolume->resize(120,75);
    ui->lcdVolume->move(270,215);
    ui->lcdVolume->setStyleSheet("color: rgb(88, 186, 236);border-style:none;");
    ui->lcdVolume->setSegmentStyle(QLCDNumber::Flat);
    ui->lcdVolume->setDigitCount(3);

    ui->sliderVolume->resize(360,20);
    ui->sliderVolume->move(150,372);
    ui->sliderVolume->setMinimum(0);
    ui->sliderVolume->setMaximum(100);
    ui->sliderVolume->setSingleStep(1);
    ui->sliderVolume->setStyleSheet(
                "QSlider::groove:horizontal {                                "
                "     border: 1px solid #262A2C;                             "
                "     height: 12px;                                           "
                "     margin: 0px 0;                                         "
                "     left: 12px; right: 12px;                               "
                " }                                                          "
                "QSlider::handle:horizontal {                                "
                "     border: 1px solid #5c5c5c;                             "
                " border-image:url(:/images/volume_slider.png);              "
                "     width: 18px;                                           "
                "     margin: -1px -1px -1px -1px;                           "
                " }                                                          "
                "                                                            "
                "QSlider::sub-page:horizontal{                               "
                " background: qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0, stop:0 rgba(120, 189, 219, 255), stop:0.25 rgba(133, 192, 214, 255), stop:0.5 rgba(180, 202, 205  , 255), stop:1 rgba(207, 219, 214, 255));                      "
                "}                                                           "
                "QSlider::add-page:horizontal{                               "
                " background-color: rgba(44,42,38,255)  "
                "}"
                );

    ui->lblSub->resize(25,20);
    ui->lblSub->move(113,370);
    ui->lblSub->setStyleSheet("border-image:url(:/images/volume_sub.png)");

    ui->lblAdd->resize(25,20);
    ui->lblAdd->move(525,370);
    ui->lblAdd->setStyleSheet("border-image:url(:/images/volume_add.png)");
}

void mainDialog::volumeShow(bool show)
{
    if(show)
    {
        // get current volume
    }
    ui->wgVolume->setVisible(show);
}

void mainDialog::displayVolume(int volume)
{
    ui->lcdVolume->display(volume);
}

void mainDialog::initDevTest()
{
    ui->wgTestDevice->resize(820,625);
    ui->wgTestDevice->move(460,40);
    ui->wgTestDevice->setAttribute(Qt::WA_TranslucentBackground, true);


    ui->lbTempHumidity->resize(180,50);
    ui->lbTempHumidity->move(140,50);
    ui->lbTempHumidity->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->lbTHResult->resize(40,50);
    ui->lbTHResult->move(350,50);
//    ui->lbTHResult->setAttribute(Qt::WA_TranslucentBackground, true);

    ui->lbTHResult->setStyleSheet("border-image:url(:/images/input_ok.png)");


    ui->lbAlcohol->resize(180,50);
    ui->lbAlcohol->move(140,110);
    ui->lbAlcohol->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->lbAlcoholResult->resize(40,50);
    ui->lbAlcoholResult->move(350,110);
//    ui->lbAlcoholResult->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->lbAlcoholResult->setStyleSheet("border-image:url(:/images/input_ok.png)");



    ui->lbBP->resize(180,50);
    ui->lbBP->move(140,170);
    ui->lbBP->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->lbBPResult->resize(40,50);
    ui->lbBPResult->move(350,170);
//    ui->lbBPResult->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->lbBPResult->setStyleSheet("border-image:url(:/images/input_ok.png)");

    ui->lbInfra->resize(180,50);
    ui->lbInfra->move(140,230);
    ui->lbInfra->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->lbInfraResult->resize(40,50);
    ui->lbInfraResult->move(350,230);
//    ui->lbInfraResult->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->lbInfraResult->setStyleSheet("border-image:url(:/images/input_ok.png)");

    ui->lbBattery->resize(180,50);
    ui->lbBattery->move(140,290);
    ui->lbBattery->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->lbBatteryResult->resize(40,50);
    ui->lbBatteryResult->move(350,290);
//    ui->lbBatteryResult->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->lbBatteryResult->setStyleSheet("border-image:url(:/images/input_ok.png)");


    ui->btnStartDevTest->resize(180,50);
    ui->btnStartDevTest->move(250,500);
    ui->btnStartDevTest->setStyleSheet("background-color: rgb(88, 186, 236);color: rgb(255, 255, 255)");

}

void mainDialog::devTestShow(bool show)
{
    ui->wgTestDevice->setVisible(show);
}

void mainDialog::hideAll()
{
    faceRecogShow(false);
    fpRecogShow(false);
    alcoholTestShow(false);
    healthTestShow(false, false);
    userReg1Show(false);
    userReg2Show(false);
    userReg3Show(false);
    settingButtonsShow(false);
    loginWayShow(false);
    dataUploadShow(false);
    ethShow(false);
    wifiShow(false);
    volumeShow(false);
    devTestShow(false);
    showMessage("");
    showAlcoholTestInfo("");
}


void mainDialog::on_sliderVolume_valueChanged(int value)
{
    displayVolume(value);
}


void mainDialog::updateUserInfo(const QString &name, const QString &no, const QString &dpt, const QString &org, const QString &id)
{
    ui->lblName->setText(name);
    ui->lblNo->setText(no);
    ui->lblDpt->setText(dpt);
    ui->lblOrg->setText(org);
    ui->lblId->setText(id);
}

void mainDialog::updateActiveButton(ActiveButton activeBtn)
{
    if(mainActiveBtn == activeBtn)
        return;
    hideAll();
    emit sigChangeFlag(true);
    showMessage("");
    switch(mainActiveBtn)
    {
    case AlcoholTest:
        ui->btnAlcoholTest->setStyleSheet("border-image:url(:/images/alcohol_test2.png)");

        break;
    case HealthTest:
        ui->btnHealthTest->setStyleSheet("border-image:url(:/images/health_test2.png)");

        break;
    case UserReg:
        ui->btnUserReg->setStyleSheet("border-image:url(:/images/user_reg2.png)");

        break;
    case UserSetting:
        ui->btnUserSetting->setStyleSheet("border-image:url(:/images/user_setting2.png)");

        break;
    default:
        break;
    }

    switch(activeBtn)
    {
    case AlcoholTest:
        ui->btnAlcoholTest->setStyleSheet("border-image:url(:/images/alcohol_test1.png)");
        initAlcoholTestWidget1();
        alcoholTestShow(true);
//        ui->lcdAlcoholTestNum->setVisible(true);
//        ui->lblAlcoholTestUnit->setVisible(true);

//        //step2
//        ui->lblAlcoholTestInfo->setVisible(true);
//        ui->btnAlcoholTestRetest->setVisible(true);
//        ui->btnAlcoholTestHear->setVisible(true);
//        ui->btnAlcoholTestCancel->setVisible(true);

        break;
    case HealthTest:
        ui->btnHealthTest->setStyleSheet("border-image:url(:/images/health_test1.png)");
        healthTestShow(true, true);
        break;
    case UserReg:
        ui->btnUserReg->setStyleSheet("border-image:url(:/images/user_reg1.png)");
        initUserReg1Widget();
        userReg1Show(true);
//        userReg2Show(true);
//        userReg3Show(true);

        break;
    case UserSetting:
        ui->btnUserSetting->setStyleSheet("border-image:url(:/images/user_setting1.png)");
        settingButtonsShow(true);
        break;
    default:
        break;
    }

    mainActiveBtn = activeBtn;
}

void mainDialog::on_btnAlcoholTest_clicked()
{
    if(mainActiveBtn != AlcoholTest)
    {
        updateActiveButton(AlcoholTest);
    }
}

void mainDialog::on_btnHealthTest_clicked()
{
    if(mainActiveBtn != HealthTest)
    {
        updateActiveButton(HealthTest);
    }
}

void mainDialog::on_btnUserReg_clicked()
{
    if(mainActiveBtn != UserReg)
    {
        updateActiveButton(UserReg);
    }
}

void mainDialog::on_btnUserSetting_clicked()
{
    if(mainActiveBtn != UserSetting)
    {
        updateActiveButton(UserSetting);
    }
}

void mainDialog::on_userInfoChanged(const UserInfo user)
{
    qDebug() << user.userID << user.userName;
    ui->lblName->setText(user.userName);
    ui->lblNo->setText(user.userNumber);
    ui->lblDpt->setText(user.department);
    ui->lblOrg->setText(user.org);
    ui->lblId->setText(user.userID);

    QPixmap pixmap = QPixmap::fromImage(user.userPic);

    // resize pixmap
    if(!pixmap.isNull())
    {
        QSize picSize = ui->lblPic->size();
        QPixmap scaledPixmap = pixmap.scaled(picSize);
        ui->lblPic->setPixmap(scaledPixmap);
    }

    ui->lblFaceRecogInfo->show();
}


void mainDialog::loginIn()
{
    ui->lblAlcoholTestInfo->setVisible(true);
    ui->lblAlcoholTestInfo->resize(400, 50);
    ui->lblAlcoholTestInfo->move(280,500);
    ui->lblAlcoholTestInfo->setAlignment(Qt::AlignHCenter);
    ui->lblAlcoholTestInfo->setText("请按指纹登录");
//    ui->lblAlcoholTestBg->resize(200,320);
//    ui->lblAlcoholTestBg->move(390,100);
//    ui->lblAlcoholTestBg->setStyleSheet("border-image:url(:/images/finger_print.png)");
//    ui->lblAlcoholTestBg->setVisible(true);

    fpVerify();

}

// verify finger print
void mainDialog::fpVerify()
{
    QtAndroid::androidActivity().callMethod<void>("OnVerify", "()V");

//    qRegisterMetaType<UserInfo>("UserInfo");
//    FacialThread *ft = new FacialThread(1);

//    connect(ft,SIGNAL(sigShowMsg(const QString)),this,SLOT(showAlcoholTestInfo(QString)),Qt::AutoConnection);
//    connect(ft,SIGNAL(sigUpdateUserInfo(const UserInfo)),this,SLOT(on_userInfoChanged(const UserInfo)),Qt::AutoConnection);
//    ft->start();
}


char facePicBuf[20 * 1024] = {0};

void mainDialog::facialVerify(int userNumber)
{
//    ui->lblAlcoholTestInfo->setVisible(true);
//    ui->lblAlcoholTestInfo->move(0,390);
//    ui->lblAlcoholTestInfo->setText("");
//    ui->lblAlcoholTestBg->resize(200,320);
//    ui->lblAlcoholTestBg->move(390,100);
//    ui->lblAlcoholTestBg->setStyleSheet("border-image:url(:/images/finger_print.png)");
//    ui->lblAlcoholTestBg->setVisible(true);
//    hideAll();
//    faceRecogShow(true);

    ui->lblAlcoholTestInfo->setText("请进行人脸识别");

    QString str = QString::number(userNumber);
    QByteArray baNum = str.toLatin1();

    char *pUserNumber = baNum.data();

    char nameBuff[128] = {0};
//    char idBuff[9] = {0};
    char cScore = 0;

    qDebug("userNum :%s", pUserNumber);
    int ret = dealFaceVerifyCmd(pUserNumber, nameBuff, &cScore);
    if(1 == ret)
    {
        showAlcoholTestInfo("识别成功");

        QByteArray ba(nameBuff);
         QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
        QTextCodec *codec = QTextCodec::codecForName("GBK");
        QString nameStr = codec->toUnicode(ba);

        QFile file(FILE_USER_INFO);
        file.open(QIODevice::ReadOnly);
        QString userInfo;

        QTextStream inputStream(&file);

        while(1)
        {
            inputStream >> userInfo;
            qDebug() << userInfo;
            if(userInfo.isEmpty())
                break;

            ba = userInfo.toLocal8Bit();
            nameStr = codec->toUnicode(ba);
            QStringList strList = nameStr.split(":::");



            qDebug() << strList.value(1) << ":" << str;

            if(strList.value(1) == str)
            {
                currentUser.userName = strList.value(0);
                currentUser.userNumber = strList.value(1);
                currentUser.department = strList.value(2);
                currentUser.org = strList.value(3);
                currentUser.userID = strList.value(4);

                break;
            }

        }
        ui->lblName->setText(currentUser.userName);
        ui->lblNo->setText(currentUser.userNumber);
        ui->lblDpt->setText(currentUser.department);
        ui->lblOrg->setText(currentUser.org);
        ui->lblId->setText(currentUser.userID);

        ba = currentUser.userName.toLatin1();
        char *p = ba.data();

        for(int j = 0; j < ba.length(); ++j)
            qDebug("%02x", p[j]);

        qDebug() << "name:" << nameStr;
        qDebug() << "id:" << str;
        qDebug() << "score:" << QString::number(cScore);

//        int iPicSize = 0;
//        char recvBuf[1024] = {0};
        qDebug("userNum :%s", pUserNumber);
#if 0
        bzero(facePicBuf, sizeof(facePicBuf));
        ret =  dealGetUserPicCmd(pUserNumber, facePicBuf, &iPicSize);
        if(0 == ret)
        {
            qDebug() << "pic size:" << iPicSize;
            for(int i =0; i < iPicSize;i++)
            {
                qDebug("%02x ", facePicBuf[i]);
            }
            QByteArray base64Data(facePicBuf, iPicSize);
            qDebug("size of ba:%d",base64Data.length());
            QImage image;

            if(!image.loadFromData(QByteArray::fromBase64(base64Data), "PNG"))
            {
                qDebug() << "failed to load image";
            }

            QPixmap p;
            // fill array with image
            if(!p.loadFromData(QByteArray::fromBase64(base64Data),"PNG"))
            {
                qDebug() << "failed to load pixmap";
            }
//            QPixmap image("./images/camera.png");


//            QImage* img=new QImage;

//            if(! ( img->load("/images/camera.png") ) ) //加载图像
//            {
//                QMessageBox::information(this,
//                                         "打开图像失败",
//                                         "打开图像失败!");
//                delete img;
//           }
//            ui->lblFaceRecogInfo->setPixmap(QPixmap::fromImage(*img));
//            ui->lblFaceRecogInfo->setStyleSheet("border-image:url(:/images/camera.png)");
            QFile file("111.png");
            file.open(QIODevice::WriteOnly);
            p.save(&file, "PNG");
            file.close();
            ui->lblFaceRecogInfo->setPixmap(p);
            ui->lblFaceRecogInfo->setText("");
            ui->lblFaceRecogInfo->show();

//            ui->lblFaceRecogInfo->setVisible(true);
            faceRecogShow(true);
        }
        else if(1 == ret)
        {
            qDebug() << "user not found " << str;

            for (int i = 0; i < iPicSize; i++)
            {
                qDebug("%02x ", recvBuf[i]);
            }
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
    }
    else if(2 == ret)
    {
        ui->lblAlcoholTestInfo->setText("未找到用户");
    }
    else
    {
        QString str = QString("人脸识别失败") + "["+ QString::number(ret) + "]";
        ui->lblAlcoholTestInfo->setText(str);

        if(-2 == ret)
        {
            for(int j = 0; j < 128; ++j)
                qDebug("%02x", nameBuff[j]);
        }
    }


//    recvReportVerifyResult(nameBuff , idBuff, &cScore);

//    qDebug() << "name:" << QString(QLatin1String(nameBuff));
//    qDebug() << "id:" << QString(QLatin1String(idBuff));
//    qDebug() << "score:" << QString::number(cScore);

}


void mainDialog :: updateMessage(const QString &info)
{
    ui->lbMessage->setText(info);
}


void mainDialog::onHealthTestSYSChanged(int sys)
{
    ui->lcdSys->display(sys);
}

void mainDialog::onHealthTestDIAChanged(int dia)
{
    ui->lcdDia->display(dia);
}

void mainDialog::onHealthTestPUISEChanged(int puise)
{
    ui->lcdPuise->display(puise);
}

void mainDialog::onHealthTestBBTChanged(int bbt)
{
    bbt /= 10;
    double bbtDb = (double)bbt / 10.0;
    qDebug() << bbtDb;
    currentUser.health.bbt = QString::number(bbtDb);
    ui->lcdBbt->display(bbtDb);
}



void mainDialog::on_pbStartBP_released()
{
    ui->pbStartBP->setDisabled(true);

    displayHealthTestSYS(0);
    displayHealthTestDIA(0);
    displayHealthTestPUISE(0);

    QThread::yieldCurrentThread();

    showMessage("请把手指轻按到血压仪上");
    HealthTestThread *htt = new HealthTestThread(0);
    connect(htt,SIGNAL(sigShowMsg(QString)),this,SLOT(updateMessage(QString)),Qt::AutoConnection);
    connect(htt,SIGNAL(sigChangeHealthTestSYS(int)),this,SLOT(onHealthTestSYSChanged(int)),Qt::AutoConnection);
    connect(htt,SIGNAL(sigChangeHealthTestDIA(int)),this,SLOT(onHealthTestDIAChanged(int)),Qt::AutoConnection);
    connect(htt,SIGNAL(sigChangeHealthTestPUISE(int)),this,SLOT(onHealthTestPUISEChanged(int)),Qt::AutoConnection);
    connect(htt,SIGNAL(sigChangeHealthTestBBT(int)),this,SLOT(onHealthTestBBTChanged(int)),Qt::AutoConnection);

    connect(htt, SIGNAL(finished()), htt, SLOT(deleteLater()));
    connect(htt,SIGNAL(finished()),this,SLOT(setBtnBpEnable()),Qt::AutoConnection);

    htt->start();

//    QTimer *timer = new QTimer(this);
//    timer->setSingleShot(true);
//    connect(timer, SIGNAL(timeout()), this, SLOT(killHTTThread(htt)));
//    timer->start(30000);
}

void mainDialog:: killHTTThread(HealthTestThread *htt)
{
    if(htt->isRunning())
    {
        htt->exit();
        showMessage("");
    }
}

void mainDialog::setBtnBpEnable()
{
    ui->pbStartBP->setDisabled(false);
}

void mainDialog::on_pbStartTemp_released()
{
    displayHealthTestBBT(0);

    ui->pbStartTemp->setDisabled(true);

    QThread::yieldCurrentThread();

    HealthTestThread *htt = new HealthTestThread(1);
    connect(htt,SIGNAL(sigShowMsg(QString)),this,SLOT(updateMessage(QString)),Qt::AutoConnection);
    connect(htt,SIGNAL(sigChangeHealthTestSYS(int)),this,SLOT(onHealthTestSYSChanged(int)),Qt::AutoConnection);
    connect(htt,SIGNAL(sigChangeHealthTestDIA(int)),this,SLOT(onHealthTestDIAChanged(int)),Qt::AutoConnection);
    connect(htt,SIGNAL(sigChangeHealthTestPUISE(int)),this,SLOT(onHealthTestPUISEChanged(int)),Qt::AutoConnection);
    connect(htt,SIGNAL(sigChangeHealthTestBBT(int)),this,SLOT(onHealthTestBBTChanged(int)),Qt::AutoConnection);

    connect(htt, SIGNAL(finished()), htt, SLOT(deleteLater()));
    connect(htt,SIGNAL(finished()),this,SLOT(setBtnTempEnable()),Qt::AutoConnection);
    connect(this, SIGNAL(sigChangeFlag(bool)),htt, SLOT(on_flagChanged(bool)), Qt::AutoConnection);


    htt->start();
}

void mainDialog::setBtnTempEnable()
{
    ui->pbStartTemp->setDisabled(false);
}

void mainDialog::on_btnStartAlcoholTest_released()
{
    ui->btnStartAlcoholTest->setDisabled(true);
    alcoholTestStart();
}


/*
 * 双击界面左上方（300，50）（600，250）区域内以激活酒精模块常量校准对话窗。
 */
void mainDialog::mouseDoubleClickEvent( QMouseEvent * e )
{
    if ( e->button() == Qt::LeftButton )
    {
       QPoint p = e->pos();
       if(p.x() > 300 && p.x() < 600  && p.y() > 50 && p.y() < 250)
       {
            AlcoholAdjustDialog *aad = new AlcoholAdjustDialog();

            aad->resize(980,625);
            aad->move(300,40);
            aad->show();
       }

       if(p.x() > 900 && p.x() < 1200  && p.y() > 0 && p.y() < 250)
       {
           QFile::remove(FILE_USER_INFO);
           QFile file(FILE_USER_INFO);

           file.open(QIODevice::WriteOnly);
           QTextStream txtOutput(&file);
           txtOutput << "";
           file.close();


           QFile::remove("fpData.txt");
           QFile fpfile("fpData.txt");

           fpfile.open(QIODevice::WriteOnly);
           QTextStream tOutput(&fpfile);
           tOutput << "11:::999999;;;";
           fpfile.close();

           showMessage("用户信息已删除");
       }

    }
}



// 在keyPressEvent中去掉对return的处理，来修正输入法键盘中按“完成”时，界面跳到其他界面的问题
void mainDialog::keyPressEvent(QKeyEvent *e)
{
    if ( e->key() == Qt::Key_Return)
    {
        return ;
    }
    else
    {
        QDialog::keyPressEvent(e);
    }
}



void mainDialog::on_btnStartDevTest_released()
{
    int ret = -1;
    char status;

    ui->btnStartDevTest->setDisabled(true);
    QThread::yieldCurrentThread();
    DeviceManager *dManager = new DeviceManager(this);


    dManager->checkDevice(ret, status);

    if(-1 == ret || !(status & STM_BIT_STATUS_TH) || !(status & STM_BIT_STATUS_ALCO) ||
            !(status & STM_BIT_STATUS_BAT) || !(status & STM_BIT_STATUS_BP) ||
            !(status & STM_BIT_STATUS_INFRA))
    {
        dManager->checkDevice(ret, status);
    }

    if(-1 == ret || !(status & STM_BIT_STATUS_TH) || !(status & STM_BIT_STATUS_ALCO) ||
            !(status & STM_BIT_STATUS_BAT) || !(status & STM_BIT_STATUS_BP) ||
            !(status & STM_BIT_STATUS_INFRA))
    {
        dManager->checkDevice(ret, status);
    }
//    QThread *th = new QThread();

//    dManager->moveToThread(th);
//    connect(th, SIGNAL(started()), dManager, SLOT(checkDevice(ret, msg)));
//    connect(th, SIGNAL(finished()), th, SLOT(deleteLater()));

//    th->start();


    QThread::sleep(1);

    ui->lbTHResult->setStyleSheet("border-image:url(:/images/input_ok.png)");
    ui->lbAlcoholResult->setStyleSheet("border-image:url(:/images/input_ok.png)");
    ui->lbBPResult->setStyleSheet("border-image:url(:/images/input_ok.png)");
    ui->lbInfraResult->setStyleSheet("border-image:url(:/images/input_ok.png)");
    ui->lbBatteryResult->setStyleSheet("border-image:url(:/images/input_ok.png)");
    if(-1 == ret)
    {
        ui->lbTHResult->setStyleSheet("border-image:url(:/images/input_err.png)");
        ui->lbAlcoholResult->setStyleSheet("border-image:url(:/images/input_err.png)");
        ui->lbBPResult->setStyleSheet("border-image:url(:/images/input_err.png)");
        ui->lbInfraResult->setStyleSheet("border-image:url(:/images/input_err.png)");
        ui->lbBatteryResult->setStyleSheet("border-image:url(:/images/input_err.png)");
    }
    else
    {
        if(!(status & STM_BIT_STATUS_TH))
        {
            ui->lbTHResult->setStyleSheet("border-image:url(:/images/input_err.png)");
        }
        if(!(status & STM_BIT_STATUS_ALCO))
        {
            ui->lbAlcoholResult->setStyleSheet("border-image:url(:/images/input_err.png)");
        }
        if(!(status & STM_BIT_STATUS_BAT))
        {
            ui->lbBatteryResult->setStyleSheet("border-image:url(:/images/input_err.png)");
        }
        if(!(status & STM_BIT_STATUS_BP))
        {
            ui->lbBPResult->setStyleSheet("border-image:url(:/images/input_err.png)");
        }
        if(!(status & STM_BIT_STATUS_INFRA))
        {
            ui->lbInfraResult->setStyleSheet("border-image:url(:/images/input_err.png)");
        }
    }

    ui->btnStartDevTest->clearFocus();
    qDebug("ret:%d, status:%x", ret, status);

    ui->btnStartDevTest->setDisabled(false);
}

/*
 * 上传数据 按钮释放时处理
 */
void mainDialog::on_btnDataUploadData_released()
{
    // save ip and port if they are modified
    DeviceManager dm;
    QList<DevNode *>nodeList;
    dm.getServerInfo(nodeList);

    QString leIP = ui->leServerIP->text();
    QString lePort = ui->leServerPort->text();

    if(nodeList.at(0)->value != leIP || nodeList.at(1)->value != lePort)
    {
        nodeList.at(0)->value = leIP;
        nodeList.at(1)->value = lePort;
        // update config file devinfo.xml
        dm.updateDevInfo(nodeList);
    }

    // restart updating thread
    if(uploadThread->isRunning())
    {
        uploadThread->exit();
    }

    uploadThread->start();
}


/*
 * 释放音量调节滑块时设置新音量
 */
void mainDialog::on_sliderVolume_sliderReleased()
{
    QAndroidJniObject::callStaticMethod<void>("o2oAttendance/QtFullscreenActivity","setVolume","(I)V",ui->sliderVolume->value());

}
