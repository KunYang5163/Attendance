#ifndef TCPFORCLIENT_H
#define TCPFORCLIENT_H


#include <QTcpServer>
#include <QTcpSocket>
#include <QLabel>

class TCPForClient : public QTcpServer
{
    Q_OBJECT
public:
    TCPForClient(QLabel *label = 0, QObject *parent = 0);
    void setLabel(QLabel *label);

protected:
    void incomingConnection(qintptr socketDescriptor);


private:
   QLabel *textLabel;
};


#endif // TCPFORCLIENT_H
