#include "alcoholadjustdialog.h"
#include "ui_alcoholadjustdialog.h"
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include "common.h"

AlcoholAdjustDialog::AlcoholAdjustDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AlcoholAdjustDialog)
{
    ui->setupUi(this);

    ui->label0->resize(120,70);
    ui->label0->move(290,10);


    ui->le0->resize(200,70);
    ui->le0->move(500,10);

    ui->le0->setStyleSheet("QLineEdit{"
                              "border: 1px solid rgb(162, 155, 144);"
                              "border-radius: 3px;"
                              "background-color: rgb(255, 255, 255, 0);"
                              "color:gray;"
                              "font-size: 24px;}");

    ui->label30->resize(120,70);
    ui->label30->move(290,90);


    ui->le30->resize(200,70);
    ui->le30->move(500,90);

    ui->le30->setStyleSheet("QLineEdit{"
                              "border: 1px solid rgb(162, 155, 144);"
                              "border-radius: 3px;"
                              "background-color: rgb(255, 255, 255, 0);"
                              "color:gray;"
                              "font-size: 24px;}");
    ui->label50->resize(120,70);
    ui->label50->move(290,180);


    ui->le50->resize(200,70);
    ui->le50->move(500,180);

    ui->le50->setStyleSheet("QLineEdit{"
                              "border: 1px solid rgb(162, 155, 144);"
                              "border-radius: 3px;"
                              "background-color: rgb(255, 255, 255, 0);"
                              "color:gray;"
                              "font-size: 24px;}");
    ui->label100->resize(120,70);
    ui->label100->move(290,270);


    ui->le100->resize(200,70);
    ui->le100->move(500,270);

    ui->le100->setStyleSheet("QLineEdit{"
                              "border: 1px solid rgb(162, 155, 144);"
                              "border-radius: 3px;"
                              "background-color: rgb(255, 255, 255, 0);"
                              "color:gray;"
                              "font-size: 24px;}");
    ui->label200->resize(120,70);
    ui->label200->move(290,360);


    ui->le200->resize(200,70);
    ui->le200->move(500,360);

    ui->le200->setStyleSheet("QLineEdit{"
                              "border: 1px solid rgb(162, 155, 144);"
                              "border-radius: 3px;"
                              "background-color: rgb(255, 255, 255, 0);"
                              "color:gray;"
                              "font-size: 24px;}");

    ui->buttonBox->resize(200,70);
    ui->buttonBox->move(340, 500);
}

AlcoholAdjustDialog::~AlcoholAdjustDialog()
{
    delete ui;
}


void AlcoholAdjustDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    if(ui->buttonBox->button(QDialogButtonBox::Ok)  == (QPushButton *)button)
    {
        if(0 == ui->le30->text().length() || 0 == ui->le50->text().length() || 0 == ui->le100->text().length()
                || 0 == ui->le200->text().length())
        {
            QMessageBox::information(this, "", "缺少实验常量");

            return ;
        }

        // save to file
        QFile file(FILE_ALCOHOL_CONSTANT);
        file.open(QIODevice::WriteOnly | QIODevice::Truncate);

        QTextStream txtOutput(&file);
        QString strToSave = ui->le0->text() + ":" + ui->le30->text() + ":" + ui->le50->text() + ":" +
                ui->le100->text() + ":" + ui->le200->text();
        txtOutput << strToSave << endl;
        file.close();


    }
}
