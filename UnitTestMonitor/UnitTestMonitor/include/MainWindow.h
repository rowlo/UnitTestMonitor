/**
  * @file MainWindow.h
  *
  * @class MainWindow
  *
  * @copyright (c) 2014, Robert Wloch
  * @license LGPL v2.1
  *
  * @brief Main window of the Unit Test Monitor
  * @details Controller interface of the main window of Unit Test Monitor.
  *
  * @author Robert Wloch, robert@rowlo.de
  *************************************************************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAtomicInt>
#include <QFutureWatcher>
#include <QPair>
#include <QMap>
#include <QList>
#include <QMainWindow>
#include <QSharedPointer>
#include <QSignalMapper>
#include <QTimer>
#include <QStandardItemModel>
#include <Model/MonitorSet.h>
#include <Model/Branch.h>
#include <Model/Testrun.h>
#include <BranchScanner.h>
#include <QMutex>

class QMenu;
class QPushButton;
class TlogViewDialog;
class LcovBrowserDialog;

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    bool hasBranches();
    bool isLibrarySelected();
    bool isTestSelected();
    bool isTestrunSelected();
    void enableIOActions(bool enabled);
    void fillMissingRuns(const QList<qint64> testrunKeys,
                                     QMap<qint64, QSharedPointer<Model::Testrun> > &runsMap);
    void appendFilledRow(
            QStandardItem *parentItem, int columnCount,
            QStandardItem *firstItem, const QMap<int, QPair<int, QIcon> > &icons,
            QMap<int, int> &passed, QMap<int, int> &failed, QMap<int, int> &skipped);
    void increaseCounter(int increment, int column, QMap<int, int> &counters);
    void pushRecentMonitorSetFile(const QString &recentFile);
    void updateRecentFilesMenu();
    bool openMonitorSetFile(const QString& fileName);

protected slots:
    void resetUi();
    void updateTools();

    void on_actionNewMonitorSet_triggered();
    void on_actionOpenMonitorSet_triggered();
    void on_actionQuit_triggered();
    void on_actionAbout_triggered();

    void on_addBranchToolButton_clicked();
    void on_removeBranchToolButton_clicked();
    void on_updateBranchToolButton_clicked();
    void on_viewLcovToolButton_clicked();
    void on_viewTlogToolButton_clicked();
    void on_deleteTestrunToolButton_clicked();

    void handleFinishedOpenMonitorSet();
    void handleFinishedSaveMonitorSet();
    void handleFinishedScanBranch();

    void initializeBranchTableModel();
    void updateBranchTabs();
    void branchTabClicked(const QString &text);
    void openRecentMonitorSetFile();
    void adjustColumnSize();

    void itemClicked(QModelIndex index);
    void itemDoubleClicked(QModelIndex index);

private:
    Ui::MainWindow *ui;

    QString m_currentMonitorSetFile;
    QFutureWatcher<QSharedPointer<Model::MonitorSet> > watcherOpenMonitorSet;
    QFutureWatcher<bool> watcherSaveMonitorSet;
    QFutureWatcher<QSharedPointer<Model::Branch> > watcherScanBranch;
    QStandardItemModel* m_branchTableModel;
    QSharedPointer<Model::MonitorSet> m_monitorSet;
    BranchScanner m_branchScanner;
    QAtomicInt m_ioBlocked;
    QTimer m_openPollTimer;
    QTimer m_savePollTimer;
    QTimer m_scanPollTimer;
    QSignalMapper m_branchTabsSignalMapper;
    QSharedPointer<Model::Branch> m_selectedBranch;
    QStringList m_recentMonitorSetFiles;
    QMutex m_recentMonitorSetFilesMutex;
    QMenu* m_recentFilesMenu;

    QStandardItem* m_selectedLibrary;
    QStandardItem* m_selectedTestcase;
    qint64 m_selectedTestrun;
    TlogViewDialog* tlogViewDialog;
    LcovBrowserDialog* lcovBrowserDialog;
    QMap<int, qint64> m_headerTimestamps;

};

#endif // MAINWINDOW_H
