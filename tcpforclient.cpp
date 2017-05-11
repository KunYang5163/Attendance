/*
 * 实现下位机与上位机之间的TCP通信
 *
 *
 *
 */

#include "tcpforclient.h"
#include "qobjecthelper.h"
#include "parser.h"
#include "devicemanager.h"
#include <serializer.h>
#include "JQChecksum.h"
#include "common.h"
#include "clientthread.h"
#ifdef Q_OS_ANDROID
#include <QtAndroidExtras/QAndroidJniObject>
#endif
#include <QGuiApplication>

TCPForClient::TCPForClient(QLabel *label, QObject *parent)
    :QTcpServer(parent), textLabel(label)
{
    setMaxPendingConnections(1);
}


void TCPForClient :: setLabel(QLabel *label)
{
    textLabel = label;
}


void TCPForClient :: incomingConnection(qintptr socketDescriptor)
{
    qDebug(__func__);

    ClientThread *thread = new ClientThread(
                socketDescriptor,textLabel, this);
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    thread->start();
}



