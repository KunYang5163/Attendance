#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>
#include <QString>
#include <QSignalMapper>
#include <QTableWidget>

#include "inputkeydialog.h"
#include "tcpforclient.h"
#include "common.h"
#include "healthtestthread.h"
#include "uploadthread.h"
#include "dbmanager.h"

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

namespace Ui {
class mainDialog;
}

const QString nameDefaultString     = "请输入姓名/用户名, 1 - 8字符(必填)";
const QString numberDefaultString   = "请输入工号, 1 - 8字符(必填)";
const QString dptDefaultString      = "请输入部门, 1 - 8字符(必填)";
const QString orgDefaultString      = "请输入单位, 1 - 8字符(必填)";
const QString idDefaultString       = "请输入证件号, 1 - 8字符(必填)";

typedef enum __UserRegInput
{
    InputName,
    InputNumber,
    InputDpt,
    InputOrg,
    InputId
}UserRegInput;

typedef enum __UserRegInputSign
{
    InputSignOk,
    InputSignErr,
    InputSignNone
}UserRegInputSign;

typedef enum __ActiveButton
{
    AlcoholTest,
    HealthTest,
    UserReg,
    UserSetting,
    ActiveButtonNone
}ActiveButton;

typedef enum __SettingType
{
    SettingLoginWay,
    SettingDataUpload,
    SettingEth,
    SettingWifi,
    SettingVolume,
    SettingSelfTest
}SettingType;

typedef enum __WiFiSignalStrength
{
    WSSStrong,
    WSSMiddle,
    WSSWeak,
    WSSNone,
}WifiSignalStrength;



class mainDialog : public QDialog
{
    Q_OBJECT

public:
    explicit mainDialog(QWidget *parent = 0);
    ~mainDialog();
    static mainDialog &instance(QWidget *parent = 0);

private:
    void takeImage();
    void initView();
    void initNaviButtons();
    void initUserInfoLabels();

    void showMessage(const QString &info);

//    void updateUserInfo(const char *name, const char *no, const char *dpt, const char *org, const char *id);
    void updateUserInfo(const QString &name, const QString &no, const QString &dpt, const QString &org, const QString &id);
    void updateActiveButton(ActiveButton activeBtn);
    //人脸识别
    void initFaceRecogWidget();
    void faceRecogShow(bool show);
    void showFaceRecogInfo(const QString &info);
    //指纹figer print识别
    void initFPRecogWidget();
    void fpRecogShow(bool show);
    void showFPRecogInfo(const QString &info);
    //酒精测试
    void initAlcoholTestWidget1();
    void initAlcoholTestWidget2();
    void initAlcoholTestWidget3();
    void alcoholTestStart();
    void alcoholTestShow(bool show);
    void displayAlcoholTestresult(int testResult);

    //健康测试
    void initHealthTestWidget();
    void healthTestShow(bool show, bool btnShow);
    void displayHealthTestSYS(int sys);
    void displayHealthTestDIA(int dia);
    void displayHealthTestPUISE(int puise);
    void displayHealthTestBBT(int bbt);
    //注册1
    void initUserReg1Widget();
    void userReg1Show(bool show);
    bool userReg1CheckInput();
    void userReg1ShowInputSign(UserRegInput input,UserRegInputSign sign);

    //注册2
    void initUserReg2Widget();
    void userReg2Show(bool show);

    //注册3
    void initUserReg3Widget();
    void userReg3Show(bool show);
    //设置按钮
    void initSettingButtonsWidget();
    void settingButtonsShow(bool show);
    void clearPreActiveButtonColor();
    void showSettingWidget();
    //登录方式设置
    void initLoginWayWidget();
    void loginWayShow(bool show);
    //数据上传设置
    void initDataUploadWidget1();
    void initDataUploadWidget2();
    void initDataUploadWidget3();
    void dataUploadShow(bool show);
    //有线网络
    void initEthWidget();
    void ethShow(bool show);
    //wifi
    void initWifiWidget();
    void insertWifiAP(QTableWidget *tableWidget, int itemIndex,QString wifiSSID,bool locked,WifiSignalStrength wss);
    void appendWifiAP(QString wifiSSID,bool locked,WifiSignalStrength wss);
    void updateWifiConnectInfo(QString wifiSSID,bool locked,WifiSignalStrength wss);
    void wifiShow(bool show);
    void resetWifi();
    //volume
    void initVolume();
    void volumeShow(bool show);
    void displayVolume(int volume);
    // device test
    void initDevTest();
    void devTestShow(bool show);


    void hideAll();
    void mouseDoubleClickEvent( QMouseEvent * e );
    void keyPressEvent(QKeyEvent *e);

public slots:
    void onNotifyEnroll(const int count, const QString temp);
    void onNotifyVerify(const int userNumber);




private:
    Ui::mainDialog *ui;

    ActiveButton mainActiveBtn;
//    QSignalMapper *signalMapper;
    SettingType currentSettingType;
    InputKeyDialog * inputWifiKeyDlg;
    TCPForClient tcpForClient;
    UserInfo currentUser;
    VideoCapture V;
    UploadThread *uploadThread;
    DBManager dbManager;


private slots:
    void on_btnAlcoholTest_clicked();//酒精测试
    void on_btnHealthTest_clicked();//健康测试
    void on_btnUserReg_clicked();//用户注册
    void on_btnUserSetting_clicked();//个人设置
    void on_userInfoChanged(const UserInfo user);
    //人脸识别
    void on_btnFaceRecogComfirm_clicked();//确定
    //指纹识别
    void on_btnFPRecogComfirm_clicked();//确定
    //酒精测试
    void on_btnAlcoholTestRetest_clicked();//重测
    void on_btnAlcoholTestHear_clicked();//审理
    void on_btnAlcoholTestCancel_clicked();//取消
    void on_btnAlcoholTestComfirm_clicked();//确定
    void onAlcoholResultChanged(int testResult);
    void switchToHealth();
    void showAlcoholTestInfo(const QString &info);

    //注册1
    void on_btnUserReg1Next_clicked();//下一步
    void on_leNameTextChanged(const QString &text);
    void on_leIdTextChanged(const QString &text);
    void on_leNumberTextChanged(const QString &text);
    void on_leDptTextChanged(const QString &text);
    void on_leOrgTextChanged(const QString &text);
    //注册2
    void on_btnUserReg2Next_clicked();//下一步
    //注册3
    void on_btnUserReg3Next_clicked();//下一步
    //设置按钮
//    void on_settingButtons_clicked(const SettingType type);
    void on_btnLoginWay_clicked();
    void on_btnDataUpload_clicked();
    void on_btnEth_clicked();
    void on_btnWifi_clicked();
    void on_btnVolume_clicked();
    void on_btnSelfTest_clicked();
    //登录方式


    //wifi
    void on_cbWifl_clicked();
    void on_twWifi_cellClicked(int row, int column);

    //volume
    void on_sliderVolume_valueChanged(int value);


    void startFingerPrintEnroll(int userNumber);
    void on_facialReg();

    void loginIn();
    void fpVerify();
    void facialVerify(int userNumber);

    void updateMessage(const QString &info);

    // health test slots for thread
    void onHealthTestSYSChanged(int sys);
    void onHealthTestDIAChanged(int dia);
    void onHealthTestPUISEChanged(int puise);
    void onHealthTestBBTChanged(int bbt);

    void on_pbStartBP_released();
    void killHTTThread(HealthTestThread *htt);
    void setBtnBpEnable();
    void on_pbStartTemp_released();
    void setBtnTempEnable();
    void alcolAndLogin();

    void on_btnStartAlcoholTest_released();
//    bool copyFileToPath(QString sourceDir ,QString toDir, bool coverFileIfExist);

    void on_btnStartDevTest_released();
    void on_btnDataUploadData_released();

    void on_sliderVolume_sliderReleased();

signals:
    void signalAlcoholTestStarted();
    void sigChangeFlag(bool);


};

#endif // MAINDIALOG_H
