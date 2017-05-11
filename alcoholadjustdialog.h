#ifndef ALCOHOLADJUSTDIALOG_H
#define ALCOHOLADJUSTDIALOG_H

#include <QDialog>
#include <QAbstractButton>

namespace Ui {
class AlcoholAdjustDialog;
}

class AlcoholAdjustDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AlcoholAdjustDialog(QWidget *parent = 0);
    ~AlcoholAdjustDialog();

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);

private:
    Ui::AlcoholAdjustDialog *ui;
};

#endif // ALCOHOLADJUSTDIALOG_H
