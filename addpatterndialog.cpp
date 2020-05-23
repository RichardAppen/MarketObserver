#include "addpatterndialog.h"
#include "ui_addpatterndialog.h"
#include <mainwindow.h>
#include <QMessageBox>

addPatternDialog::addPatternDialog(QWidget *parent, QString incomingPatternName) :
    QDialog(parent),
    ui(new Ui::addPatternDialog)
{
    ui->setupUi(this);

    if (incomingPatternName != nullptr) {

        QSqlQuery querry;
        querry.prepare("select * from patternstable where name='" + incomingPatternName + "' ");

        if (querry.exec()) {
            qDebug() << "Found selected pattern to edit";

            while (querry.next()) {
                ui->patternNameLineEdit->setText(querry.value(0).toString());
                ui->indicator1LineEdit->setText(querry.value(1).toString());
                ui->indicator2lineEdit->setText(querry.value(2).toString());
            }

        } else {
            qDebug() << "Failed to deal with selected pattern to edit";
            QMessageBox::critical(this, tr("error"), querry.lastError().text());
        }


    }

}

addPatternDialog::~addPatternDialog()
{

    delete ui;

}

void addPatternDialog::on_finishNewPatternButton_clicked()
{
    QString patternName, indicator1, indicator2;
    patternName = ui->patternNameLineEdit->text();
    indicator1 = ui->indicator1LineEdit->text();
    indicator2 = ui->indicator2lineEdit->text();

    // if we aren't editing
    if (incomingPatternName.compare("") == 0) {
        qDebug() << "test 1.2.3... " << incomingPatternName;
        QSqlQuery querry;
        querry.prepare("insert into patternstable (name,indicator1,indicator2) values ('" + patternName + "', '" + indicator1 + "','" + indicator2 + "')");

        if (querry.exec()) {
            qDebug() << "[Database] Inserted new pattern: " + patternName;
        } else {
            qDebug() << "[Database] Failed to insert new pattern: " + patternName;
            QMessageBox::critical(this, tr("error: "), tr("This pattern already exists!"));
        }
    } else {

        QSqlQuery querry;
        querry.prepare("update patternstable set name = '" + patternName + "', indicator1 = '" + indicator1 + "', indicator2 = '" + indicator2 + "' where name = '" + patternName + "';");

        if (querry.exec()) {
            qDebug() << "[Database] Edited the pattern: " + patternName;

        } else {
            qDebug() << "[Database] Failed to edit the pattern: " + patternName;
            QMessageBox::critical(this, tr("error: "), tr("Couldn't find the pattern you were trying to edit!"));
        }
    }


    close();
}

void addPatternDialog::on_cancelNewPatternButton_clicked()
{
    close();
}
