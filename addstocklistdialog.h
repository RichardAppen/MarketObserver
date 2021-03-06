#ifndef ADDSTOCKLISTDIALOG_H
#define ADDSTOCKLISTDIALOG_H

#include <QDialog>
#include <QSqlDatabase>

namespace Ui {
class addStockListDialog;
}

class addStockListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit addStockListDialog(QWidget *parent = nullptr, QString incomingStockListName = "");
    ~addStockListDialog();
    QString incomingStockListName;
    QSqlDatabase maindb;

private slots:
    void on_finishNewStockListButton_clicked();

    void on_cancelNewStockListButton_clicked();

private:
    Ui::addStockListDialog *ui;
};

#endif // ADDSTOCKLISTDIALOG_H
