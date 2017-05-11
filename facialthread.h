#ifndef FACIALTHREAD_H
#define FACIALTHREAD_H

#include <QThread>
#include <QObject>
#include <QString>
#include "common.h"

class FacialThread : public QThread
{
    Q_OBJECT
public:
    FacialThread(int userNum);


    void run();

private:
    int userNumber;
    UserInfo currentUser;

signals:
    void sigChangeMsg(const QString msg);
    void sigUpdateUserInfo(const UserInfo uInfo);
};

#endif // FACIALTHREAD_H
