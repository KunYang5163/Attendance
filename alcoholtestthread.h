#ifndef AlcoholTestThread_H
#define AlcoholTestThread_H
#include <QThread>
#include <QObject>

class AlcoholTestThread : public QThread
{
    Q_OBJECT
public:
    AlcoholTestThread();

    void run();

private:
        int computeAlcohol(int adc);

        // alcohol test constant
        int voltage0;
        int voltage30;
        int voltage50;
        int voltage100;
        int voltage200;


signals:
    void sigShowMsg(const QString msg);
    void sigChangeAlcoholResult(int result);

};

#endif // AlcoholTestThread_H
