#include <QFileInfo>
#include <QDir>
#include <unistd.h>
#include <QNetworkInterface>
#include <QProcess>

#include "devicemanager.h"


DeviceManager::DeviceManager(QObject *parent) :
    QObject(parent)
{

}


/*
 * 从设备文件中读取设备基本信息
 *
 */
void DeviceManager :: getDevInfo(QList<DevNode *> &nodeList)
{
    QDomDocument doc;
    QFile file(DEV_FILE);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "failed to open devinfo.xml" ;
        return;
    }
    QString strError;
    int errLine;
    int errColumn;
    if(!doc.setContent(&file, &strError, &errLine, &errColumn))
    {
        qDebug() << "parse xml error : " << strError
                    << "line" << errLine << "col" << errColumn;
        file.close();
        return;
    }
    file.close();
    QDomElement element = doc.documentElement();


    if(!element.isNull())
    {
        DevNode *node = new DevNode();
        node->name = "name";  // 0
        node->value = element.firstChildElement("name").text();
        nodeList.append(node);

        node = new DevNode();
        node->name = "id";      // 1
        node->value = element.firstChildElement("id").text();
        nodeList.append(node);

        node = new DevNode();
        node->name = "status";  // 2
        node->value = element.firstChildElement("status").text();
        nodeList.append(node);

        node = new DevNode();
        node->name = "signature";   // 3
        node->value = element.firstChildElement("signature").text();
        nodeList.append(node);

        node = new DevNode();
        node->name = "appversion";  // 4
        node->value = element.firstChildElement("appversion").text();
        nodeList.append(node);

        node = new DevNode();
        node->name = "wiredMac";  // 5
        node->value = element.firstChildElement("wiredMac").text();
        nodeList.append(node);

        node = new DevNode();
        node->name = "wirelessMac";  // 6
        node->value = element.firstChildElement("wirelessMac").text();
        nodeList.append(node);
    }

    return;
}


/*
 * 从设备文件中读取server信息
 */
bool DeviceManager :: getServerInfo(QList<DevNode *> &nodeList)
{
    QDomDocument doc;
    QFile file(DEV_FILE);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "failed to open ./devinfo.xml" ;
        return false;
    }
    QString strError;
    int errLine;
    int errColumn;
    if(!doc.setContent(&file, &strError, &errLine, &errColumn))
    {
        qDebug() << "parse xml error : " << strError
                    << "line" << errLine << "col" << errColumn;
        file.close();
        return false;
    }
    file.close();
    QDomElement element = doc.documentElement();

//    qDebug() << doc.toString();

    if(element.isNull())
        return false;

    DevNode *node = new DevNode();
    node->name = "serverip";  // 0
    node->value = element.firstChildElement("serverip").text();

    nodeList.append(node);

    node = new DevNode();
    node->name = "serverport";      // 1
    node->value = element.firstChildElement("serverport").text();

    nodeList.append(node);

    return true;
}


/*
 * 创建默认设备信息文件
 *
 */
void DeviceManager :: createDefaultDevInfo()
{

   QFileInfo fileinfo(DEV_FILE);
    if(fileinfo.isFile())
    {
        // dev file exists, return
        qDebug() << DEV_FILE << " exists";
        return;
    }

    qDebug() << "Creating dev file : " << DEV_FILE;

    QFile file(DEV_FILE);

    if(!file.open(QIODevice::WriteOnly))
    {
        perror("open");
        qDebug() << "Failed to open " << DEV_FILE;
        return ;
    }
    QDomDocument doc;
    QDomProcessingInstruction instruction;
    instruction = doc.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(instruction);
    QDomElement root=doc.createElement("devinfo");
    doc.appendChild(root);

    QDomElement name=doc.createElement("name");
    QDomElement id=doc.createElement("id");
    QDomElement status=doc.createElement("status");
    QDomElement signature=doc.createElement("signature");
    QDomElement appversion=doc.createElement("appversion");
    QDomElement serverip=doc.createElement("serverip");
    QDomElement serverport=doc.createElement("serverport");
    QDomElement wiredMac=doc.createElement("wiredMac");
    QDomElement wirelessMac=doc.createElement("wirelessMac");
    QDomText text;
    text=doc.createTextNode("Station01");
    name.appendChild(text);
    text=doc.createTextNode("100101");
    id.appendChild(text);
    text=doc.createTextNode("正常");
    status.appendChild(text);
    text=doc.createTextNode("12345678");
    signature.appendChild(text);
    text=doc.createTextNode("v0.1");
    appversion.appendChild(text);
    text=doc.createTextNode("192.168.199.137");
    serverip.appendChild(text);
    text=doc.createTextNode("9000");
    serverport.appendChild(text);

    // add MAC address infos
    QStringList arguments;
    arguments << "/sys/class/net/eth0/address";
    QProcess *myProcess = new QProcess(this);
    myProcess->start("cat", arguments);
    myProcess->waitForFinished();
    QString addr =  myProcess->readAllStandardOutput();
    addr.remove('\n');
    qDebug() <<addr;
    text=doc.createTextNode(addr);
    wiredMac.appendChild(text);

    // wireless MAC address
    arguments.clear();
    arguments << "/sys/class/net/wlan0/address";
    myProcess->start("cat", arguments);
    myProcess->waitForFinished();
    addr =  myProcess->readAllStandardOutput();
    addr.remove('\n');
    qDebug() <<addr;
    text=doc.createTextNode(addr);
    wirelessMac.appendChild(text);



    root.appendChild(name);
    root.appendChild(id);
    root.appendChild(status);
    root.appendChild(signature);
    root.appendChild(appversion);
    root.appendChild(serverip);
    root.appendChild(serverport);
    root.appendChild(wiredMac);
    root.appendChild(wirelessMac);
    QTextStream out(&file);
    doc.save(out,4);
    file.close();
}


/*
 *  检查STM32设备状态
 *
 *  所有设备正常ret返回0，通信出错返回-1， 有设备出现异常返回1
 *
 *  #define STM_BIT_STATUS_SUCCESS     (1 << 7)
    #define STM_BIT_STATUS_TH           (1 << 6)
    #define STM_BIT_STATUS_ALCO         (1 << 5)
    #define STM_BIT_STATUS_BP           (1 << 4)
    #define STM_BIT_STATUS_INFRA        (1 << 3)
    #define STM_BIT_STATUS_BAT          (1 << 2)

    status 中对应各设备的位被置1，则该设备正常，置0则设备异常。
 */
void DeviceManager :: checkDevice(int &ret, char &status)
{
    Stm32Comm * stmCom = new Stm32Comm(this);
//    QString msg;
    ret = stmCom->CheckDev(status);
//    if(0 == ret)
//    {
//        err = QString("正常");
//    }
//    else if(-1 == ret)
//    {
//        err = msg;
//    }
//    else if(1 == ret)
//    {
//        err = msg + QString("异常");
//    }
}

/*
 *  更新设备信息文件
 *
 *  nodeList中的每个节点将被更新到文件中
 *
 *  操作成功返回0， 失败返回-1
 */
int DeviceManager :: updateDevInfo(QList<DevNode *> nodeList)
{
    QFile file(DEV_FILE);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "open error";
        file.close();

        return -1;
    }

    QDomDocument doc;
    if(!doc.setContent(&file))
    {
        qDebug() << "setcontent error";
        file.close();

        return -1;
    }
    file.close();

    // update value of the nodes specified in nodeList
    DevNode *pDev;
    foreach (pDev, nodeList)
    {
        QDomNodeList lists = doc.elementsByTagName(pDev->name);
        QDomElement ele = lists.at(0).toElement();
        ele.firstChild().setNodeValue(pDev->value);
    }

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "open for update error!";
        return -1;
    }
    QTextStream out(&file);
    doc.save(out,4);
    file.close();

    return 0;

}
