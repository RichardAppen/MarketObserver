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


}

bool MainWindow::sqlOpen() {
    appdataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    mainDataBase = QSqlDatabase::addDatabase("QSQLITE");
    mainDataBase.setDatabaseName(appdataLocation + "/maindatabase.db");

    if (!mainDataBase.open()) {
         qDebug() << "[Database] failed to open maindatabase.db";
         return false;

    } else {
        qDebug() << "[Database] maindatabase.db opened...";
        return true;
    }
}

void MainWindow::sqlClose() {

    mainDataBase.close();
    mainDataBase.removeDatabase(QSqlDatabase::defaultConnection);
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
    querry1.prepare("create table if not exists patternstable (name varchar(20) primary key, indicator1 varchar(20), indicator2 varchar(20))");

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
}

void MainWindow::on_patternsListView_clicked(const QModelIndex &index)
{
    ui->editNewPatternsButton->setEnabled(true);
    ui->removeNewPatternsButton->setEnabled(true);
    currPatternListViewIndex = index;

    QString patternName, indicator1, indicator2;
    patternName = ui->patternsListView->model()->data(index).toString();

    QSqlQuery querry;
    querry.prepare("select * from patternstable where name='" + patternName + "' ");

    if (querry.exec()) {
        qDebug() << "Found selected pattern to display in additional info box";

        while (querry.next()) {
            indicator1 = querry.value(1).toString();
            indicator2 = querry.value(2).toString();
        }

    } else {
        qDebug() << "Failed to deal with addtional info display information";
        indicator1 = indicator2 = "";
    }


    ui->additionalInfoBrowser->setText(patternName + ":\n" + "indicator1: " + indicator1 + "\n" + "indicator2: " + indicator2);
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
    // Find the stock list name of the currently selected item in the list view
    QString stockListName;
    stockListName = ui->stockListListView->model()->data(currStockListListViewIndex).toString();

    QSqlQuery querry;
    querry.prepare("delete from stocklisttable where name = '" + stockListName + "';");

    if (querry.exec()) {
        qDebug() << "[Database] deleted pattern: " + stockListName;
    } else {
        qDebug() << "[Databse] Did not handle deleting of a pattern";
        QMessageBox::critical(this, tr("error: "), querry.lastError().text());
    }


    //Load the stock list view
    QSqlQueryModel * model = new QSqlQueryModel();
    QSqlQuery * querry2 = new QSqlQuery(mainDataBase);
    querry2->prepare("select name from stocklisttable");
    querry2->exec();
    model->setQuery(*querry2);
    ui->stockListListView->setModel(model);
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
    querry1.prepare("create table if not exists " + stockListName + "table (symbol varchar(20) primary key, pattern varchar(20), patternTriggered int, patternSuccess int, avgPriceInc int, avgDayPeak int, streakAmount int)");

    if (querry1.exec()) {
        qDebug() << "[Database] specific stock table for: " + stockListName + " created or already exist";
    } else {
        qDebug() << "[Database] something is wrong with specific stock table intialization";
        QMessageBox::critical(this, tr("error: "), querry1.lastError().text());
    }


    //Load the stocks list view
    QSqlQueryModel * model = new QSqlQueryModel();
    QSqlQuery * querry2 = new QSqlQuery(mainDataBase);
    querry2->prepare("select symbol from " + stockListName + "table");
    querry2->exec();
    model->setQuery(*querry2);
    ui->stocksListView->setModel(model);

    ui->stackedWidget_2->setCurrentIndex(0);
    ui->addDataWithinStockButton->setEnabled(false);
    ui->addDataWithinStockButton->setText("Add/Edit");

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


void MainWindow::on_controlPageNewStockButton_clicked()
{
    ui->stackedWidget_2->setCurrentIndex(1);
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
    querry1->prepare("select * from " + stockListName + "$" + stockName + "datatable");
    querry1->exec();
    model->setQuery(*querry1);
    ui->newStockTableView->setModel(model);

    QSqlQuery querry;
    querry.prepare("insert into " + stockListName + "table (symbol, pattern, patternTriggered, patternSuccess, avgPriceInc, avgDayPeak, streakAmount) values ('" + stockName + "', '', '', '', '', '', '')");

    if (querry.exec()) {
        qDebug() << "[Database] Inserted new stock into the stock List: " + stockName;
    } else {
        qDebug() << "[Database] Failed to insert new stock into the stock list " + stockName;
        QMessageBox::critical(this, tr("error: "), tr("This stock list already exists!"));
    }

    //Load this specific stocks list view
    QSqlQueryModel * model2 = new QSqlQueryModel();
    QSqlQuery * querry2 = new QSqlQuery(mainDataBase);
    querry2->prepare("select symbol from " + stockListName + "table");
    qDebug() << stockListName << "table";
    querry2->exec();
    model2->setQuery(*querry2);
    ui->stocksListView->setModel(model2);

}


void MainWindow::on_stocksListView_clicked(const QModelIndex &index)
{
    // First clear all values in the line edits
    ui->dateStockLineEdit->setText("");
    ui->patternTriggeredStockLineEdit->setText("");
    ui->patternSuccessStockLineEdit->setText("");
    ui->priceIncreaseStockLineEdit->setText("");
    ui->dateOfPeakPriceLineEdit->setText("");

    ui->stackedWidget_2->setCurrentIndex(1);
    //Load the clicked stock's datatable
    QString stockListName = ui->stockListListView->model()->data(currStockListListViewIndex).toString();
    QString stockName;
    stockName = ui->stocksListView->model()->data(index).toString();

    QSqlQueryModel * model = new QSqlQueryModel();
    QSqlQuery * querry1 = new QSqlQuery(mainDataBase);
    querry1->prepare("select * from " + stockListName + "$" + stockName + "datatable");
    querry1->exec();
    model->setQuery(*querry1);
    ui->newStockTableView->setModel(model);

    currStockDataTable = stockListName + "$" + stockName + "datatable";

    ui->addDataWithinStockButton->setText("Add");

}




void MainWindow::on_newStockTableView_clicked(const QModelIndex &index)
{
    QString val = ui->newStockTableView->model()->data(index).toString();

    QSqlQuery querry1;
    querry1.prepare("select * from " + currStockDataTable + " where date = '" + val + "' or pattern = '" + val + "' or patternTriggered = '" + val + "' or patternSuccess = '" + val + "' or priceIncrease = '" + val + "' or dateOfPricePeak = '" + val + "'");
    if (querry1.exec()) {

        while (querry1.next()) {
            ui->dateStockLineEdit->setText(querry1.value(0).toString());
            incomingDate = querry1.value(0).toString();
            ui->patternTriggeredStockLineEdit->setText(querry1.value(2).toString());
            ui->patternSuccessStockLineEdit->setText(querry1.value(3).toString());
            ui->priceIncreaseStockLineEdit->setText(querry1.value(4).toString());
            ui->dateOfPeakPriceLineEdit->setText(querry1.value(5).toString());

        }
    } else {
        QMessageBox::critical(this, tr("error: "), querry1.lastError().text());
    }

    ui->addDataWithinStockButton->setText("Update");


}

void MainWindow::on_addDataWithinStockButton_clicked()
{

    QString date, pattern, patternTriggered, patternSuccess, priceIncrease, dateOfPeakPrice;
    date = ui->dateStockLineEdit->text();
    pattern = currentStockListsPattern;
    patternTriggered = ui->patternTriggeredStockLineEdit->text();
    patternSuccess = ui->patternSuccessStockLineEdit->text();
    priceIncrease = ui->priceIncreaseStockLineEdit->text();
    dateOfPeakPrice = ui->dateOfPeakPriceLineEdit->text();
    // if we aren't editing
    if (incomingDate.compare("") == 0) {

        QSqlQuery querry;
        querry.prepare("insert into " + currStockDataTable + " (date,pattern,patternTriggered,patternSuccess,priceIncrease,dateOfPricePeak) values ('" + date + "', '" + pattern + "', '" + patternTriggered + "', '" + patternSuccess + "', '" + priceIncrease + "', '" + dateOfPeakPrice + "')");

        if (querry.exec()) {
            qDebug() << "[Database] Inserted new data into datatable: " + currStockDataTable;
        } else {
            qDebug() << "[Database] Failed to insert new data into datatable: " + currStockDataTable;
            QMessageBox::critical(this, tr("error: "), tr("The data for this date already exists!"));
        }
    } else {

        QSqlQuery querry;
        querry.prepare("update " + currStockDataTable + " set date = '" + date + "', pattern = '" + pattern + "', patternTriggered = '" + patternTriggered + "', patternSuccess = '" + patternSuccess + "', priceIncrease = '" + priceIncrease + "', dateOfPricePeak = '" + dateOfPeakPrice + "' where date = '" + incomingDate + "';");

        if (querry.exec()) {
            qDebug() << "[Database] Edited the data in: " + currStockDataTable + " where date had equaled " + incomingDate;

        } else {
            qDebug() << "[Database] Failed to edit the data in: " + currStockDataTable;
            QMessageBox::critical(this, tr("error: "), tr("Couldn't find the date you were trying to edit!"));
        }
    }

    // Reload the datatable for both cases
    QSqlQueryModel * model = new QSqlQueryModel();
    QSqlQuery * querry1 = new QSqlQuery(mainDataBase);
    querry1->prepare("select * from " + currStockDataTable);
    querry1->exec();
    model->setQuery(*querry1);
    ui->newStockTableView->setModel(model);

    // once updated clear the line edits and reset the add button and incomingDate
    ui->dateStockLineEdit->setText("");
    ui->patternTriggeredStockLineEdit->setText("");
    ui->patternSuccessStockLineEdit->setText("");
    ui->priceIncreaseStockLineEdit->setText("");
    ui->dateOfPeakPriceLineEdit->setText("");
    ui->addDataWithinStockButton->setText("Add");
    incomingDate = "";
    ui->addDataWithinStockButton->setEnabled(false);

}

void MainWindow::on_dateStockLineEdit_textChanged(const QString &arg1)
{
    ui->addDataWithinStockButton->setEnabled(true);
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



