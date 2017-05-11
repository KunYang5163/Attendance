#include "stm32comm.h"
#include "uart.h"

#include <QDebug>
#include <QByteArray>

Stm32Comm::Stm32Comm(QObject *parent) : QObject(parent)
{
    QByteArray ba = QString(DEVICE_UART3).toLatin1();
    char *pDev = ba.data();
    serial_fd = UART_Init_COM(pDev, B115200);
}

Stm32Comm::~Stm32Comm()
{
    if(serial_fd >= 0)
    {
        UART_Close(serial_fd);
    }
}


int Stm32Comm::OpenBloodPresure(QString &err)
{
    if(serial_fd < 0)
    {
        QByteArray ba = QString(DEVICE_UART3).toLatin1();
        char *pDev = ba.data();
        serial_fd = UART_Init_COM(pDev, B115200);
    }

    if(serial_fd < 0)
    {
        err = QString("打开血压设备失败");
        return -1;
    }

    char bloodPressureOpenCmd[] = {STM_CMD_HEAD, STM_CMD_BP_OPEN, STM_CMD_TAIL};

    // send open command
    if(0 != UART_Send(serial_fd, bloodPressureOpenCmd, sizeof(bloodPressureOpenCmd)))
    {
        err = QString("发送数据失败");
        return -2;
    }

    // get response
    char buff[32] = {'\0'};
    int recvLen =  UART_Recv(serial_fd, buff, 4);

//    for(int i = 0; i < recvLen; i++)
//    {
//        qDebug("%02x ", buff[i]);
//    }

//    QByteArray ba(buff);

//    qDebug() << ba;

    if(recvLen < 4)
    {
//        err = QString("返回数据长度不足:")+QString::number(recvLen);
//        char prtbuff[5] = {0};
//        snprintf(prtbuff, sizeof(prtbuff), "%02X%02x%02x%02x", buff[0], buff[1], buff[2], buff[3]);
//        err = QString(prtbuff);
        return -3;
    }

    if(buff[0] != STM_CMD_HEAD || buff[1] != STM_CMD_BP_OPEN )
    {
        err = QString("返回数据格式有误");
        return -4;
    }

    if(buff[2] != STM_OP_SUCCESS)
    {
        err = QString("打开血压设备失败");
        return -5;
    }

    return 0;
}


int Stm32Comm::GetBloodPressureData(QString &err)
{
    if(serial_fd < 0)
    {
        QByteArray ba = QString(DEVICE_UART3).toLatin1();
        char *pDev = ba.data();
        serial_fd = UART_Init_COM(pDev, B115200);
    }

    if(serial_fd < 0)
    {
        err = QString("打开血压设备失败");
        return -1;
    }

    char bloodPressureCmd[] = {STM_CMD_HEAD, STM_CMD_BP_DATA, STM_CMD_TAIL};

    // send open command
    if(0 != UART_Send(serial_fd, bloodPressureCmd, sizeof(bloodPressureCmd)))
    {
        err = QString("发送数据失败");
        return -2;
    }

    // get response
    char lastValue = 0;
    char buff[32] = {'\0'};
    int recvLen =  UART_Recv(serial_fd, buff, 11);

    if(recvLen < 11)
    {
//        err = QString("返回数据长度不足:")+QString::number(recvLen);
        err = QString("血压设备未响应");
        return -3;
    }

    if(buff[0] != STM_CMD_HEAD || buff[1] != STM_CMD_BP_DATA )
    {
        err = QString("返回数据格式有误");
        return -4;
    }


    if(buff[2] != STM_OP_SUCCESS)
    {
        err = QString("打开血压设备失败");
        return -5;
    }

    if(buff[9] == 1)
    {
        err = QString("测量异常,请重新测量");
        return -6;
    }

    if(buff[7] == 1)
    {
        // test is done
        err = QString::number(buff[4])+":"+QString::number(buff[5])+":"+QString::number(buff[8]);
        return 1;
    }
    else
    {
        // test is in progress
        if(buff[6] >= lastValue)
        {
            lastValue = buff[6];
        }
        err = QString("测量中，请保持手指轻触血压仪[")+QString::number(lastValue * 10) + QString("%]");
        return 0;
    }


}



int Stm32Comm::CloseBloodPressure(QString &err)
{
    if(serial_fd < 0)
    {
        QByteArray ba = QString(DEVICE_UART3).toLatin1();
        char *pDev = ba.data();
        serial_fd = UART_Init_COM(pDev, B115200);
    }

    if(serial_fd < 0)
    {
        err = QString("打开血压设备失败");
        return -1;
    }

    char bloodPressureCmd[] = {STM_CMD_HEAD, STM_CMD_BP_CLOSE, STM_CMD_TAIL};

    // send open command
    if(0 != UART_Send(serial_fd, bloodPressureCmd, sizeof(bloodPressureCmd)))
    {
        err = QString("发送数据失败");
        return -2;
    }

    // get response
    char buff[32] = {'\0'};
    int recvLen =  UART_Recv(serial_fd, buff, 4);

    if(recvLen < 4)
    {
//        err = QString("返回数据长度不足:")+QString::number(recvLen);
        err = QString("血压设备未响应");
        return -3;
    }

    if(buff[0] != STM_CMD_HEAD || buff[1] != STM_CMD_BP_CLOSE )
    {
        err = QString("返回数据格式有误");
        return -4;
    }

    if(buff[2] != STM_OP_SUCCESS)
    {
//        err = QString("关闭血压计失败");
        return -5;
    }

    return 0;
}


int Stm32Comm::GetTemperature(QString &err)
{
    if(serial_fd < 0)
    {
        QByteArray ba = QString(DEVICE_UART3).toLatin1();
        char *pDev = ba.data();
        serial_fd = UART_Init_COM(pDev, B115200);
    }

    if(serial_fd < 0)
    {
        err = QString("打开体温设备失败");
        return -1;
    }

    char bloodPressureCmd[] = {STM_CMD_HEAD, STM_CMD_INFRARED_TEMP, STM_CMD_TAIL};

    // send open command
    if(0 != UART_Send(serial_fd, bloodPressureCmd, sizeof(bloodPressureCmd)))
    {
        err = QString("发送数据失败");
        return -2;
    }

    // get response
    char buff[32] = {'\0'};
    int recvLen =  UART_Recv(serial_fd, buff, 12);

//    for(int i = 0; i < recvLen; i++)
//        qDebug("%02x ", buff[i]);

    if(recvLen < 12)
    {
//        err = QString("返回数据长度不足:")+QString::number(recvLen);
        err = QString("体温设备未响应");
        return -3;
    }

    if(buff[0] != STM_CMD_HEAD || buff[1] != STM_CMD_INFRARED_TEMP )
    {
        err = QString("返回数据格式有误");
        return -4;
    }

    if(buff[2] != STM_OP_SUCCESS || 0xFF == buff[3])
    {
        err = QString("读取温度失败");
        return -5;
    }

    int temp = 0;
    char *p = (char *)&temp;
    p[0] = buff[6];
    p[1] = buff[5];
    p[2] = buff[4];
    p[3] = buff[3];

    err = QString::number(temp);
    qDebug() << err << ":" << temp;

    return 0;
}


int Stm32Comm :: GetTempAndHumi(QString &err)
{
    if(serial_fd < 0)
    {
        QByteArray ba = QString(DEVICE_UART3).toLatin1();
        char *pDev = ba.data();
        serial_fd = UART_Init_COM(pDev, B115200);
    }

    if(serial_fd < 0)
    {
        err = QString("打开测酒设备失败");
        return -1;
    }

    char bloodPressureCmd[] = {STM_CMD_HEAD, STM_CMD_ALCO_TEMP_HUMI, STM_CMD_TAIL};

    // send open command
    if(0 != UART_Send(serial_fd, bloodPressureCmd, sizeof(bloodPressureCmd)))
    {
        err = QString("发送数据失败");
        return -2;
    }

    // get response
    char buff[32] = {'\0'};
    int recvLen =  UART_Recv(serial_fd, buff, 6);

//    for(int i = 0; i < recvLen; i++)
//        qDebug("%02x ", buff[i]);

    if(recvLen < 6)
    {
//        err = QString("返回数据长度不足:")+QString::number(recvLen);
        err = QString("测酒设备未响应");
        return -3;
    }

    if(buff[0] != STM_CMD_HEAD || buff[1] != STM_CMD_ALCO_TEMP_HUMI )
    {
        err = QString("返回数据格式有误");
        return -4;
    }

    if(buff[2] != STM_OP_SUCCESS)
    {
        err = QString("读取温湿度失败");
        return -5;
    }

    // return HUMI only
    err = QString::number(buff[4]);
    qDebug() << err;

    return 0;
}


int Stm32Comm :: GetAlcoholStrength(QString &err)
{
    if(serial_fd < 0)
    {
        QByteArray ba = QString(DEVICE_UART3).toLatin1();
        char *pDev = ba.data();
        serial_fd = UART_Init_COM(pDev, B115200);
    }

    if(serial_fd < 0)
    {
        err = QString("打开测酒设备失败");
        return -1;
    }

    char bloodPressureCmd[] = {STM_CMD_HEAD, STM_CMD_ALCO_ADC, STM_CMD_TAIL};

    // send open command
    if(0 != UART_Send(serial_fd, bloodPressureCmd, sizeof(bloodPressureCmd)))
    {
        err = QString("发送数据失败");
        return -2;
    }

    // get response
    char buff[32] = {'\0'};
    int recvLen =  UART_Recv(serial_fd, buff, 6);

//    for(int i = 0; i < recvLen; i++)
//        qDebug("%02x ", buff[i]);

    if(recvLen < 6)
    {
//        err = QString("返回数据长度不足:")+QString::number(recvLen);
        err = QString("测酒设备未响应");
        return -3;
    }

    if(buff[0] != STM_CMD_HEAD || buff[1] != STM_CMD_ALCO_ADC )
    {
        err = QString("返回数据格式有误");
        return -4;
    }

    if(buff[2] != STM_OP_SUCCESS)
    {
        err = QString("读取酒精浓度失败");
        return -5;
    }

    // compute adc
    int adc = 0;
    char *p = (char *)&adc;
    p[0] = buff[4];
    p[1] = buff[3];

    err = QString::number(adc);

    adc = adc * 3300 / 4096;
    qDebug() << QString::number(adc);

    return 0;
}


/*
 * 获取stm32板上设备状态
 *
 * return
 *  -1：通信出错， err返回错误信息
 *  0：设备正常
 *  1: 设备异常，err返回出现异常的设备
 *
 */
int Stm32Comm :: CheckDev(char &status)
{
    status = 0xff;
    if(serial_fd < 0)
    {
        QByteArray ba = QString(DEVICE_UART3).toLatin1();
        char *pDev = ba.data();
        serial_fd = UART_Init_COM(pDev, B115200);
    }

    if(serial_fd < 0)
    {
        return -1;
    }

    char stmCmd[] = {STM_CMD_HEAD, STM_CMD_STATUS, STM_CMD_TAIL};

    // send open command
    if(0 != UART_Send(serial_fd, stmCmd, sizeof(stmCmd)))
    {
        return -1;
    }

    // get response
    char buff[32] = {'\0'};
    int expectLen = 9;
    int recvLen =  UART_Recv(serial_fd, buff, expectLen);

//    for(int i = 0; i < recvLen; i++)
//        qDebug("%02x ", buff[i]);

    if(0 == recvLen)
    {
        return -1;
    }

    if(recvLen < expectLen || buff[0] != STM_CMD_HEAD || buff[1] != STM_CMD_STATUS)
    {
        return -1;
    }

    if(buff[2] != STM_OP_SUCCESS)
    {
        status &= ~STM_BIT_STATUS_SUCCESS;
        return -1;
    }


    if(STM_OP_FAIL == buff[3])
    {
//        err = QString("[温湿度模块]");
        status &= ~STM_BIT_STATUS_TH;
    }
    if(STM_OP_FAIL == buff[4])
    {
//        err += QString("[酒精测试模块]");
        status &= ~STM_BIT_STATUS_ALCO;
    }
    if(STM_OP_FAIL == buff[5])
    {
//        err += QString("[血压模块]");
        status &= ~STM_BIT_STATUS_BP;
    }
    if(STM_OP_FAIL == buff[6])
    {
//        err += QString("[红外体温模块]");
        status &= ~STM_BIT_STATUS_INFRA;
    }
    if(STM_OP_FAIL == buff[7])
    {
//        err += QString("[电池]");
        status &= ~STM_BIT_STATUS_BAT;
    }

    if(buff[3] + buff[4] + buff[5] + buff[6] + buff[7] != 5)
    {
//        qDebug() << err;

        return 1;
    }


    return 0;
}
