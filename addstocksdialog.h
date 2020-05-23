#ifndef ADDSTOCKSDIALOG_H
#define ADDSTOCKSDIALOG_H

#include <QDialog>

namespace Ui {
class addStocksDialog;
}

class addStocksDialog : public QDialog
{
    Q_OBJECT

public:
    explicit addStocksDialog(QWidget *parent = nullptr, QString incomingStockListName = "");
    ~addStocksDialog();
    QString stockName;
    QString incomingStockListName;

private slots:
    void on_addButton_clicked();

    void on_cancelButton_clicked();

private:
    Ui::addStocksDialog *ui;
};

#endif // ADDSTOCKSDIALOG_H
