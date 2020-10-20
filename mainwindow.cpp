#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include "QStackedWidget"
#include <vector>
#include <pattern.h>
#include <addpatterndialog.h>
#include <addstocklistdialog.h>
#include <addstocksdialog.h>
#include <QItemDelegate>
using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // On entrance to the program set the window to the homescreen
    ui->stackedWidget->setCurrentIndex(0);

    // Load up the user app data location, where to store the databases
    setAppDataDirectory();

    // Open the maindatabase on application open
    sqlOpen();

    // This was kept incase revert was necessary
    ui->patternSuccessStockLineEdit->setVisible(false);
    ui->patternSuccessStockLineEdit->setEnabled(false);
    ui->patternTriggeredStockLineEdit->setVisible(false);
    ui->patternTriggeredStockLineEdit->setEnabled(false);

    ui->patternSuccessRadioButton->setAutoExclusive(false);
    ui->patternTriggeredRadioButton->setAutoExclusive(false);

    ui->tradeTrackerButton->setVisible(false);

}

bool MainWindow::sqlOpen() {
    appdataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    mainDataBase = QSqlDatabase::addDatabase("QSQLITE");
    mainDataBase.setDatabaseName(appdataLocation + "/maindatabase.db");

    if (!mainDataBase.open()) {
         //qDebug() << "[Database] failed to open maindatabase.db";
        qDebug() << "+";
         return false;

    } else {
        //qDebug() << "[Database] maindatabase.db opened...";
        return true;
    }
}

void MainWindow::sqlClose() {


    mainDataBase.close();
    mainDataBase.removeDatabase("QSQLITE");
    qDebug() << "-";
}

void MainWindow::setAppDataDirectory() {
    appdataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    QDir dir;
    if (dir.mkdir(appdataLocation)) {
        qDebug() << "[Database] Success: creates directory:" + appdataLocation;
    } else {
        qDebug() << "[Database] Didn't create appdata directory, it probably already exists.";
    }

}

MainWindow::~MainWindow()
{

    delete ui;
    sqlClose();
}

/* fileExist
 *
 * @params path : QString that contains the path to the file
 * @returns bool : true if it does exist
 */
bool fileExists(QString path) {
    QFileInfo check_file(path);
    // check if path exists and if yes: Is it really a file and no directory?
    return check_file.exists() && check_file.isFile();
}


void MainWindow::on_patternsButton_clicked()
{
    // When the patterns button is clicked go to the patterns page
    ui->stackedWidget->setCurrentIndex(1);

    QSqlQuery querry1;
    querry1.prepare("create table if not exists patternstable (name varchar(20) primary key, description varchar(20), indicator2 varchar(20))");

    if (querry1.exec()) {
        qDebug() << "[Database] patternstable created or already exist";
    } else {
        qDebug() << "[Database] something is wrong with patternstable intialization";
        QMessageBox::critical(this, tr("error: "), querry1.lastError().text());
    }


    //Load the patterns list view
    QSqlQueryModel * model = new QSqlQueryModel();
    QSqlQuery * querry2 = new QSqlQuery(mainDataBase);
    querry2->prepare("select name from patternstable");
    querry2->exec();
    model->setQuery(*querry2);
    ui->patternsListView->setModel(model);

    ui->editNewPatternsButton->setEnabled(false);
    ui->removeNewPatternsButton->setEnabled(false);

    ui->additionalInfoBrowser->setText("");

    // Set up patternsInUse
    QSqlQuery * querry3 = new QSqlQuery(mainDataBase);
    querry3->prepare("select * from stocklisttable");

    if (querry3->exec()) {
        while (querry3->next()) {
            QString currPattern = querry3->value(1).toString();
            patternsInUse.insert(currPattern);
        }
    } else {
        QMessageBox::critical(this, tr("error: "), tr("error setting up patternsInUse"));
    }

    for (auto i : patternsInUse) {
        qDebug() << "printing out patterns in use: " + i;
    }


}

void MainWindow::on_stockListButton_clicked()
{
    //When the stock list button is clicked go to the stock list page
    ui->stackedWidget->setCurrentIndex(2);

    QSqlQuery querry1;
    querry1.prepare("create table if not exists stocklisttable (name varchar(20) primary key, pattern varchar(20))");

    if (querry1.exec()) {
        qDebug() << "[Database] stocklisttable created or already exist";
    } else {
        qDebug() << "[Database] something is wrong with stocklisttable intialization";
        QMessageBox::critical(this, tr("error: "), querry1.lastError().text());
    }


    //Load the patterns list view
    QSqlQueryModel * model = new QSqlQueryModel();
    QSqlQuery * querry2 = new QSqlQuery(mainDataBase);
    querry2->prepare("select name from stocklisttable");
    querry2->exec();
    model->setQuery(*querry2);
    ui->stockListListView->setModel(model);

    ui->editNewStockListsButton->setEnabled(false);
    ui->removeNewStockListButton->setEnabled(false);
    ui->viewStockListsButton->setEnabled(false);

    ui->additionalInfoStockListBrowser->setText("");

    // Set up patternsInUse
    QSqlQuery * querry3 = new QSqlQuery(mainDataBase);
    querry3->prepare("select * from stocklisttable");

    if (querry3->exec()) {
        while (querry3->next()) {
            QString currPattern = querry3->value(1).toString();
            patternsInUse.insert(currPattern);
        }
    } else {
        QMessageBox::critical(this, tr("error: "), tr("error setting up patternsInUse"));
    }


}



/*=============================================================================
 *
 * Patterns Page
 *
 *=============================================================================*/

void MainWindow::on_patternPageBackButton_clicked()
{
  ui->stackedWidget->setCurrentIndex(0);
}


void MainWindow::on_plusminusPatternsButton_clicked()
{
    addPatternDialog addPattern;
    addPattern.setWindowTitle("Add Pattern");
    addPattern.setModal(true);
    addPattern.exec();

    //Load the patterns list view
    QSqlQueryModel * model = new QSqlQueryModel();
    QSqlQuery * querry = new QSqlQuery(mainDataBase);
    querry->prepare("select name from patternstable");
    querry->exec();
    model->setQuery(*querry);
    ui->patternsListView->setModel(model);

}

void MainWindow::on_editNewPatternsButton_clicked()
{

    // Find the pattern name of the currently selected item in the list view
    QString patternName;
    patternName = ui->patternsListView->model()->data(currPatternListViewIndex).toString();

    addPatternDialog editPattern(this, patternName);
    editPattern.setWindowTitle("Edit Pattern");
    editPattern.setModal(true);


    editPattern.exec();

    //Load the patterns list view
    QSqlQueryModel * model = new QSqlQueryModel();
    QSqlQuery * querry = new QSqlQuery(mainDataBase);
    querry->prepare("select name from patternstable");
    querry->exec();
    model->setQuery(*querry);
    ui->patternsListView->setModel(model);



}

void MainWindow::on_removeNewPatternsButton_clicked()
{

    // Find the pattern name of the currently selected item in the list view
    QString patternName;
    patternName = ui->patternsListView->model()->data(currPatternListViewIndex).toString();

    for (QString i : patternsInUse) {
        if (i == patternName) {
            QMessageBox::critical(this, tr("error: "), "This pattern is in use in one or more of your stock list! You must delete all list that use this pattern before you can delete the pattern itself.");
            return;
        }
    }

    QSqlQuery querry;
    querry.prepare("delete from patternstable where name = '" + patternName + "';");

    if (querry.exec()) {
        qDebug() << "[Database] deleted pattern: " + patternName;
    } else {
        qDebug() << "[Databse] Did not handle deleting of a pattern";
        QMessageBox::critical(this, tr("error: "), querry.lastError().text());
    }

    //Load the patterns list view
    QSqlQueryModel * model = new QSqlQueryModel();
    QSqlQuery * querry2 = new QSqlQuery(mainDataBase);
    querry2->prepare("select name from patternstable");
    querry2->exec();
    model->setQuery(*querry2);
    ui->patternsListView->setModel(model);

    // Fix the addition info text browser
    qApp->processEvents();
    ui->additionalInfoBrowser->setText("");
}

void MainWindow::on_patternsListView_clicked(const QModelIndex &index)
{
    ui->editNewPatternsButton->setEnabled(true);
    ui->removeNewPatternsButton->setEnabled(true);
    currPatternListViewIndex = index;

    QString patternName, description, indicator2;
    patternName = ui->patternsListView->model()->data(index).toString();

    QSqlQuery querry;
    querry.prepare("select * from patternstable where name='" + patternName + "' ");

    if (querry.exec()) {
        qDebug() << "Found selected pattern to display in additional info box";

        while (querry.next()) {
            description = querry.value(1).toString();
            indicator2 = querry.value(2).toString();
        }

    } else {
        qDebug() << "Failed to deal with addtional info display information";
        description = indicator2 = "";
    }


    ui->additionalInfoBrowser->setText(patternName + ":\n" + "description: " + description);
}












/*=============================================================================
 *
 * Stock List Page
 *
 *=============================================================================*/

void MainWindow::on_stockListPageBackButton_clicked()
{

   ui->stackedWidget->setCurrentIndex(0);
}



void MainWindow::on_plusStockListButton_clicked()
{
    // Check to see if there are any patterns to even use
    QSqlQuery * querryCheck = new QSqlQuery(mainDataBase);
    querryCheck->prepare("select * from patternstable");
    querryCheck->exec();

    if (!querryCheck->next()) {
        QMessageBox::critical(this, tr("error: "), tr("No patterns to use!"));
        return;
    }

    querryCheck->finish();

    addStockListDialog addStockList;
    addStockList.setWindowTitle("Add Stock List");
    addStockList.setModal(true);
    addStockList.exec();

    //Load the stock list view
    QSqlQueryModel * model = new QSqlQueryModel();
    QSqlQuery * querry = new QSqlQuery(mainDataBase);
    querry->prepare("select name from stocklisttable");
    querry->exec();
    model->setQuery(*querry);
    ui->stockListListView->setModel(model);



}


void MainWindow::on_editNewStockListsButton_clicked()
{

    // Find the stock list name of the currently selected item in the list view
    QString stockListName;
    stockListName = ui->stockListListView->model()->data(currStockListListViewIndex).toString();

    addStockListDialog editStockList(this, stockListName);
    editStockList.setWindowTitle("Edit Stock List");
    editStockList.setModal(true);


    editStockList.exec();

    //Load the stock list view
    QSqlQueryModel * model = new QSqlQueryModel();
    QSqlQuery * querry = new QSqlQuery(mainDataBase);
    querry->prepare("select name from stocklisttable");
    querry->exec();
    model->setQuery(*querry);
    ui->stockListListView->setModel(model);
}

void MainWindow::on_removeNewStockListButton_clicked()
{
    // Ensure the user wants to delete their stock list (this CANNOT be undone)
    QMessageBox areYouSure;
    areYouSure.setText("Are you sure you want to delete this stock list?");
    areYouSure.setInformativeText("Deleting a stock list CANNOT be undone and you will lose any data saved within this stock list!");
    areYouSure.setIcon(QMessageBox::Critical);
    QPushButton * cancelButton = areYouSure.addButton(tr("Cancel"), QMessageBox::DestructiveRole);
    QPushButton * deleteButton = areYouSure.addButton(tr("Delete"), QMessageBox::RejectRole);
    areYouSure.setDefaultButton(cancelButton);
    areYouSure.exec();
    if (areYouSure.clickedButton() == cancelButton) {
        areYouSure.close();
        return;
    } else if (areYouSure.clickedButton() == deleteButton) {
        areYouSure.close();
    }

    // Find the stock list name of the currently selected item in the list view
    QString stockListName;
    QString patternListIsUsing;
    stockListName = ui->stockListListView->model()->data(currStockListListViewIndex).toString();

    QSqlQuery querry;
    querry.prepare("delete from stocklisttable where name = '" + stockListName + "';");

    if (querry.exec()) {
        qDebug() << "[Database] deleted stock list: " + stockListName;
    } else {
        qDebug() << "[Database] Did not handle deleting of a pattern";
        QMessageBox::critical(this, tr("error: "), querry.lastError().text());
    }
    querry.finish();

    // First drop all stock tables associated with this list
    //QSqlQuery * querry3 = new QSqlQuery(mainDataBase);
    QSqlQuery querry3;
    querry3.prepare("select * from '" + stockListName + "table'");

    QVector<QString> stocksInThisList;
    if (querry3.exec()) {
        while (querry3.next()) {
           //querry3->first();
           stocksInThisList.push_back(querry3.value(0).toString());
        }
    } else {
        qDebug() << "[Database] Tried to delete table: " + stockListName + "table, but it wasnt there";
    }
    querry3.finish();


    for (QString i : stocksInThisList) {
        //QSqlQuery * querryInside = new QSqlQuery(mainDataBase);
        QSqlQuery querryInside;
        querryInside.prepare("DROP TABLE IF EXISTS '" + stockListName + "$" + i + "datatable';");
        if (querryInside.exec()) {

        } else {
            QMessageBox::critical(this, tr("error: "), querryInside.lastError().text());
        }
        querryInside.finish();

    }

    // Now drop its actual table holding what stocks it has
    QSqlQuery querryForFinalTable;
    querryForFinalTable.prepare("DROP TABLE IF EXISTS '" + stockListName + "table';");
    if (querryForFinalTable.exec()) {

    } else {
        QMessageBox::critical(this, tr("error: "), querryForFinalTable.lastError().text());
    }

    querryForFinalTable.finish();

    // Finally drop it's pattern from being in use
    for (auto i : patternsInUse) {
        patternsInUse.remove(i);
    }

    //Load the stock list view
    QSqlQueryModel * model = new QSqlQueryModel();
    QSqlQuery * querry2 = new QSqlQuery(mainDataBase);
    querry2->prepare("select name from stocklisttable");
    querry2->exec();
    model->setQuery(*querry2);
    ui->stockListListView->setModel(model);

    // Fix the text browser to show nothing
    qApp->processEvents();
    ui->additionalInfoStockListBrowser->setText("");
}

void MainWindow::on_stockListListView_clicked(const QModelIndex &index)
{
    ui->editNewStockListsButton->setEnabled(true);
    ui->removeNewStockListButton->setEnabled(true);
    ui->viewStockListsButton->setEnabled(true);
    currStockListListViewIndex = index;

    QString stockListName, patternToUse;
    stockListName = ui->stockListListView->model()->data(index).toString();

    QSqlQuery querry;
    querry.prepare("select * from stocklisttable where name='" + stockListName + "' ");

    if (querry.exec()) {
        qDebug() << "Found selected pattern to display in additional info box";

        while (querry.next()) {
            patternToUse = querry.value(1).toString();
        }

    } else {
        qDebug() << "Failed to deal with addtional info display information";
        patternToUse = "";
    }

    ui->additionalInfoStockListBrowser->setText("Uses pattern: \n" + patternToUse);
    currentStockListsPattern = patternToUse;
}

void MainWindow::on_viewStockListsButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(3);

    // Save the currently clicked stockListName
    QString stockListName;
    stockListName = ui->stockListListView->model()->data(currStockListListViewIndex).toString();

    QSqlQuery querry1;
    querry1.prepare("create table if not exists '" + stockListName + "table' (symbol varchar(20) primary key, pattern varchar(20), patternTriggered int, patternSuccess int, avgPriceInc int, avgDayPeak int, streakAmount int)");

    if (querry1.exec()) {
        qDebug() << "[Database] specific stock table for: " + stockListName + " created or already exist";
    } else {
        qDebug() << "[Database] something is wrong with specific stock table intialization";
        QMessageBox::critical(this, tr("error: "), querry1.lastError().text());
    }
    querry1.finish();

    //Load the stocks list view
    QSqlQueryModel * model = new QSqlQueryModel();
    QSqlQuery * querry2 = new QSqlQuery(mainDataBase);
    querry2->prepare("select symbol from '" + stockListName + "table'");
    querry2->exec();
    model->setQuery(*querry2);
    ui->stocksListView->setModel(model);

    ui->stackedWidget_2->setCurrentIndex(0);
    ui->otherAddNewStockButton->setVisible(false);
    ui->addDataWithinStockButton->setEnabled(false);
    ui->addDataWithinStockButton->setText("Add/Edit");
    ui->removeDataWithinStockButton->setEnabled(false);

    ui->priceAtTriggerLineEdit->setVisible(false);
    ui->priceAtPeakLineEdit->setVisible(false);
    ui->label_12->setVisible(false);
    ui->label_15->setVisible(false);

    ui->dateOfPeakPriceLineEdit->setVisible(false);
    ui->label_14->setVisible(false);
    ui->patternSuccessRadioButton->setChecked(false);
    ui->patternSuccessRadioButton->setVisible(false);
    ui->specialDataTableAddDataButton->setVisible(false);
    //querry2->finish();
    sqlClose();
    sqlOpen();

}






/*=============================================================================
 *
 * Stocks Page
 *
 *=============================================================================*/


void MainWindow::on_stocksBackButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
}

void MainWindow::on_otherAddNewStockButton_clicked()
{
    ui->stackedWidget_2->setCurrentIndex(0);
    QString stockListName = ui->stockListListView->model()->data(currStockListListViewIndex).toString();

    // Create new datatable for this new stock
    addStocksDialog newStock(this, stockListName);
    newStock.setWindowTitle("Add a New Stock");
    newStock.setModal(true);
    newStock.exec();

    //Load the new stock's datatable
    QString stockName;
    stockName = newStock.stockName;
    QSqlQueryModel * model = new QSqlQueryModel();
    QSqlQuery * querry1 = new QSqlQuery(mainDataBase);
    querry1->prepare("select * from '" + stockListName + "$" + stockName + "datatable'");
    querry1->exec();
    model->setQuery(*querry1);
    ui->newStockTableView->setModel(model);
    ui->newStockTableView->setColumnHidden(1, true);

    QSqlQuery querry;
    querry.prepare("insert into '" + stockListName + "table' (symbol, pattern, patternTriggered, patternSuccess, avgPriceInc, avgDayPeak, streakAmount) values ('" + stockName + "', '', '', '', '', '', '')");

    if (querry.exec()) {
        qDebug() << "[Database] Inserted new stock into the stock List: " + stockName;
    } else {
        qDebug() << "[Database] Failed to insert new stock into the stock list " + stockName;
        QMessageBox::critical(this, tr("error: "), tr("This stock list already exists!"));
    }

    //Load this specific stocks list view
    QSqlQueryModel * model2 = new QSqlQueryModel();
    QSqlQuery * querry2 = new QSqlQuery(mainDataBase);
    querry2->prepare("select symbol from '" + stockListName + "table'");
    querry2->exec();
    model2->setQuery(*querry2);
    ui->stocksListView->setModel(model2);
    ui->newStockTableView->setColumnHidden(1, true);

    sqlClose();
    sqlOpen();

}

void MainWindow::on_controlPageNewStockButton_clicked()
{
    ui->stackedWidget_2->setCurrentIndex(0);
    QString stockListName = ui->stockListListView->model()->data(currStockListListViewIndex).toString();

    // Create new datatable for this new stock
    addStocksDialog newStock(this, stockListName);
    newStock.setWindowTitle("Add a New Stock");
    newStock.setModal(true);
    newStock.exec();

    //Load the new stock's datatable
    QString stockName;
    stockName = newStock.stockName;
    QSqlQueryModel * model = new QSqlQueryModel();
    QSqlQuery * querry1 = new QSqlQuery(mainDataBase);
    querry1->prepare("select * from '" + stockListName + "$" + stockName + "datatable'");
    querry1->exec();
    model->setQuery(*querry1);
    ui->newStockTableView->setModel(model);
    ui->newStockTableView->setColumnHidden(1, true);

    QSqlQuery querry;
    querry.prepare("insert into '" + stockListName + "table' (symbol, pattern, patternTriggered, patternSuccess, avgPriceInc, avgDayPeak, streakAmount) values ('" + stockName + "', '', '', '', '', '', '')");

    if (querry.exec()) {
        qDebug() << "[Database] Inserted new stock into the stock List: " + stockName;
    } else {
        qDebug() << "[Database] Failed to insert new stock into the stock list " + stockName;
        QMessageBox::critical(this, tr("error: "), tr("This stock list already exists!"));
    }

    //Load this specific stocks list view
    QSqlQueryModel * model2 = new QSqlQueryModel();
    QSqlQuery * querry2 = new QSqlQuery(mainDataBase);
    querry2->prepare("select symbol from '" + stockListName + "table'");
    querry2->exec();
    model2->setQuery(*querry2);
    ui->stocksListView->setModel(model2);
    ui->newStockTableView->setColumnHidden(1, true);

    sqlClose();
    sqlOpen();

    ui->otherAddNewStockButton->setVisible(true);

}


void MainWindow::on_stocksListView_clicked(const QModelIndex &index)
{

    ui->dateOfPeakPriceLineEdit->setVisible(false);
    ui->label_14->setVisible(false);
    ui->label_12->setVisible(false);
    ui->label_15->setVisible(false);
    indexToSave = index;
    incomingDate = "";
    // First clear all values in the line edits
    ui->dateStockLineEdit->setText("");
    ui->patternTriggeredStockLineEdit->setText("");
    ui->patternSuccessStockLineEdit->setText("");

    ui->priceAtTriggerLineEdit->setText("");
    ui->priceAtPeakLineEdit->setText("");
    ui->dateOfPeakPriceLineEdit->setText("");

    //Load the clicked stock's datatable
    QString stockListName = ui->stockListListView->model()->data(currStockListListViewIndex).toString();
    currStockListName = stockListName;
    QString stockName;
    stockName = ui->stocksListView->model()->data(index).toString();

    QSqlQueryModel * model = new QSqlQueryModel();
    QSqlQuery * querry1 = new QSqlQuery(mainDataBase);
    querry1->prepare("select * from '" + stockListName + "$" + stockName + "datatable'");
    querry1->exec();
    model->setQuery(*querry1);
    ui->newStockTableView->setModel(model);
    ui->newStockTableView->setColumnHidden(1, true);

    // TODO: Stats page updated too
    ui->stackedWidget_2->setCurrentIndex(2);

    ui->stockNameLabel->setText(stockName);

    // Reset text browser
    ui->statsTextBrowser->setText("");

    currStockDataTable = stockListName + "$" + stockName + "datatable";
    currStockName = stockName;

    ui->addDataWithinStockButton->setText("Add");
    ui->specialDataTableAddDataButton->setVisible(false);

    load_stockStats();

    sqlClose();
    sqlOpen();

}




void MainWindow::on_newStockTableView_clicked(const QModelIndex &index)
{
    QString val = ui->newStockTableView->model()->data(index).toString();

    QSqlQuery querry1;
    querry1.prepare("select * from '" + currStockDataTable + "' where date = '" + val + "' or pattern = '" + val + "' or patternTriggered = '" + val + "' or patternSuccess = '" + val + "' or priceAtTrigger = '" + val + "' or priceAtPeak = '" + val + "' or dateOfPricePeak = '" + val + "'");
    if (querry1.exec()) {

        while (querry1.next()) {
            ui->dateStockLineEdit->setText(querry1.value(0).toString());
            incomingDate = querry1.value(0).toString();
            if (querry1.value(2).toString() == '1') {
                ui->patternTriggeredRadioButton->setChecked(true);
                ui->patternSuccessRadioButton->setVisible(true);
                on_patternTriggeredRadioButton_clicked();
            } else {
                ui->patternTriggeredRadioButton->setChecked(false);
                ui->patternSuccessRadioButton->setVisible(false);
                ui->patternSuccessRadioButton->setChecked(false);
                on_patternTriggeredRadioButton_clicked();
            }
            ui->patternTriggeredStockLineEdit->setText(querry1.value(2).toString());
            if (querry1.value(3).toString() == '1') {
                ui->patternSuccessRadioButton->setChecked(true);

                ui->priceAtTriggerLineEdit->setVisible(true);
                ui->priceAtPeakLineEdit->setVisible(true);
                ui->dateOfPeakPriceLineEdit->setVisible(true);
                on_patternSuccessRadioButton_clicked();
            } else {
                ui->patternSuccessRadioButton->setChecked(false);
                on_patternSuccessRadioButton_clicked();
            }
            ui->patternSuccessStockLineEdit->setText(querry1.value(3).toString());

            ui->priceAtTriggerLineEdit->setText(querry1.value(4).toString());
            ui->priceAtPeakLineEdit->setText(querry1.value(5).toString());
            ui->dateOfPeakPriceLineEdit->setText(querry1.value(6).toString());

        }
    } else {
        QMessageBox::critical(this, tr("error: "), querry1.lastError().text());
    }

    ui->addDataWithinStockButton->setText("Update");
    ui->specialDataTableAddDataButton->setVisible(true);

    sqlClose();
    sqlOpen();

    return;


}

void MainWindow::on_addDataWithinStockButton_clicked()
{
    bool edit = false;
    QString date, pattern, patternTriggered, patternSuccess, priceIncrease, priceAtTrigger, priceAtPeak, dateOfPeakPrice;
    date = ui->dateStockLineEdit->text();
    pattern = currentStockListsPattern;
    patternTriggered = ui->patternTriggeredStockLineEdit->text();
    if (ui->patternTriggeredRadioButton->isChecked()) {
        patternTriggered = '1';
    } else {
        patternTriggered = '0';
        patternSuccess = '0';
        priceIncrease = "";
        priceAtTrigger = "";
        priceAtPeak = "";
        dateOfPeakPrice = "";

    }
    qApp->processEvents();
    patternSuccess = ui->patternSuccessStockLineEdit->text();
    if (ui->patternSuccessRadioButton->isChecked()) {
        patternSuccess = '1';

        priceAtTrigger = ui->priceAtTriggerLineEdit->text();
        priceAtPeak = ui->priceAtPeakLineEdit->text();
        dateOfPeakPrice = ui->dateOfPeakPriceLineEdit->text();
    } else {
        patternSuccess = '0';
        priceIncrease = "";
        priceAtTrigger = "";
        priceAtPeak = "";
        dateOfPeakPrice = "";
    }
    // if we aren't editing
    if (incomingDate.compare("") == 0) {

        QSqlQuery querry;
        querry.prepare("insert into '" + currStockDataTable + "' (date,pattern,patternTriggered,patternSuccess,priceAtTrigger,priceAtPeak,dateOfPricePeak) values ('" + date + "', '" + pattern + "', '" + patternTriggered + "', '" + patternSuccess + "', '" + priceAtTrigger + "', '" + priceAtPeak + "' , '" + dateOfPeakPrice + "')");

        if (querry.exec()) {
            qDebug() << "[Database] Inserted new data into datatable: " + currStockDataTable;
        } else {
            qDebug() << "[Database] Failed to insert new data into datatable: " + currStockDataTable;
            QMessageBox::critical(this, tr("error: "), tr("The data for this date already exists!"));
        }
    } else {
        edit = true;
        QSqlQuery querry;
        querry.prepare("update '" + currStockDataTable + "' set date = '" + date + "', pattern = '" + pattern + "', patternTriggered = '" + patternTriggered + "', patternSuccess = '" + patternSuccess + "', priceAtTrigger = '" + priceAtTrigger + "', priceAtPeak = '" + priceAtPeak + "', dateOfPricePeak = '" + dateOfPeakPrice + "' where date = '" + incomingDate + "';");

        if (querry.exec()) {
            qDebug() << "[Database] Edited the data in: " + currStockDataTable + " where date had equaled " + incomingDate;

        } else {
            qDebug() << "[Database] Failed to edit the data in: " + currStockDataTable;
            QMessageBox::critical(this, tr("error: "), tr("There is already an entry with this date!"));
        }
    }

    ui->newStockTableView->clearSelection();
    ui->newStockTableView->update();

    qApp->processEvents();

    // Reload the datatable for both cases
    QSqlQueryModel * model = new QSqlQueryModel();
    QSqlQuery * querry1 = new QSqlQuery(mainDataBase);
    querry1->prepare("select * from '" + currStockDataTable + "'");
    querry1->exec();
    model->setQuery(*querry1);
    ui->newStockTableView->setModel(model);
    ui->newStockTableView->setColumnHidden(1, true);

    // once updated clear the line edits and reset the add button and incomingDate
    ui->dateStockLineEdit->setText("");
    ui->patternTriggeredStockLineEdit->setText("");
    ui->patternSuccessStockLineEdit->setText("");

    qApp->processEvents();

    ui->priceAtTriggerLineEdit->setText("");
    ui->priceAtPeakLineEdit->setText("");
    ui->dateOfPeakPriceLineEdit->setText("");
    ui->addDataWithinStockButton->setText("Add");
    ui->specialDataTableAddDataButton->setVisible(false);
    incomingDate = "";
    ui->removeDataWithinStockButton->setEnabled(false);
    ui->addDataWithinStockButton->setEnabled(false);

    if (edit) {
       QMessageBox::information(this, tr("Edited"), tr("Date Edited"));
    } else {
       QMessageBox::information(this, tr("Added"), tr("New entry added"));
    }

    sqlClose();
    sqlOpen();

    ui->patternSuccessRadioButton->setChecked(false);
    on_patternSuccessRadioButton_clicked();

}

void MainWindow::on_dateStockLineEdit_textChanged(const QString &arg1)
{
    ui->addDataWithinStockButton->setEnabled(true);
    ui->removeDataWithinStockButton->setEnabled(true);
    if (incomingDate == "") {
        ui->addDataWithinStockButton->setText("Add");
    } else {
        ui->addDataWithinStockButton->setText("Update");
    }
    ui->specialDataTableAddDataButton->setVisible(false);

    QSqlQuery querry;
    querry.prepare("select date from '" + currStockListName + "$" + currStockName + "datatable'");
    if (querry.exec()) {
        while (querry.next()) {
            if (arg1 == querry.value(0).toString()) {
                ui->addDataWithinStockButton->setText("Update");
                ui->specialDataTableAddDataButton->setVisible(true);
            }
        }
    } else {
        QMessageBox::critical(this, tr("error: "), querry.lastError().text());
    }
}

void MainWindow::on_removeDataWithinStockButton_clicked()
{
    QString date;
    date = ui->dateStockLineEdit->text();

    QSqlQuery querry;
    querry.prepare("delete from '" + currStockDataTable + "' where date = '" + date + "';");

    if (querry.exec()) {
        qDebug() << "[Database] Deleted data at date " + date + "in datatable: " + currStockDataTable;
    } else {
        qDebug() << "[Database] Failed to delete data at date " + date + "in datatable: " + currStockDataTable;
        QMessageBox::critical(this, tr("error: "), tr("This date doesn't exist!"));
    }

    // Reload the datatable for both cases
    QSqlQueryModel * model = new QSqlQueryModel();
    QSqlQuery * querry1 = new QSqlQuery(mainDataBase);
    querry1->prepare("select * from '" + currStockDataTable + "'");
    querry1->exec();
    model->setQuery(*querry1);
    ui->newStockTableView->setModel(model);
    ui->newStockTableView->setColumnHidden(1, true);

    ui->dateStockLineEdit->setText("");
    ui->patternTriggeredStockLineEdit->setText("");
    ui->patternSuccessStockLineEdit->setText("");

    ui->priceAtTriggerLineEdit->setText("");
    ui->priceAtPeakLineEdit->setText("");
    ui->dateOfPeakPriceLineEdit->setText("");
    ui->addDataWithinStockButton->setText("Add");
    ui->specialDataTableAddDataButton->setVisible(false);
    incomingDate = "";
    ui->removeDataWithinStockButton->setEnabled(false);
    ui->addDataWithinStockButton->setEnabled(false);

    ui->patternSuccessRadioButton->setChecked(false);
    on_patternSuccessRadioButton_clicked();

    QMessageBox::information(this, tr("Edited"), tr("Date Edited"));

    sqlClose();
    sqlOpen();
}


void MainWindow::on_editStatsbutton_clicked()
{
    ui->stackedWidget_2->setCurrentIndex(1);
}

// Return -1 if date values are wrong sytax
qint64 determineDateValue(QString startDate, QString endDate) {

    int startNumYears = 0;
    int startNumMonths = 0;
    int startNumDays = 0;
    int endNumYears = 0;
    int endNumMonths = 0;
    int endNumDays = 0;

    if (startDate.size() < 7 || endDate.size() < 7) {
        qDebug() << "returning at 50"; return -1;
    }

    if (startDate.at(1) == '/') {


        switch (startDate.at(0).unicode()) {
            case 57:
                startNumMonths = 9;
                break;
            case 56:
                startNumMonths = 8;
                break;
            case 55: startNumMonths = 7;
            break;
            case 54: startNumMonths = 6;
            break;
            case 53: startNumMonths = 5;
            break;
            case 52: startNumMonths = 4;
            break;
            case 51: startNumMonths = 3;
            break;
            case 50: startNumMonths = 2;
            break;
            case 49: startNumMonths = 1;
            break;
            default: qDebug() << "returning at 51"; return -1;
        }

        switch (startDate.at(2).unicode()) {
            case 51: startNumDays += 30; break;
            case 50: startNumDays += 20; break;
            case 49: startNumDays += 10; break;
            case 48: startNumDays += 00; break;
            default: qDebug() << "returning at 52"; return -1;
        }

        switch (startDate.at(3).unicode()) {
            case 57: startNumDays += 9; break;
            case 56: startNumDays += 8; break;
            case 55: startNumDays += 7; break;
            case 54: startNumDays += 6; break;
            case 53: startNumDays += 5; break;
            case 52: startNumDays += 4; break;
            case 51: startNumDays += 3; break;
            case 50: startNumDays += 2; break;
            case 49: startNumDays += 1; break;
            case 48: startNumDays += 0; break;
            default: qDebug() << "returning at 53"; return -1;
        }

        if (startDate.at(4) != '/') {
            qDebug() << "returning at 54"; return -1;
        }

        switch (startDate.at(5).unicode()) {
            case 57: startNumYears += 90; break;
            case 56: startNumYears += 80; break;
            case 55: startNumYears += 70; break;
            case 54: startNumYears += 60; break;
            case 53: startNumYears += 50; break;
            case 52: startNumYears += 40; break;
            case 51: startNumYears += 30; break;
            case 50: startNumYears += 20; break;
            case 49: startNumYears += 10;break;
            case 48: startNumYears += 00; break;
            default: qDebug() << "returning at 55"; return -1;
        }

        switch (startDate.at(6).unicode()) {
            case 57: startNumYears += 9; break;
            case 56: startNumYears += 8; break;
            case 55: startNumYears += 7; break;
            case 54: startNumYears += 6; break;
            case 53: startNumYears += 5; break;
            case 52: startNumYears += 4; break;
            case 51: startNumYears += 3; break;
            case 50: startNumYears += 2; break;
            case 49: startNumYears += 1; break;
            case 48: startNumYears += 0; break;
            default: qDebug() << "returning at 56"; return -1;
        }




    } else if (startDate.at(0) == '0') {
        if (startDate.size() < 8) {
            qDebug() << "returning at 57"; return -1;
        }

        switch (startDate.at(1).unicode()) {
            case 57:
                startNumMonths = 9;
                break;
            case 56:
                startNumMonths = 8;
                break;
            case 55: startNumMonths = 7;
            break;
            case 54: startNumMonths = 6;
            break;
            case 53: startNumMonths = 5;
            break;
            case 52: startNumMonths = 4;
            break;
            case 51: startNumMonths = 3;
            break;
            case 50: startNumMonths = 2;
            break;
            case 49: startNumMonths = 1;
            break;
            default: qDebug() << "returning at 58"; return -1;
        }

        if (startDate.at(2) != '/') {
            qDebug() << "returning at 59"; return -1;
        }

        switch (startDate.at(3).unicode()) {
            case 51: startNumDays += 30; break;
            case 50: startNumDays += 20; break;
            case 49: startNumDays += 10; break;
            case 48: startNumDays += 00; break;
            default: qDebug() << "returning at 60"; return -1;
        }

        switch (startDate.at(4).unicode()) {
            case 57: startNumDays += 9; break;
            case 56: startNumDays += 8; break;
            case 55: startNumDays += 7; break;
            case 54: startNumDays += 6; break;
            case 53: startNumDays += 5; break;
            case 52: startNumDays += 4; break;
            case 51: startNumDays += 3; break;
            case 50: startNumDays += 2; break;
            case 49: startNumDays += 1; break;
            case 48: startNumDays += 0; break;
            default: qDebug() << "returning at 61"; return -1;
        }

        if (startDate.at(5) != '/') {
            qDebug() << "returning at 62"; return -1;
        }

        switch (startDate.at(6).unicode()) {
            case 57: startNumYears += 90; break;
            case 56: startNumYears += 80; break;
            case 55: startNumYears += 70; break;
            case 54: startNumYears += 60; break;
            case 53: startNumYears += 50; break;
            case 52: startNumYears += 40; break;
            case 51: startNumYears += 30; break;
            case 50: startNumYears += 20; break;
            case 49: startNumYears += 10;break;
            case 48: startNumYears += 00; break;
            default: qDebug() << "returning at 63"; return -1;
        }

        switch (startDate.at(7).unicode()) {
            case 57: startNumYears += 9; break;
            case 56: startNumYears += 8; break;
            case 55: startNumYears += 7; break;
            case 54: startNumYears += 6; break;
            case 53: startNumYears += 5; break;
            case 52: startNumYears += 4; break;
            case 51: startNumYears += 3; break;
            case 50: startNumYears += 2; break;
            case 49: startNumYears += 1; break;
            case 48: startNumYears += 0; break;
            default: qDebug() << "returning at 64"; return -1;
        }

    } else if ((startDate.at(1) == '0' || startDate.at(1) == '1' || startDate.at(1) == '2') && startDate.at(0) == '1') {

        if (startDate.size() < 8) {
            qDebug() << "returning at 65"; return -1;
        }


        switch (startDate.at(1).unicode()) {
            case 50: startNumMonths = 12; break;
            case 49: startNumMonths = 11; break;
            case 48: startNumMonths = 10; break;
            qDebug() << "returning at 66"; default: return -1;
        }

        if (startDate.at(2) != '/') {
            qDebug() << "returning at 67"; return -1;
        }

        switch (startDate.at(3).unicode()) {
            case 51: startNumDays += 30; break;
            case 50: startNumDays += 20; break;
            case 49: startNumDays += 10; break;
            case 48: startNumDays += 0; break;
            default: qDebug() << "returning at 68"; return -1;
        }

        switch (startDate.at(4).unicode()) {
            case 57: startNumDays += 9; break;
            case 56: startNumDays += 8; break;
            case 55: startNumDays += 7; break;
            case 54: startNumDays += 6; break;
            case 53: startNumDays += 5; break;
            case 52: startNumDays += 4;break;
            case 51: startNumDays += 3;break;
            case 50: startNumDays += 2;break;
            case 49: startNumDays += 1; break;
            case 48: startNumDays += 0; break;
            default: qDebug() << "returning at 69"; return -1;
        }

        if (startDate.at(5) != '/') {
            qDebug() << "returning at 70"; return -1;
        }

        switch (startDate.at(6).unicode()) {
            case 57: startNumYears += 90; break;
            case 56: startNumYears += 80; break;
            case 55: startNumYears += 70; break;
            case 54: startNumYears += 60; break;
            case 53: startNumYears += 50; break;
            case 52: startNumYears += 40; break;
            case 51: startNumYears += 30; break;
            case 50: startNumYears += 20; break;
            case 49: startNumYears += 10; break;
            case 48: startNumYears += 00; break;
            default: qDebug() << "returning at 71"; return -1;
        }

        switch (startDate.at(7).unicode()) {
            case 57: startNumYears += 9; break;
            case 56: startNumYears += 8; break;
            case 55: startNumYears += 7; break;
            case 54: startNumYears += 6; break;
            case 53: startNumYears += 5; break;
            case 52: startNumYears += 4; break;
            case 51: startNumYears += 3; break;
            case 50: startNumYears += 2; break;
            case 49: startNumYears += 1; break;
            case 48: startNumYears += 0; break;
            default: qDebug() << "returning at 72"; return -1;
        }

    } else {
        qDebug() << "returning at 73"; return -1;
    }


    //=====================================
    if (endDate.at(1) == '/') {


        switch (endDate.at(0).unicode()) {
            case 57:
                endNumMonths = 9;
                break;
            case 56:
                endNumMonths = 8;
                break;
            case 55: endNumMonths = 7;
            break;
            case 54: endNumMonths = 6;
            break;
            case 53: endNumMonths = 5;
            break;
            case 52: endNumMonths = 4;
            break;
            case 51: endNumMonths = 3;
            break;
            case 50: endNumMonths = 2;
            break;
            case 49: endNumMonths = 1;
            break;
            default: qDebug() << "returning at 1"; return -1;
        }

        switch (endDate.at(2).unicode()) {
            case 51: endNumDays += 30; break;
            case 50: endNumDays += 20; break;
            case 49: endNumDays += 10; break;
            case 48: endNumDays += 00; break;
            default: qDebug() << "returning at 2"; return -1;
        }

        switch (endDate.at(3).unicode()) {
            case 57: endNumDays += 9; break;
            case 56: endNumDays += 8; break;
            case 55: endNumDays += 7; break;
            case 54: endNumDays += 6; break;
            case 53: endNumDays += 5; break;
            case 52: endNumDays += 4; break;
            case 51: endNumDays += 3; break;
            case 50: endNumDays += 2; break;
            case 49: endNumDays += 1; break;
            case 48: endNumDays += 0; break;
            default: qDebug() << "returning at 3"; return -1;
        }

        if (endDate.at(4) != '/') {
            qDebug() << "returning at 4"; return -1;
        }

        switch (endDate.at(5).unicode()) {
            case 57: endNumYears += 90; break;
            case 56: endNumYears += 80; break;
            case 55: endNumYears += 70; break;
            case 54: endNumYears += 60; break;
            case 53: endNumYears += 50; break;
            case 52: endNumYears += 40; break;
            case 51: endNumYears += 30; break;
            case 50: endNumYears += 20; break;
            case 49: endNumYears += 10;break;
            case 48: endNumYears += 00; break;
            default: qDebug() << "returning at 5"; return -1;
        }

        switch (endDate.at(6).unicode()) {
            case 57: endNumYears += 9; break;
            case 56: endNumYears += 8; break;
            case 55: endNumYears += 7; break;
            case 54: endNumYears += 6; break;
            case 53: endNumYears += 5; break;
            case 52: endNumYears += 4; break;
            case 51: endNumYears += 3; break;
            case 50: endNumYears += 2; break;
            case 49: endNumYears += 1; break;
            case 48: endNumYears += 0; break;
            default: qDebug() << "returning at 6"; return -1;
        }




    } else if (endDate.at(0) == '0') {

        if (endDate.size() < 8) {
            qDebug() << "returning at 7"; return -1;
        }

        switch (endDate.at(1).unicode()) {
            case 57:
                endNumMonths = 9;
                break;
            case 56:
                endNumMonths = 8;
                break;
            case 55: endNumMonths = 7;
            break;
            case 54: endNumMonths = 6;
            break;
            case 53: endNumMonths = 5;
            break;
            case 52: endNumMonths = 4;
            break;
            case 51: endNumMonths = 3;
            break;
            case 50: endNumMonths = 2;
            break;
            case 49: endNumMonths = 1;
            break;
            default: qDebug() << "returning at 8"; return -1;
        }

        if (endDate.at(2) != '/') {
            qDebug() << "returning at 9"; return -1;
        }


        switch (endDate.at(3).unicode()) {
            case 51: endNumDays += 30; break;
            case 50: endNumDays += 20; break;
            case 49: endNumDays += 10; break;
            case 48: endNumDays += 00; break;
            default: qDebug() << "returning at 10"; return -1;
        }

        switch (endDate.at(4).unicode()) {
            case 57: endNumDays += 9; break;
            case 56: endNumDays += 8; break;
            case 55: endNumDays += 7; break;
            case 54: endNumDays += 6; break;
            case 53: endNumDays += 5; break;
            case 52: endNumDays += 4; break;
            case 51: endNumDays += 3; break;
            case 50: endNumDays += 2; break;
            case 49: endNumDays += 1; break;
            case 48: endNumDays += 0; break;
            default: qDebug() << "returning at 11"; return -1;
        }

        if (endDate.at(5) != '/') {
            qDebug() << "returning at 12"; return -1;
        }

        switch (endDate.at(6).unicode()) {
            case 57: endNumYears += 90; break;
            case 56: endNumYears += 80; break;
            case 55: endNumYears += 70; break;
            case 54: endNumYears += 60; break;
            case 53: endNumYears += 50; break;
            case 52: endNumYears += 40; break;
            case 51: endNumYears += 30; break;
            case 50: endNumYears += 20; break;
            case 49: endNumYears += 10;break;
            case 48: endNumYears += 00; break;
            default: qDebug() << "returning at 13"; return -1;
        }

        switch (endDate.at(7).unicode()) {
            case 57: endNumYears += 9; break;
            case 56: endNumYears += 8; break;
            case 55: endNumYears += 7; break;
            case 54: endNumYears += 6; break;
            case 53: endNumYears += 5; break;
            case 52: endNumYears += 4; break;
            case 51: endNumYears += 3; break;
            case 50: endNumYears += 2; break;
            case 49: endNumYears += 1; break;
            case 48: endNumYears += 0; break;
            default: qDebug() << "returning at 14"; return -1;
        }

    } else if ((endDate.at(1) == '0' || endDate.at(1) == '1' || endDate.at(1) == '2') && endDate.at(0) == '1') {

        if (endDate.size() < 8) {
            qDebug() << "returning at 15"; return -1;
        }


        switch (endDate.at(1).unicode()) {
            case 50: endNumMonths = 12; break;
            case 49: endNumMonths = 11; break;
            case 48: endNumMonths = 10; break;
            default: qDebug() << "returning at 16"; return -1;
        }

        if (endDate.at(2) != '/') {
            qDebug() << "returning at 17"; return -1;
        }

        switch (endDate.at(3).unicode()) {
            case 51: endNumDays += 30; break;
            case 50: endNumDays += 20; break;
            case 49: endNumDays += 10; break;
            case 48: endNumDays += 0; break;
            default: qDebug() << "returning at 18"; return -1;
        }

        switch (endDate.at(4).unicode()) {
            case 57: endNumDays += 9; break;
            case 56: endNumDays += 8; break;
            case 55: endNumDays += 7; break;
            case 54: endNumDays += 6; break;
            case 53: endNumDays += 5; break;
            case 52: endNumDays += 4;break;
            case 51: endNumDays += 3;break;
            case 50: endNumDays += 2;break;
            case 49: endNumDays += 1; break;
            case 48: endNumDays += 0; break;
            default: qDebug() << "returning at 19"; return -1;
        }

        if (endDate.at(5) != '/') {
            qDebug() << "returning at 20"; return -1;
        }

        switch (endDate.at(6).unicode()) {
            case 57: endNumYears += 90; break;
            case 56: endNumYears += 80; break;
            case 55: endNumYears += 70; break;
            case 54: endNumYears += 60; break;
            case 53: endNumYears += 50; break;
            case 52: endNumYears += 40; break;
            case 51: endNumYears += 30; break;
            case 50: endNumYears += 20; break;
            case 49: endNumYears += 10; break;
            case 48: endNumYears += 00; break;
            default: qDebug() << "returning at 21"; return -1;
        }

        switch (endDate.at(7).unicode()) {
            case 57: endNumYears += 9; break;
            case 56: endNumYears += 8; break;
            case 55: endNumYears += 7; break;
            case 54: endNumYears += 6; break;
            case 53: endNumYears += 5; break;
            case 52: endNumYears += 4; break;
            case 51: endNumYears += 3; break;
            case 50: endNumYears += 2; break;
            case 49: endNumYears += 1; break;
            case 48: endNumYears += 0; break;
            default: qDebug() << "returning at 22"; return -1;
        }

    } else {
        qDebug() << "returning at 23"; return -1;
    }

    endNumYears += 2000;
    startNumYears += 2000;
    QDate start = QDate(startNumYears, startNumMonths, startNumDays);
    QDate end = QDate(endNumYears, endNumMonths, endNumDays);
    return start.daysTo(end);

}

void MainWindow::load_stockStats() {
    int patternTriggeredTotal = 1;
    int patternSuccessTotal = 0;
    int avgPriceInc = 0;
    float priceAtTrigger = 0;
    float priceAtPeak = 0;
    QString dateOfPeak = "";
    QString currDate = "";
    int totalDaysPeakHit = 0;
    int numberOfPeakEntries = 0;
    float avgDaysPeakhit = 0;
    float totalPercentsTogether = 0;
    float numberOfPercentEntries = 0;

    QString patternDescription, indicator2;
    QSqlQuery * querry5 = new QSqlQuery(mainDataBase);
    querry5->prepare("select * from patternstable where \"name\" = '" + currentStockListsPattern + "'");
    if (querry5->exec()) {
        querry5->first();
        patternDescription = querry5->value(1).toString();
        // Indicator 2 is on hold for the future
        indicator2 = querry5->value(2).toString();
    } else {
        QMessageBox::critical(this, tr("error: "), tr("error in 5"));
    }

    ui->statsTextBrowser->append("The list uses pattern \"" + currentStockListsPattern + "\" defined as, \"" + patternDescription + "\"\n");

    QSqlQuery * querry1 = new QSqlQuery(mainDataBase);
    querry1->prepare("select sum(patternTriggered) from '" + currStockDataTable + "'");
    if (querry1->exec()) {
        querry1->first();
        patternTriggeredTotal = querry1->value(0).toInt();
    } else {
        QMessageBox::critical(this, tr("error: "), tr("error in 1"));
    }

    QSqlQuery * querry2 = new QSqlQuery(mainDataBase);
    querry2->prepare("select sum(patternSuccess) from '" + currStockDataTable + "'");
    if (querry2->exec()) {
        querry2->first();
        patternSuccessTotal = querry2->value(0).toInt();
    } else {
        QMessageBox::critical(this, tr("error: "), tr("error in 2"));
    }

    ui->statsTextBrowser->append("Number of data entries: " + QString::number(ui->newStockTableView->model()->rowCount()) + "\n");

    if (patternTriggeredTotal == 0) {
        ui->statsTextBrowser->append("Success rate: Invalid input");
    } else {
        float successOverTriggered = ((float)patternSuccessTotal / patternTriggeredTotal) * 100;
        ui->statsTextBrowser->append("<html><b>" + QString::number(successOverTriggered) + "%</b</html> successful for \"" + currentStockListsPattern + "\" pattern\n");
    }


    /*QSqlQuery * querry3 = new QSqlQuery(mainDataBase);
    querry3->prepare("select avg(priceAtTrigger) from '" + currStockDataTable + "' where patternSuccess = 1");
    if (querry3->exec()) {
        querry3->first();
        avgPriceInc = querry3->value(0).toInt();
    } else {
        QMessageBox::critical(this, tr("error: "), tr("error in 3"));
    }

    ui->statsTextBrowser->append("Average Price Increase: " + QString::number(avgPriceInc) + "$\n");*/

    QSqlQuery * querry4 = new QSqlQuery(mainDataBase);
    querry4->prepare("select * from '" + currStockDataTable + "' where patternSuccess = 1");
    if (querry4->exec()) {
        while (querry4->next()) {
            dateOfPeak = querry4->value(6).toString();
            currDate = querry4->value(0).toString();

            qDebug() << "[Important] Going in with peak = " + dateOfPeak + " and curr = " + currDate;

            qint64 daysBetween = determineDateValue(currDate, dateOfPeak);

            qDebug() << "[Important] Returned with daysbetween = ";
            qDebug() << daysBetween;

            if (daysBetween == -1) {
                totalDaysPeakHit += 0;
            } else {
                totalDaysPeakHit += daysBetween;
                numberOfPeakEntries += 1;
            }

            priceAtTrigger = querry4->value(4).toFloat();
            priceAtPeak = querry4->value(5).toFloat();

            if (priceAtTrigger != 0 && priceAtPeak != 0) {
                float priceDiff = priceAtPeak - priceAtTrigger;
                float percentIncrease = priceDiff / priceAtTrigger;
                totalPercentsTogether += percentIncrease;
                numberOfPercentEntries += 1;
            }
        }
    } else {
        QMessageBox::critical(this, tr("error: "), tr("error in 4"));
    }

    if (numberOfPeakEntries != 0) {
        avgDaysPeakhit = (float)totalDaysPeakHit / numberOfPeakEntries;
        ui->statsTextBrowser->append("On average the stock hits peak price <html><b>" + QString::number(avgDaysPeakhit) + " days</b</html> after the start of the event when the trigger was successful \n");

    } else {
        ui->statsTextBrowser->append("There is no date data or Syntax for date entries are wrong, use MM/DD/YY");
    }

    if (numberOfPercentEntries != 0) {
        float avgPercentIncrease = (totalPercentsTogether / numberOfPercentEntries)*100;
        ui->statsTextBrowser->append("Average percent increase when pattern is successful: <html><b>" + QString::number(avgPercentIncrease) + "%</b</html>");
    }

    sqlClose();
    sqlOpen();


}


void MainWindow::on_returnToStatsButton_clicked()
{
    on_stocksListView_clicked(indexToSave);
}

void MainWindow::on_removeStockButton_clicked()
{
    QString stockName;
    stockName = ui->stocksListView->model()->data(indexToSave).toString();

    QSqlQuery querry;
    querry.prepare("delete from '" + currStockListName + "table' where symbol = '" + stockName + "';");
    if (querry.exec()) {
        qDebug() << "[Database] Deleted stock " + stockName + " from table " + currStockListName;

    } else {
        QMessageBox::critical(this, tr("error in 1: "), querry.lastError().text());
    }

    QSqlQuery querry2;
    querry2.prepare("drop table if exists '" + currStockListName + "$" + stockName + "datatable';");
    if (querry2.exec()) {
        qDebug() << "[Database] Deleted data table for " + stockName;
    } else {
        QMessageBox::critical(this, tr("error in 2: "), querry2.lastError().text());
    }

    ui->stackedWidget_2->setCurrentIndex(0);

    //Load the stocks list view
    QSqlQueryModel * model = new QSqlQueryModel();
    QSqlQuery * querry3 = new QSqlQuery(mainDataBase);
    querry3->prepare("select symbol from '" + currStockListName + "table'");
    querry3->exec();
    model->setQuery(*querry3);
    ui->stocksListView->setModel(model);

    ui->stackedWidget_2->setCurrentIndex(0);
    ui->addDataWithinStockButton->setEnabled(false);
    ui->otherAddNewStockButton->setVisible(false);
    ui->addDataWithinStockButton->setText("Add/Edit");
    ui->specialDataTableAddDataButton->setVisible(false);
    ui->removeDataWithinStockButton->setEnabled(false);

    ui->priceAtTriggerLineEdit->setVisible(false);
    ui->priceAtPeakLineEdit->setVisible(false);
    ui->label_12->setVisible(false);
    ui->label_15->setVisible(false);

    ui->dateOfPeakPriceLineEdit->setVisible(false);
    ui->label_14->setVisible(false);
    ui->patternSuccessRadioButton->setChecked(false);
    ui->patternSuccessRadioButton->setVisible(false);
    ui->specialDataTableAddDataButton->setVisible(false);



}

void MainWindow::on_patternSuccessRadioButton_clicked()
{
    if (ui->patternSuccessRadioButton->isChecked()) {

        ui->priceAtTriggerLineEdit->setVisible(true);
        ui->priceAtPeakLineEdit->setVisible(true);
        ui->label_12->setVisible(true);
        ui->label_15->setVisible(true);

        ui->dateOfPeakPriceLineEdit->setVisible(true);
        ui->label_14->setVisible(true);
    } else {

        ui->priceAtTriggerLineEdit->setVisible(false);
        ui->priceAtPeakLineEdit->setVisible(false);
        ui->label_12->setVisible(false);
        ui->label_15->setVisible(false);

        ui->dateOfPeakPriceLineEdit->setVisible(false);
        ui->label_14->setVisible(false);
    }
}

void MainWindow::on_patternTriggeredRadioButton_clicked()
{
    if (ui->patternTriggeredRadioButton->isChecked()) {
        ui->patternSuccessRadioButton->setVisible(true);
        on_patternSuccessRadioButton_clicked();

    } else {
        ui->patternSuccessRadioButton->setVisible(false);

        ui->priceAtTriggerLineEdit->setVisible(false);
        ui->priceAtPeakLineEdit->setVisible(false);
        ui->label_12->setVisible(false);
        ui->label_15->setVisible(false);

        ui->dateOfPeakPriceLineEdit->setVisible(false);
        ui->label_14->setVisible(false);
    }
}


void MainWindow::on_dateStockLineEdit_editingFinished()
{
    QString currDate;
    currDate = ui->dateStockLineEdit->text();


}

void MainWindow::on_specialDataTableAddDataButton_clicked()
{


    ui->dateOfPeakPriceLineEdit->setVisible(false);
    ui->label_14->setVisible(false);
    ui->label_12->setVisible(false);
    ui->label_15->setVisible(false);
    incomingDate = "";

    qApp->processEvents();
    //clear all values in the line edits
    ui->dateStockLineEdit->setText("");
    ui->patternTriggeredStockLineEdit->setText("");
    ui->patternSuccessStockLineEdit->setText("");

    ui->priceAtTriggerLineEdit->setText("");
    ui->priceAtPeakLineEdit->setText("");
    ui->dateOfPeakPriceLineEdit->setText("");

    ui->newStockTableView->clearSelection();
    qApp->processEvents();
    ui->patternSuccessRadioButton->setChecked(false);
    on_patternSuccessRadioButton_clicked();





}




/*=============================================================================
 *
 * Trade Tracker Page
 *
 *=============================================================================*/

void MainWindow::on_tradeTrackerButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(6);

    QSqlQuery querry1;
    querry1.prepare("create table if not exists tradeTrackerOpentable (date varchar(20) primary key, symbol varchar(20), orderType varchar(20), amount varchar(20), price varchar(20), strikePrice varchar(20), expiration varchar(20))");

    if (querry1.exec()) {
        qDebug() << "[Database] tradeTrackerOpentable created or already exist";
    } else {
        qDebug() << "[Database] something is wrong with tradeTrackerOpentable intialization";
        QMessageBox::critical(this, tr("error: "), querry1.lastError().text());
    }


    QSqlQuery querry2;
    querry2.prepare("create table if not exists tradeTrackerClosetable (date varchar(20) primary key, symbol varchar(20), orderType varchar(20), amount varchar(20), price varchar(20), strikePrice varchar(20), expiration varchar(20))");

    if (querry2.exec()) {
        qDebug() << "[Database] tradeTrackerClosetable created or already exist";
    } else {
        qDebug() << "[Database] something is wrong with tradeTrackerClosetable intialization";
        QMessageBox::critical(this, tr("error: "), querry2.lastError().text());
    }

}

void MainWindow::on_backTradeTrackerButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}










































/*=============================================================================
 *
 * Testing
 *
 *=============================================================================*/
void MainWindow::on_pushButton_clicked()
{

    // Go to test page *******CURRENTLY DISCONNECTED*********
    ui->stackedWidget->setCurrentIndex(0);

    // Show the appdata path
    ui->label_3->setText(appdataLocation);


}

// Testing directory making
void MainWindow::on_createMainDataDirectoryButton_clicked()
{

    appdataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    ui->label_3->setText(appdataLocation);

    if (ui->label_3->text() == appdataLocation) {
        QDir dir;
        if (dir.mkdir(appdataLocation)) {
            ui->label_3->setText("Success: creates directory:" + appdataLocation);
        } else {
            ui->label_3->setText("failiure, directory probably already created");
        }
    } else {
        ui->label_3->setText(appdataLocation);
    }


}

// Testing directory empty
void MainWindow::on_emptyButton_clicked()
{

    appdataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    if (QDir(appdataLocation).entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 0) {
        ui->label_4->setText("It's empty");
    } else {
        ui->label_4->setText("It's not empty!");
    }
}

// Create a new database testing
void MainWindow::on_createDatabaseButton_clicked()
{

    if (!sqlOpen()) {
         ui->label_5->setText("Failed to open the database");

    } else {
        ui->label_5->setText("Connected...");
    }
}

// Removing a database testing
void MainWindow::on_deleteDatabaseButton_clicked()
{
    sqlClose();
    QFile::remove(appdataLocation + "/maindatabase.db");
}

// checking existance
void MainWindow::on_checkExistanceButton_clicked()
{
    if(fileExists(appdataLocation + "/maindatabase.db")) {
        ui->label_6->setText("maindatabase.db exist in this directory");
    } else {
        ui->label_6->setText("maindatabase.db does no exist here");
    }
}

// Return to homescreen
void MainWindow::on_returnButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

/*************************Test Page 2***********************************/

void MainWindow::on_pushButton_3_clicked()
{
    //sqlOpen();
    QString name, number, amount;
    name = ui->lineEdit_3->text();
    number = ui->lineEdit_2->text();
    amount = ui->lineEdit->text();

    QSqlQuery querry;
    querry.prepare("insert into testtable (number,name,amount) values ('"+number+"', '"+name+"','"+amount+"')");

    if (querry.exec()) {
        QMessageBox::critical(this, tr("Save"), tr("Saved"));
        //sqlClose();
        on_loadTableButton_clicked();
    } else {
        QMessageBox::critical(this, tr("error: "), querry.lastError().text());
        //sqlClose();
    }


}

void MainWindow::on_nextTestButton_clicked()
{
    sqlClose();
    ui->stackedWidget->setCurrentIndex(4);
    sqlOpen();
}

void MainWindow::on_returnButton2_clicked()
{
    sqlClose();
    ui->stackedWidget->setCurrentIndex(3);

}

void MainWindow::on_createTableButton_clicked()
{
    //sqlOpen();
    QSqlQuery querry;
    querry.prepare("create table if not exists testtable (number int primary key, "
                   "name varchar(20), amount int)");

    if (querry.exec()) {
        ui->creationLabel->setText("Created");
        on_loadTableButton_clicked();
    } else {
        ui->creationLabel->setText("Not Created");
    }

    //sqlClose();
}

void MainWindow::on_pushButton_2_clicked()
{
    //sqlClose();
}

void MainWindow::on_deleteTableButton_clicked()
{
    //sqlOpen();
    QSqlQuery querry;
    querry.prepare("DROP TABLE testtable;");
    if (querry.exec()) {
        QMessageBox::critical(this, tr("Table Edit"), tr("Table Deleted"));
        //sqlClose();
        on_loadTableButton_clicked();
    } else {
        QMessageBox::critical(this, tr("error: "), querry.lastError().text());
        //sqlClose();
    }
    //sqlClose();
}

void MainWindow::on_updateButton_clicked()
{
    //sqlOpen();
    QString name, number, amount;
    name = ui->lineEdit_3->text();
    number = ui->lineEdit_2->text();
    amount = ui->lineEdit->text();

    QSqlQuery querry;
    querry.prepare("update testtable set number='"+number+"', name='"+name+"', amount='"+amount+"' where number='"+number+"'");

    if (querry.exec()) {
        QMessageBox::critical(this, tr("Edit"), tr("Updated"));
        //sqlClose();
        on_loadTableButton_clicked();
    } else {
        QMessageBox::critical(this, tr("error: "), querry.lastError().text());
        //sqlClose();
    }
}

void MainWindow::on_deleteButton_clicked()
{
    //sqlOpen();
    QString number;
    number = ui->lineEdit_2->text();
    //number = ui->lineEdit_2->text();
    //amount = ui->lineEdit->text();

    QSqlQuery querry;
    querry.prepare("delete from testtable where number = " + number + ";");

    if (querry.exec()) {
        QMessageBox::critical(this, tr("Delete"), tr("Deleted"));
        //sqlClose();
        on_loadTableButton_clicked();
    } else {
        QMessageBox::critical(this, tr("error: "), querry.lastError().text());
        //sqlClose();
    }
}

void MainWindow::on_pushButton_4_clicked()
{
    QString qry;
    qry = ui->lineEdit_4->text();

    QSqlQuery querry;
    querry.prepare("SELECT * FROM testtable where number='"+qry+"'");

    if (querry.exec()) {
        QMessageBox::critical(this, tr("Find"), tr("Found"));
        //sqlClose();
    } else {
        QMessageBox::critical(this, tr("error: "), querry.lastError().text());
        //sqlClose();
    }
}

void MainWindow::on_loadTableButton_clicked()
{
    QSqlQueryModel * mainDatabaseModel = new QSqlQueryModel();

    QSqlQuery * querry = new QSqlQuery(mainDataBase);

    querry->prepare("select * from testtable");

    querry->exec();
    mainDatabaseModel->setQuery(*querry);
    ui->tableView->setModel(mainDatabaseModel);
}





