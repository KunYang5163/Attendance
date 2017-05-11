#ifndef INPUTKEYDIALOG_H
#define INPUTKEYDIALOG_H

#include <QDialog>

namespace Ui {
class InputKeyDialog;
}

class InputKeyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InputKeyDialog(QWidget *parent = 0);
    ~InputKeyDialog();

    void initView(QString ssid);
    QString getKey() {return key;}

private slots:
    void on_btnCancel_clicked();

    void on_btnJoin_clicked();

private:


private:
    Ui::InputKeyDialog *ui;

    QString key;
};

#endif // INPUTKEYDIALOG_H
