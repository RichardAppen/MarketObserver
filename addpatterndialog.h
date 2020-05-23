#ifndef ADDPATTERNDIALOG_H
#define ADDPATTERNDIALOG_H

#include <QDialog>

namespace Ui {
class addPatternDialog;
}

class addPatternDialog : public QDialog
{
    Q_OBJECT

public:
    explicit addPatternDialog(QWidget *parent = nullptr, QString incomingPatternName = "");
    ~addPatternDialog();
    QString incomingPatternName;

private slots:
    void on_finishNewPatternButton_clicked();

    void on_cancelNewPatternButton_clicked();

private:
    Ui::addPatternDialog *ui;
};

#endif // ADDPATTERNDIALOG_H
