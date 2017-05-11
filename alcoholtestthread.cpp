#include "alcoholtestthread.h"
#include "stm32comm.h"
#include "common.h"
#include <QFile>
#include <qDebug>


AlcoholTestThread::AlcoholTestThread()
{
    // reset constants
    voltage0 = -1;
    voltage30 = -1;
    voltage50 = -1;
    voltage100 = -1;
    voltage200 = -1;

    // load constants from file
    QFile file(FILE_ALCOHOL_CONSTANT);
    file.open(QIODevice::ReadOnly);

    QTextStream txtInput(&file);
    QString strToSave;
    txtInput >> strToSave;
    file.close();

    if(!strToSave.isEmpty())
    {
        QStringList strList = strToSave.split(":");
        QString value = strList.value(0);
        voltage0 = value.toInt();
        value = strList.value(1);
        voltage30 = value.toInt();
        value = strList.value(2);
        voltage50 = value.toInt();
        value = strList.value(3);
        voltage100 = value.toInt();
        value = strList.value(4);
        voltage200 = value.toInt();
    }
}


void AlcoholTestThread::run()
{
    // check alcohol constant
    if( voltage0 < 0 || voltage30 < 0 || voltage50 < 0 || voltage100 < 0 || voltage200 < 0)
    {
        sigShowMsg("未校准测酒模块");
        return ;
    }
    QString err;

    Stm32Comm *stm = new Stm32Comm();

    int ret = 0;

    // watch tempearture and humi
    sigShowMsg("请吹气测酒");
    QThread::sleep(1);
    err.clear();
    int retry = 5;
//    int baseValue = 0;


//    while(retry)
//    {
//        ret = stm->GetTempAndHumi(err);
//        qDebug() << "ret:" << ret;
//        if(0 == ret)
//        {
//            if(abs(err.toInt() - baseValue) < 2)
//            {
//                baseValue = err.toInt();
//                break;
//            }
//            baseValue = err.toInt();
//        }
//        QThread::sleep(1);
//        --retry;
//    }

//    if(0 == baseValue)
//    {
//        // error occured
//        sigShowMsg("读取测酒仪湿度失败");

//        delete stm;
//        return;
//    }
#if 0
    retry = 20;

    while(retry)
    {
        ret = stm->GetTempAndHumi(err);

        if(0 == ret)
        {
            if(abs(err.toInt() - baseValue) > 5)
            {
                // user started blowing
                break;
            }
            QThread::yieldCurrentThread();
        }
        QThread::sleep(1);
        --retry;
    }

    if(0 == retry)
    {
        sigShowMsg("测试超时");
        delete stm;
        return;
    }
#endif

    sigShowMsg("正在测量酒精浓度，请保持吹气");
    retry = 10;

    int max = 0;
    while(retry--)
    {
        ret = stm->GetAlcoholStrength(err);

        if(0 == ret)
        {
           if(err.toInt() > max)
           {
               max = err.toInt();
           }
        }

        QThread::sleep(1);
        QThread::yieldCurrentThread();
    }

    max = max * 3300 / 4096;

    max = computeAlcohol(max);

    qDebug() << "max alcohol:" << max;
    sigChangeAlcoholResult(max);
    sigShowMsg("测量酒精结束");

    delete stm;
}


/*
 * 通过电压值和实验常数计算酒精浓度
 */
int AlcoholTestThread::computeAlcohol(int adc)
{
    int x1 = 0, x2 = 0, y1 = 0, y2 = 0;
    int ret = 0;

    qDebug() << voltage0 << ":" << voltage30  << ":" << voltage50 << ":" << voltage100 <<
                ":" << voltage200;
    qDebug() << adc;
    if(adc > voltage200)
    {
        return 200;
    }
    else if(adc > voltage100)
    {
        x1 = 100;
        x2 = 200;
        y1 = voltage100;
        y2 = voltage200;
    }
    else if(adc > voltage50)
    {
        x1 = 50;
        x2 = 100;
        y1 = voltage50;
        y2 = voltage100;
    }
    else if(adc > voltage30)
    {
        x1 = 30;
        x2 = 50;
        y1 = voltage30;
        y2 = voltage50;
    }
    else if(adc > voltage0)// 0 ~ 30
    {
        x1 = 0;
        x2 = 30;
        y1 = voltage0;
        y2 = voltage30;
    }
    else
    {
        return 0;
    }

    ret = (adc - y1) * (x2 - x1) / (y2 - y1) + x1;

    qDebug() << ret;

    return ret;
}
