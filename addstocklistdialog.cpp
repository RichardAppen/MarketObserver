#include "addstocklistdialog.h"
#include "ui_addstocklistdialog.h"
#include <mainwindow.h>
#include <QMessageBox>
#include <QSqlDatabase>

addStockListDialog::addStockListDialog(QWidget *parent, QString incomingStockListName) :
    QDialog(parent),
    ui(new Ui::addStockListDialog)
{
    ui->setupUi(this);
    ui->stockListNameLineEdit->setFocus();

    if (incomingStockListName != nullptr) {

        this->incomingStockListName = incomingStockListName;

        QSqlQuery querry;
        querry.prepare("select * from stocklisttable where name='" + incomingStockListName + "' ");

        if (querry.exec()) {
            qDebug() << "Found selected stockList to edit";

            while (querry.next()) {
                ui->stockListNameLineEdit->setText(querry.value(0).toString());
                //ui->patternToUseLineEdit->setText(querry.value(1).toString());
                //ui->patternToUseLineEdit->setVisible(false);
                ui->patternToUseComboBox->setVisible(false);
                ui->label_2->setVisible(false);

            }

        } else {
            qDebug() << "Failed to deal with selected stockList to edit";
            QMessageBox::critical(this, tr("error"), querry.lastError().text());
        }

    } else {
        //Load the patterns combo box
        QSqlQueryModel * model = new QSqlQueryModel();
        QSqlQuery querry2;
        querry2.prepare("select name from patternstable");
        if (querry2.exec()) {

        } else {
            QMessageBox::critical(this, tr("error: "), querry2.lastError().text());
        }
        model->setQuery(querry2);
        ui->patternToUseComboBox->setModel(model);
    }

}

addStockListDialog::~addStockListDialog()
{
    delete ui;
}

void addStockListDialog::on_finishNewStockListButton_clicked()
{
    QString stockListName, patternToUse;
    stockListName = ui->stockListNameLineEdit->text();
    patternToUse = ui->patternToUseComboBox->currentText();



    // if we aren't editing
    if (incomingStockListName.compare("") == 0) {

        QSqlQuery querry;
        querry.prepare("insert into stocklisttable (name,pattern) values ('" + stockListName + "', '" + patternToUse + "')");

        if (querry.exec()) {
            qDebug() << "[Database] Inserted new stock list: " + stockListName;
        } else {
            qDebug() << "[Database] Failed to insert new stock list: " + stockListName;
            QMessageBox::critical(this, tr("error: "), tr("This stock list already exists!"));
        }

    } else {

        QSqlQuery querry;
        querry.prepare("update stocklisttable set name = '" + stockListName + "', pattern = '" + patternToUse + "' where name = '" + incomingStockListName + "';");

        if (querry.exec()) {
            qDebug() << "[Database] Edited the stock list: " + stockListName;

        } else {
            qDebug() << "[Database] Failed to edit the stock list: " + stockListName;
            QMessageBox::critical(this, tr("error: "), tr("Couldn't find the stock list you were trying to edit!"));
        }

    }


    close();
}

void addStockListDialog::on_cancelNewStockListButton_clicked()
{
    close();
}
