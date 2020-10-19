#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <QApplication>
#include <QtSql>
#include <QtDebug>
#include <QFileInfo>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <vector>
#include <QSet>



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QSqlDatabase mainDataBase;
    QString appdataLocation;
    void sqlClose();
    bool sqlOpen();
    QModelIndex currPatternListViewIndex;
    QModelIndex currStockListListViewIndex;
    QString currentStockListsPattern;
    QString currStockDataTable;
    QString currStockName;
    QString currStockListName;
    QString incomingDate;
    QModelIndex indexToSave;
    QSet<QString> patternsInUse;

private slots:
    void on_patternsButton_clicked();

    void on_stockListButton_clicked();

    void on_pushButton_clicked();




    // Testing slots ** Eventually to be disconnected and removed **
    void on_createMainDataDirectoryButton_clicked();
    void on_emptyButton_clicked();
    void on_createDatabaseButton_clicked();
    void on_deleteDatabaseButton_clicked();
    void on_checkExistanceButton_clicked();
    void on_returnButton_clicked();
    void on_pushButton_3_clicked();
    void on_nextTestButton_clicked();
    void on_returnButton2_clicked();
    void on_createTableButton_clicked();
    void on_pushButton_2_clicked();
    void on_deleteTableButton_clicked();
    void on_updateButton_clicked();
    void on_deleteButton_clicked();
    void on_pushButton_4_clicked();
    void on_loadTableButton_clicked();




    void on_patternPageBackButton_clicked();

    void on_plusminusPatternsButton_clicked();

    void on_editNewPatternsButton_clicked();

    void on_removeNewPatternsButton_clicked();

    void on_patternsListView_clicked(const QModelIndex &index);

    void on_stockListPageBackButton_clicked();

    void on_plusStockListButton_clicked();

    void on_editNewStockListsButton_clicked();

    void on_removeNewStockListButton_clicked();

    void on_stockListListView_clicked(const QModelIndex &index);

    void on_viewStockListsButton_clicked();

    void on_stocksBackButton_clicked();

    void on_controlPageNewStockButton_clicked();

    void on_stocksListView_clicked(const QModelIndex &index);

    void on_newStockTableView_clicked(const QModelIndex &index);

    void on_addDataWithinStockButton_clicked();

    void on_dateStockLineEdit_textChanged(const QString &arg1);

    void on_removeDataWithinStockButton_clicked();

    void on_editStatsbutton_clicked();

    void load_stockStats();

    void on_returnToStatsButton_clicked();

    void on_removeStockButton_clicked();

    void on_otherAddNewStockButton_clicked();

    void on_patternSuccessRadioButton_clicked();

    void on_patternTriggeredRadioButton_clicked();

    void on_dateStockLineEdit_editingFinished();

    void on_specialDataTableAddDataButton_clicked();

    void on_tradeTrackerButton_clicked();

    void on_backTradeTrackerButton_clicked();

private:
    Ui::MainWindow *ui;
    void setAppDataDirectory();

};
#endif // MAINWINDOW_H
