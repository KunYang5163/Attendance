#ifndef UPLOADTHREAD_H
#define UPLOADTHREAD_H

#include <QTcpSocket>
#include <QThread>

class UploadThread : public QThread
{
    Q_OBJECT
public:
    explicit UploadThread(QObject *parent = 0);
    void run();

private:
    QTcpSocket *socket;

private slots:
    void devAuthentication();

signals:
    void sigShowMsg(const QString msg);


};

#endif // UPLOADTHREAD_H
