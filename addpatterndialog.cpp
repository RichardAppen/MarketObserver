#include "addpatterndialog.h"
#include "ui_addpatterndialog.h"
#include <mainwindow.h>
#include <QMessageBox>

addPatternDialog::addPatternDialog(QWidget *parent, QString incomingPatternName) :
    QDialog(parent),
    ui(new Ui::addPatternDialog)
{
    ui->setupUi(this);

    ui->patternNameLineEdit->setFocus();

    // After a change in plans "indicator 2" field is no longer in use but will be kept for now
    ui->indicator2lineEdit->setVisible(false);
    ui->label_3->setVisible(false);

    if (incomingPatternName != nullptr) {
        ui->patternNameLineEdit->setVisible(false);
        ui->label_4->setVisible(false);

        QSqlQuery querry;
        querry.prepare("select * from patternstable where name='" + incomingPatternName + "' ");

        if (querry.exec()) {
            qDebug() << "Found selected pattern to edit";

            while (querry.next()) {
                ui->patternNameLineEdit->setText(querry.value(0).toString());
                this->incomingPatternName = querry.value(0).toString();
                ui->descriptionTextEdit->setText(querry.value(1).toString());
                // Indicator 2 field currently in need of work
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
    QString patternName, description, indicator2;
    patternName = ui->patternNameLineEdit->text();
    description = ui->descriptionTextEdit->toPlainText();
    indicator2 = ui->indicator2lineEdit->text();

    // if we aren't editing
    if (incomingPatternName.compare("") == 0) {
        QSqlQuery querry;
        querry.prepare("insert into patternstable (name,description,indicator2) values ('" + patternName + "', '" + description + "','" + indicator2 + "')");

        if (querry.exec()) {
            qDebug() << "[Database] Inserted new pattern: " + patternName;
        } else {
            qDebug() << "[Database] Failed to insert new pattern: " + patternName;
            QMessageBox::critical(this, tr("error: "), tr("This pattern already exists!"));
        }
    } else {

        QSqlQuery querry;
        querry.prepare("update patternstable set name = '" + patternName + "', description = '" + description + "', indicator2 = '" + indicator2 + "' where name = '" + patternName + "';");

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
