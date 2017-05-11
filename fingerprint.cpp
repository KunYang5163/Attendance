#include "fingerprint.h"
#include <QtAndroid>

FingerPrint::FingerPrint(QObject *parent):
    QThread(parent)
{

}


FingerPrint &FingerPrint::instance( QWidget *parent)
{
    static FingerPrint fp(parent);
    return fp;
}


void FingerPrint:: onNotifyQt(int count)
{
    qDebug("FingerPrint onNotifyQt is invoked!!!!!!!!!!!!!!!!!%d", count);
}


 void FingerPrint :: run()
 {
     QtAndroid::androidActivity().callMethod<jint>("OnEnroll", "()I");

     exec();
 }
