#include <jni.h>

#include <QMetaObject>
#include <QDebug>
#include <QtAndroidExtras/QAndroidJniObject>
#include <QAndroidJniEnvironment>
#include <QtAndroid>

#include "maindialog.h"
//#include "fingerprint.h"

// define our native static functions
// these are the functions that Java part will call directly from Android UI thread
static void onNotifyEnroll(JNIEnv * /*env*/, jobject /*obj*/, jint count, jstring temp)
{
    qDebug() << "native.onNotifyEnroll";
    QAndroidJniObject myNewJavaString("java/lang/String", "(Ljava/lang/String;)V", temp);
//    QAndroidJniObject string = QAndroidJniObject::callObjectMethod<jstring>(temp, "getString");
//    QAndroidJniObject string = QAndroidJniObject::callStaticObjectMethod(
//            "java/lang/String", "getString", "(Ljava/lang/String;)Ljava/lang/String;", myNewJavaString.object<jstring>);

//    QString qstring = string.toString();
    QString qstring = myNewJavaString.toString();
    qDebug() << "str len:" << qstring.length();
    // call MainWindow::onReceiveMounted from Qt thread
    QMetaObject::invokeMethod(&mainDialog::instance(), "onNotifyEnroll"
                              , Qt::AutoConnection,  Q_ARG(int, (int)count), Q_ARG(QString, qstring));
}


static void onNotifyVerify(JNIEnv * /*env*/, jobject /*obj*/, jint userNumber)
{
    qDebug() << "native.onNotifyVerify";
    // call MainWindow::onReceiveMounted from Qt thread
    QMetaObject::invokeMethod(&mainDialog::instance(), "onNotifyVerify"
                              , Qt::AutoConnection,
                              Q_ARG(int, userNumber));
}


//create a vector with all our JNINativeMethod(s)
static JNINativeMethod methods[] = {
    {"onNotifyEnroll", "(ILjava/lang/String;)V", (void *)onNotifyEnroll},
    {"onNotifyVerify", "(I)V", (void *)onNotifyVerify},
};

// this method is called automatically by Java after the .so file is loaded
JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* /*reserved*/)
{
    JNIEnv* env;
    // get the JNIEnv pointer.
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK)
      return JNI_ERR;

    // search for Java class which declares the native methods
    jclass javaClass = env->FindClass("o2oAttendance/NativeFunctions");
    if (!javaClass)
      return JNI_ERR;

    // register our native methods
    if (env->RegisterNatives(javaClass, methods,
                          sizeof(methods) / sizeof(methods[0])) < 0) {
      return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}

