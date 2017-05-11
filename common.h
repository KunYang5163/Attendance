#ifndef COMMON_H
#define COMMON_H

#include <qglobal.h>
#include <QImage>


#define PACKAGE_HEADER_FLAG      0XFF    // 报头标识
#define PACKAGE_VERSION          0X01    // 版本号



// 上位机命令
#define COMMAND_GET_DEVICE_INFO  0X01  // 获取设备信息
#define COMMAND_ALCOHOL_DETECTION 0X02   // 开始测酒
#define COMMAND_DISPLAY_MESSAGE  0X03  // 屏幕显示文字信息
#define COMMAND_PLAY_SOUND       0X04  // 播放语音
#define COMMAND_CHECK_DEVICE        0X05  // 下位机自检
#define COMMAND_DEL_FINGER_PRINT    0X06  // 删除指纹特征
#define COMMAND_DEL_FACIAL   0X07  // 删除人脸特征
#define COMMAND_DEL_FINGER_PRINT    0X06  // 删除指纹特征
#define COMMAND_DEL_FACIAL   0X07  // 删除人脸特征
#define COMMAND_ADD_FINGER_PRINT    0X08  // 增加指纹特征
#define COMMAND_ADD_FACIAL   0X09  // 增加人脸特征
#define COMMAND_DEV_IDENTIFY   0X0A  // 设备认证
#define COMMAND_TRANS_VIDEO     0X0B // 开始传输视频
#define COMMAND_STOP_VIDEO      0X0C    // 停止视频传输
#define COMMAND_FACIAL_REGISTER 0X0E    // 人脸注册
#define COMMAND_FINGER_REGISTER 0X0F    // 指纹注册
#define COMMAND_DOWNLOAD_USER   0X10    // 上位机更新下位机用户信息
#define COMMAND_DEL_USER        0X11    // 删除用户
#define COMMAND_GET_TEMPERATURE        0X12    // 体温测量


// 下位机命令
#define COMMAND_IDENTIFICATION       0X50  // 注册认证服务器
#define COMMAND_DATA_UPLOAD          0X51    // 传输检测信息
#define COMMAND_STATUS_CHANGE        0X52   // 设备状态变更

#define COMMAND_DEV_REGISTER        0X20    // 设备注册， 心跳包
#define COMMAND_NEW_USER        0X21    // 新用户上传
#define COMMAND_USER_RECOG      0X22    // 用户识别上报

// 下位机认证结果
#define SUCCESS         1
#define FAILUER         0

typedef struct msg_header {
    unsigned char mHFlag;
    unsigned char mFFlag;
    unsigned char mVersion;
    quint32  mCrc;
    quint32  mLength;
}  __attribute__ ((packed)) MSG_HEADER;


struct HealthInfo
{
    QString sys;
    QString dia;
    QString pulse;
    QString bbt;
};

// user info
struct UserInfo
{
    QString userName;
    QString userNumber;
    QString department;
    QString org;
    QString userID;
    bool    bLogin;
    HealthInfo health;
    QString alcoholTest;
    QImage userPic;
    QString userFP; // finger print
};

// files
#define FILE_ALCOHOL_CONSTANT  "AlcoholConstant"
#define FILE_USER_INFO          "User"
#define FILE_FP                "fpData.txt"


#endif // COMMON_H
