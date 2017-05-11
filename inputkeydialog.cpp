#include "inputkeydialog.h"
#include "ui_inputkeydialog.h"

InputKeyDialog::InputKeyDialog(QWidget *parent) :
    QDialog(parent,Qt::FramelessWindowHint),
    ui(new Ui::InputKeyDialog)
{
    ui->setupUi(this);
}

InputKeyDialog::~InputKeyDialog()
{
    delete ui;
}

void InputKeyDialog::initView(QString ssid)
{
    this->resize(320,180);
    this->move(630,280);
//    this->setWindowFlags(Qt::FramelessWindowHint|Qt::WindowSystemMenuHint); // 设置成无边框对话框
    this->setStyleSheet("background-color:rgb(255,255,255);");

    ui->lblSsid->resize(180,25);
    ui->lblSsid->move(70,10);
    ui->lblSsid->setText(ssid);
    ui->lblSsid->setAlignment(Qt::AlignCenter);
    ui->lblSsid->setStyleSheet("background-color:rgba(0,0,0,0);color:black; font-size:16px");//text-align: center; font-size:16px");

    ui->btnCancel->resize(45,25);
    ui->btnCancel->move(15,10);
    ui->btnCancel->setFlat(true);
    ui->btnCancel->setStyleSheet("background-color:rgba(0,0,0,0);color:black; font-size:16px");

    ui->btnJoin->resize(45,25);
    ui->btnJoin->move(260,10);
    ui->btnJoin->setFlat(true);
    ui->btnJoin->setStyleSheet("background-color:rgba(0,0,0,0);color:rgb(88, 186, 236); font-size:16px");

    ui->leKey->resize(280,50);
    ui->leKey->move(20,80);
    ui->leKey->setPlaceholderText(QString("请输入 \"%1\" 的密码").arg(ssid));
    ui->leKey->setAlignment(Qt::AlignCenter);
    ui->leKey->setStyleSheet("background-color:rgb(192,192,192); color:black; font-size:16px");// text-align:center;");
    ui->leKey->setEchoMode(QLineEdit::Password  );
}

void InputKeyDialog::on_btnCancel_clicked()
{
    key = "";
    this->close();
}

void InputKeyDialog::on_btnJoin_clicked()
{
    key = ui->leKey->text();
    this->close();
}
