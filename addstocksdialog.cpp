#include "addstocksdialog.h"
#include "ui_addstocksdialog.h"
#include <mainwindow.h>
#include <QMessageBox>

addStocksDialog::addStocksDialog(QWidget *parent, QString incomingStockListName) :
    QDialog(parent),
    ui(new Ui::addStocksDialog)
{
    ui->setupUi(this);
    this->incomingStockListName = incomingStockListName;
}

addStocksDialog::~addStocksDialog()
{
    delete ui;
}

void addStocksDialog::on_addButton_clicked()
{
    stockName = ui->newSymbolLineEdit->text();

    QSqlQuery querry;
    querry.prepare("create table if not exists '" + incomingStockListName + "$" + stockName + "datatable' (date varchar(20) primary key, pattern varchar(20), patternTriggered varchar(20), patternSuccess varchar(20), priceAtTrigger int, priceAtPeak int, dateOfPricePeak int)");

    if (querry.exec()) {
        qDebug() << "[Database] specific datatable for: " + stockName + " under list: " + incomingStockListName + " created or already exist";
    } else {
        qDebug() << "[Database] something is wrong with specific datatable intialization";
        QMessageBox::critical(this, tr("error: "), querry.lastError().text());
    }

    close();
}

void addStocksDialog::on_cancelButton_clicked()
{
    close();
}
