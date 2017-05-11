#ifndef STM32COMM_H
#define STM32COMM_H

#include <QObject>

#define STM_CMD_HEAD  0XA5
#define STM_CMD_TAIL  0X5A
#define STM_CMD_ALCO_TEMP_HUMI    0X01
#define STM_CMD_ALCO_ADC          0X02
#define STM_CMD_BP_OPEN           0X03
#define STM_CMD_BP_DATA           0X04
#define STM_CMD_BP_CLOSE          0X05
#define STM_CMD_INFRARED_TEMP     0X06
#define STM_CMD_BATTERY           0X07
#define STM_CMD_STATUS            0X08

#define STM_OP_SUCCESS          0X01
#define STM_OP_FAIL             0X00

#define STM_BIT_STATUS_SUCCESS     (1 << 7)
#define STM_BIT_STATUS_TH           (1 << 6)
#define STM_BIT_STATUS_ALCO         (1 << 5)
#define STM_BIT_STATUS_BP           (1 << 4)
#define STM_BIT_STATUS_INFRA        (1 << 3)
#define STM_BIT_STATUS_BAT          (1 << 2)

class Stm32Comm : public QObject
{
    Q_OBJECT
public:
    explicit Stm32Comm(QObject *parent = 0);
    ~Stm32Comm();

    int OpenBloodPresure(QString &err);
    int GetBloodPressureData(QString &err);
    int CloseBloodPressure(QString &err);
    int GetTemperature(QString &err);
    int GetTempAndHumi(QString &err);
    int GetAlcoholStrength(QString &err);
    int CheckDev(char &status);

private:
    int serial_fd;

signals:

public slots:
};

#endif // STM32COMM_H
