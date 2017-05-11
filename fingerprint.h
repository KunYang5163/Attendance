#ifndef FINGERPRINT_H
#define FINGERPRINT_H

#include <QThread>
#include <QLabel>

class FingerPrint : public QThread
{
    Q_OBJECT
public:
    explicit FingerPrint(QObject *parent = 0);

    static FingerPrint &instance(QWidget *parent = 0);

    void run();

    void onNotifyQt(const int count);

signals:
    void regReciever(void);

private:
//    QLabel *lbMsgDisplay;
};

#endif // FINGERPRINT_H
