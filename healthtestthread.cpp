#include "healthtestthread.h"

#include "stm32comm.h"
#include <QDebug>


HealthTestThread::HealthTestThread(int optValue)
{
    opt = optValue;
    stopRunning = false;
}

void HealthTestThread::on_flagChanged(bool flag)
{
    stopRunning = flag;
}


void HealthTestThread :: run()
{
    QString err;

    Stm32Comm *stm = new Stm32Comm();

    int ret = 0;

    // bp test
    if(0 == opt)
    {
//        stm->CloseBloodPressure(err);

        // might restart the test
        int retry = 3;
startTest:
        {
            ret = stm->OpenBloodPresure(err);

            if(0 == ret)
            {
                sigShowMsg("请把手指轻按到血压仪上");
            }
            else
            {
                if(0 == retry)
                {
                    if(err.length() > 0)
                        sigShowMsg(err);
                    else
                        sigShowMsg("打开血压设备失败");

                    return ;
                }

                --retry;
                goto startTest;

            }

            QThread::sleep(1);

            ret = 0;
            err.clear();

            while(ret <= 0 && ret != -6)
            {
                ret = stm->GetBloodPressureData(err);

                if(0 == ret)
                {
                    sigShowMsg(err);
                }
                QThread::yieldCurrentThread();
                QThread::sleep(3);
                if(stopRunning)
                {
                    stm->CloseBloodPressure(err);
                    break;
                }

            }

            if(1 == ret)
            {
                qDebug() << err;
                QStringList strList = err.split(":");
                QString value = strList.value(0);
                sigChangeHealthTestSYS(value.toInt());
                value = strList.value(1);
                sigChangeHealthTestDIA(value.toInt());
                value = strList.value(2);
                sigChangeHealthTestPUISE(value.toInt());


                sigShowMsg("测量结束");
                stm->CloseBloodPressure(err);

            }

            if(-6 == ret)
            {
                // timeout, restart the test
                stm->CloseBloodPressure(err);
                goto startTest;
            }
        }
    }
    else if(1 == opt)
    {

        // temperature test
        sigShowMsg("请将额头对准红外体温计");
//        QThread::sleep(1);
        err.clear();
        int retry = 3;
        int temp = 0;
        while(retry)
        {
            ret = stm->GetTemperature(err);

            if(0 == ret)
            {
                if(err.toInt() > temp)
                {
                    temp = err.toInt();
                    sigChangeHealthTestBBT(temp + 250);
                }
            }
            QThread::yieldCurrentThread();
            QThread::sleep(1);
            --retry;
        }
        sigShowMsg("体温测量结束");
    }

    delete stm;
}
