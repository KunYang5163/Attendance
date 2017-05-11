#ifndef HEALTERTESTTHREAD_H
#define HEALTERTESTTHREAD_H
#include <QObject>
#include <QThread>

class HealthTestThread : public QThread
{
    Q_OBJECT
public:
    HealthTestThread(int opt = 0);
    bool stopRunning;

    void run();

private:
    int opt;

signals:
    void sigShowMsg(const QString msg);
    void sigChangeHealthTestSYS(int sys);
    void sigChangeHealthTestDIA(int dia);
    void sigChangeHealthTestPUISE(int puise);
    void sigChangeHealthTestBBT(int bbt);

public slots:
    void on_flagChanged(bool flag);


};

#endif // HEALTERTESTTHREAD_H
