#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H

#include <QTcpSocket>
#include <QThread>
#include <QLabel>
#include <QCamera>
#include <QCameraImageCapture>

class ClientThread : public QThread
{
    Q_OBJECT
public:
    explicit ClientThread(int socketDes, QLabel *label = 0, QObject *parent = 0);

    void run();

signals:

public slots:
    void readData();
    void disconnected();

private:
    int socketDescriptor;
    QTcpSocket *socket;
    QLabel *textLabel;
    QCamera *camera;
    QCameraImageCapture *imageCapture;
    QTimer *captureTimer;
    bool isCameraStarted;
    int alcValue;

    bool sendPackage(unsigned char command, QByteArray data);
    void packageHandler_devinfo();
    void packageHandler_displayText(quint32 len, quint32 crc);
    void packageHandler_playSound(quint32 len, quint32 crc);
    void packageHandler_checkDev();
    void packageHandler_identification();
    void packageHandler_alcoholDetection(quint32 len, quint32 crc);
    void packageHandler_delFingerPrint(quint32 len, quint32 crc);
    void packageHandler_delFacial(quint32 len, quint32 crc);
    void packageHandler_addFingerPrint(quint32 len, quint32 crc);
    void packageHandler_addFacial(quint32 len, quint32 crc);
    void packageHandler_transportVideo();
    void setupCamera();
    void takeImage();
    void packageHandler_stopVideo();
    void packageHandler_facialRegister(quint32 len, quint32 crc);
    void packageHandler_fingerRegister(quint32 len, quint32 crc);
    void packageHandler_downloadUser(quint32 len, quint32 crc);
    void packageHandler_delUser(quint32 len, quint32 crc);
    void packageHandler_getTemp(quint32 len, quint32 crc);

private slots:
    void processCapturedImage(int requestId, const QImage& img);
    void reportDevStatusPer10min();
    void getAlcValue(int v);

};

#endif // CLIENTTHREAD_H
