/**
  * @file MainWindow.cpp
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
#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QtCore>
#include <QFileDialog>
#include <QMessageBox>
#include <QIcon>
#include <QMenu>
#include <QDateTime>
#include <QSettings>
#include <QFutureWatcher>
#include <QLayoutItem>
#include <QPushButton>
#include <QtConcurrent>

#include <Model/MonitorSet.h>
#include "Model/Branch.h"
#include "Model/Project.h"
#include "Model/Library.h"
#include "Model/Testcase.h"
#include "Model/Testrun.h"
#include <MonitorSetReader.h>
#include <MonitorSetWriter.h>
#include <AboutDialog.h>
#include <TlogViewDialog.h>
#include <LcovBrowserDialog.h>

using Model::MonitorSet;
using Model::Branch;
using Model::Project;
using Model::Library;
using Model::Testcase;
using Model::Testrun;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_branchTableModel(),
    m_ioBlocked(0),
    m_selectedLibrary(0),
    m_selectedTestcase(0),
    m_selectedTestrun(-1),
    tlogViewDialog(0),
    lcovBrowserDialog(0)
{
    ui->setupUi(this);
    m_recentFilesMenu = new QMenu("&Recent Files");
    ui->menuFile->insertMenu(ui->actionPlaceHolderRecentFiles, m_recentFilesMenu);

    ui->mainToolBar->setVisible(false);
    resetUi();
    ui->stackedWidget->setCurrentIndex(0);

    m_branchTableModel = new QStandardItemModel(ui->branchTestsTreeView);
    ui->branchTestsTreeView->setModel(m_branchTableModel);

    m_openPollTimer.setInterval(1000);
    m_openPollTimer.setSingleShot(false);
    m_savePollTimer.setInterval(1000);
    m_savePollTimer.setSingleShot(false);
    m_scanPollTimer.setInterval(1000);
    m_scanPollTimer.setSingleShot(false);

    connect(&m_openPollTimer, SIGNAL(timeout()),
            this, SLOT(handleFinishedOpenMonitorSet()));
    connect(&m_savePollTimer, SIGNAL(timeout()),
            this, SLOT(handleFinishedSaveMonitorSet()));
    connect(&m_scanPollTimer, SIGNAL(timeout()),
            this, SLOT(handleFinishedScanBranch()));
    connect(&m_branchTabsSignalMapper, SIGNAL(mapped(const QString &)),
                 this, SLOT(branchTabClicked(const QString &)));
    connect(ui->branchTestsTreeView, SIGNAL(expanded(QModelIndex)), SLOT(adjustColumnSize()));
    connect(ui->branchTestsTreeView, SIGNAL(collapsed(QModelIndex)), SLOT(adjustColumnSize()));
    connect(ui->branchTestsTreeView, SIGNAL(clicked(QModelIndex)),
            SLOT(itemClicked(QModelIndex)));
    connect(ui->branchTestsTreeView, SIGNAL(doubleClicked(QModelIndex)),
            SLOT(itemDoubleClicked(QModelIndex)));
    connect(ui->toolButtonGetStarted, SIGNAL(clicked()), SLOT(on_actionNewMonitorSet_triggered()));
    connect(ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    QSettings settings;
    for (int i = 0, size = settings.beginReadArray("recentFiles"); i < size; ++i) {
        settings.setArrayIndex(i);
        QString recentFile = settings.value("recentFile").toString();
        QFileInfo fileInfo(recentFile);
        if (fileInfo.isFile() && fileInfo.exists())
        {
            m_recentMonitorSetFiles.append(recentFile);
        }
    }
    settings.endArray();
    if (m_recentMonitorSetFiles.isEmpty())
    {
        ui->stackedWidgetGetStarted->setCurrentIndex(1);
    }
    updateRecentFilesMenu();

    if (settings.contains("MainWindow/size"))
    {
        QVariant var = settings.value("MainWindow/size");
        if (var.canConvert<QSize>())
        {
            resize(var.toSize());
        }
    }
    if (settings.contains("MainWindow/pos"))
    {
        QVariant var = settings.value("MainWindow/pos");
        if (var.canConvert<QPoint>())
        {
            move(var.toPoint());
        }
    }
}

void MainWindow::adjustColumnSize()
{
    ui->branchTestsTreeView->resizeColumnToContents(0);
}

void MainWindow::updateRecentFilesMenu()
{
    QList<QAction*> oldActions = m_recentFilesMenu->actions();
    foreach (const QAction* oldAction, oldActions)
    {
        if (oldAction)
        {
            delete oldAction;
        }
    }
    m_recentFilesMenu->clear();

    m_recentMonitorSetFilesMutex.lock();
    for (int i = 0, size = m_recentMonitorSetFiles.size(); i < size; ++i) {
        QString recentFile = m_recentMonitorSetFiles.at(i);
        QString fileName = QFileInfo(recentFile).fileName();

        QAction* recentFileAction = new QAction(
                    QIcon(":/images/recent.png"),
                    QString("&%1 %2").arg(i + 1).arg(fileName), this);
        recentFileAction->setShortcut(QKeySequence(tr("Ctrl+%1").arg(i + 1)));
        recentFileAction->setData(recentFile);
        m_recentFilesMenu->addAction(recentFileAction);
        connect(recentFileAction, SIGNAL(triggered()), SLOT(openRecentMonitorSetFile()));
    }
    m_recentMonitorSetFilesMutex.unlock();
}

void MainWindow::resetUi()
{
    // hide tool menu buttons
    {
        ui->branchTabsToolsMenu->setVisible(false);
        ui->tabToolsMenu->setVisible(false);
    }
    // disable tools
    {
        ui->addBranchToolButton->setEnabled(false);
        ui->removeBranchToolButton->setEnabled(false);
        ui->updateBranchToolButton->setEnabled(false);

        ui->viewLcovToolButton->setEnabled(false);
        ui->viewTlogToolButton->setEnabled(false);
        ui->deleteTestrunToolButton->setEnabled(false);
    }
}

MainWindow::~MainWindow()
{
    QSettings settings;
    settings.setValue("MainWindow/size", size());
    settings.setValue("MainWindow/pos", pos());
    settings.sync();

    delete ui;
}

void MainWindow::updateTools()
{
    bool modelHasBranches = hasBranches();
    bool isBranchSelected = not m_selectedBranch.isNull();
    ui->addBranchToolButton->setEnabled(true);
    ui->removeBranchToolButton->setEnabled(modelHasBranches && isBranchSelected);
    ui->updateBranchToolButton->setEnabled(modelHasBranches && isBranchSelected);

    ui->viewLcovToolButton->setEnabled(isLibrarySelected());
    ui->viewTlogToolButton->setEnabled(isTestSelected());
    ui->deleteTestrunToolButton->setEnabled(isTestrunSelected());
}

bool MainWindow::hasBranches()
{
    return not m_monitorSet.isNull() && m_monitorSet->getBranchesCount() > 0;
}

bool MainWindow::isLibrarySelected()
{
    return m_selectedLibrary != 0;
}

bool MainWindow::isTestSelected()
{
    return m_selectedTestcase != 0;
}

bool MainWindow::isTestrunSelected()
{
    return m_selectedTestrun >= 0;
}

void MainWindow::on_actionNewMonitorSet_triggered()
{
    if (not m_ioBlocked.testAndSetAcquire(0, 1))
    {
        ui->statusBar->showMessage(
                    tr("MainWindow: Failed to create new monitor set as I/O operation is pending."),
                    5000);
        return;
    }
    enableIOActions(false);
    QSettings settings;
    QString  lastFileDialogPath = settings.value("lastFileDialogPath", QDir::homePath()).toString();

    QString fileName = QFileDialog::getSaveFileName(
                this, tr("New Monitor Set"), lastFileDialogPath, tr("Unit Test Monitor (*.utm)"));

    if (not fileName.isNull() && not fileName.isEmpty())
    {
        if (not fileName.endsWith(".utm"))
        {
            fileName = fileName.append(".utm");
        }
        lastFileDialogPath = QFileInfo(fileName).absoluteDir().absolutePath();
        settings.setValue("lastFileDialogPath", lastFileDialogPath);

        m_currentMonitorSetFile = fileName;
        pushRecentMonitorSetFile(m_currentMonitorSetFile);

        m_monitorSet = QSharedPointer<MonitorSet>(new MonitorSet());

        MonitorSetWriter writer(fileName);
        QFuture<bool> future = QtConcurrent::run(writer, &MonitorSetWriter::write, m_monitorSet);
        watcherSaveMonitorSet.setFuture(future);
        m_savePollTimer.start(10);

        m_branchTableModel->clear();

        updateTools();
        ui->stackedWidget->setCurrentIndex(1);
        ui->stackedWidgetGetStarted->setCurrentIndex(0);
    }
    else
    {
        m_ioBlocked = 0;
        enableIOActions(true);
    }
}

void MainWindow::on_actionOpenMonitorSet_triggered()
{
    if (not m_ioBlocked.testAndSetAcquire(0, 1))
    {
        ui->statusBar->showMessage(
                    tr("MainWindow: Failed to open as I/O operation is pending."), 5000);
        return;
    }
    enableIOActions(false);

    QSettings settings;
    QString  lastFileDialogPath = settings.value("lastFileDialogPath", QDir::homePath()).toString();

    QString fileName = QFileDialog::getOpenFileName(
                this, tr("Open Monitor Set"), lastFileDialogPath, tr("Unit Test Monitor (*.utm)"));
    if (openMonitorSetFile(fileName))
    {
        lastFileDialogPath = QFileInfo(fileName).absoluteDir().absolutePath();
        settings.setValue("lastFileDialogPath", lastFileDialogPath);
    }
    else
    {
        m_ioBlocked = 0;
        enableIOActions(true);
    }
}

bool MainWindow::openMonitorSetFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        ui->statusBar->showMessage(
                    tr("Cannot read file %1: %2.").arg(fileName).arg(file.errorString()), 5000);
        return false;
    }

    if (not fileName.isNull() && not fileName.isEmpty())
    {
        resetUi();

        MonitorSetReader reader(fileName);
        QFuture<QSharedPointer<MonitorSet> > future =
                QtConcurrent::run(reader, &MonitorSetReader::read);
        watcherOpenMonitorSet.setFuture(future);
        m_openPollTimer.start(10);

        m_currentMonitorSetFile = fileName;
        pushRecentMonitorSetFile(m_currentMonitorSetFile);

        return true;
    }
    else
    {
        ui->statusBar->showMessage(tr("Cannot open invalid file."), 5000);
    }

    return false;
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::instance()->quit();
}

void MainWindow::on_actionAbout_triggered()
{
    AboutDialog dialog(this);
    dialog.exec();
}

void MainWindow::on_addBranchToolButton_clicked()
{
    if (not m_ioBlocked.testAndSetAcquire(0, 1))
    {
        ui->statusBar->showMessage(
                    tr("MainWindow: Failed to add branch as I/O operation is pending."),
                    5000);
        return;
    }
    enableIOActions(false);

    QSettings settings;
    QString  lastFileDialogPath =
            settings.value("lastBranchDialogPath", QDir::homePath()).toString();

    QString branchPath = QFileDialog::getExistingDirectory(
                this, tr("New Monitor Set"), lastFileDialogPath);

    if (not branchPath.isNull() && not branchPath.isEmpty())
    {
        lastFileDialogPath = QFileInfo(branchPath).absoluteFilePath();
        settings.setValue("lastBranchDialogPath", lastFileDialogPath);

        BranchScanner scanner;
        QFuture<QSharedPointer<Branch> > future =
                QtConcurrent::run(scanner, &BranchScanner::scanBranch, branchPath);
        watcherScanBranch.setFuture(future);
        m_scanPollTimer.start(10);
    }
    else
    {
        m_ioBlocked = 0;
        enableIOActions(true);
    }
}

void MainWindow::on_removeBranchToolButton_clicked()
{
    if (not m_ioBlocked.testAndSetAcquire(0, 1))
    {
        ui->statusBar->showMessage(
                    tr("MainWindow: Failed to add branch as I/O operation is pending."),
                    5000);
        return;
    }
    enableIOActions(false);

    if (not m_selectedBranch.isNull())
    {
        m_monitorSet->removeBranch(m_selectedBranch);
    }

    QFileInfo fileInfo(m_currentMonitorSetFile);
    if (fileInfo.exists() && fileInfo.isFile())
    {
        MonitorSetWriter writer(m_currentMonitorSetFile);
        QFuture<bool> futureSave = QtConcurrent::run(writer, &MonitorSetWriter::write, m_monitorSet);
        watcherSaveMonitorSet.setFuture(futureSave);
        m_savePollTimer.start(10);
    }
    else
    {
        m_ioBlocked = 0;
        enableIOActions(true);
        initializeBranchTableModel();
    }
    updateBranchTabs();
}

void MainWindow::on_updateBranchToolButton_clicked()
{
    if (not m_ioBlocked.testAndSetAcquire(0, 1))
    {
        ui->statusBar->showMessage(
                    tr("MainWindow: Failed to update branch as I/O operation is pending."),
                    5000);
        return;
    }
    enableIOActions(false);

    QSharedPointer<Branch> branch = m_selectedBranch;

    if (not branch.isNull())
    {
        BranchScanner scanner;
        QFuture<QSharedPointer<Branch> > future =
                QtConcurrent::run(scanner, &BranchScanner::updateBranch, branch);
        watcherScanBranch.setFuture(future);
        m_scanPollTimer.start(10);
    }
    else
    {
        m_ioBlocked = 0;
        enableIOActions(true);
    }
}

void MainWindow::on_viewLcovToolButton_clicked()
{
    QStandardItem* libraryItem = m_selectedLibrary;
    if (not libraryItem)
    {
        return;
    }
    QVariant lcovPathData = libraryItem->data(Qt::UserRole + 1);
    if (lcovPathData.isValid())
    {
        QString lcovPath = lcovPathData.toString();
        if (not lcovBrowserDialog)
        {
            lcovBrowserDialog = new LcovBrowserDialog(this);
        }
        lcovBrowserDialog->initializeForLcov(lcovPath);
        lcovBrowserDialog->show();

        QSettings settings;
        if (settings.contains("LcovBrowserDialog/size"))
        {
            QVariant var = settings.value("LcovBrowserDialog/size");
            if (var.canConvert<QSize>())
            {
                lcovBrowserDialog->resize(var.toSize());
            }
        }
        if (settings.contains("LcovBrowserDialog/pos"))
        {
            QVariant var = settings.value("LcovBrowserDialog/pos");
            if (var.canConvert<QPoint>())
            {
                lcovBrowserDialog->move(var.toPoint());
            }
        }

    }

}

void MainWindow::on_viewTlogToolButton_clicked()
{
    QStandardItem* testcaseItem = m_selectedTestcase;
    if (not testcaseItem)
    {
        return;
    }
    QVariant tlogPathData = testcaseItem->data(Qt::UserRole + 1);
    QVariant tlogStartLineData = testcaseItem->data(Qt::UserRole + 2);
    QVariant tlogEndLineData = testcaseItem->data(Qt::UserRole + 3);
    if (tlogPathData.isValid() && tlogStartLineData.isValid() && tlogEndLineData.isValid())
    {
        QString tlogPath = tlogPathData.toString();
        QString tlogStartLine = tlogStartLineData.toString();
        QString tlogEndLine = tlogEndLineData.toString();
        if (not tlogViewDialog)
        {
            tlogViewDialog = new TlogViewDialog(this);
        }
        tlogViewDialog->initializeForTlog(tlogPath, tlogStartLine, tlogEndLine);
        tlogViewDialog->show();

        QSettings settings;
        if (settings.contains("TlogViewDialog/size"))
        {
            QVariant var = settings.value("TlogViewDialog/size");
            if (var.canConvert<QSize>())
            {
                tlogViewDialog->resize(var.toSize());
            }
        }
        if (settings.contains("TlogViewDialog/pos"))
        {
            QVariant var = settings.value("TlogViewDialog/pos");
            if (var.canConvert<QPoint>())
            {
                tlogViewDialog->move(var.toPoint());
            }
        }

    }
}

void MainWindow::on_deleteTestrunToolButton_clicked()
{
    if (not m_ioBlocked.testAndSetAcquire(0, 1))
    {
        ui->statusBar->showMessage(
                    tr("MainWindow: Failed to create delete testrun as I/O operation is pending."),
                    5000);
        return;
    }
    enableIOActions(false);

    qint64 timestamp = m_selectedTestrun;
    if (timestamp < 0)
    {
        return;
    }

    foreach (const QSharedPointer<Project> &project, m_selectedBranch->getProjects())
    {
        foreach (const QSharedPointer<Library> &library, project->getLibraries())
        {
            foreach (const QSharedPointer<Testcase> &testcase, library->getTestcases())
            {
                testcase->deleteTestrun(timestamp);
            }
        }
    }

    QFileInfo fileInfo(m_currentMonitorSetFile);
    if (fileInfo.exists() && fileInfo.isFile())
    {
        MonitorSetWriter writer(m_currentMonitorSetFile);
        QFuture<bool> futureSave = QtConcurrent::run(writer, &MonitorSetWriter::write, m_monitorSet);
        watcherSaveMonitorSet.setFuture(futureSave);
        m_savePollTimer.start(10);
    }
    else
    {
        m_ioBlocked = 0;
        enableIOActions(true);
        initializeBranchTableModel();
    }
    m_selectedTestrun = -1;
}

void MainWindow::handleFinishedOpenMonitorSet()
{
    QFuture<QSharedPointer<MonitorSet> > future = watcherOpenMonitorSet.future();
    if (not future.isResultReadyAt(0))
    {
        return;
    }
    m_openPollTimer.stop();
    QSharedPointer<MonitorSet> monitorSet = future.result();

    m_monitorSet = monitorSet;

    initializeBranchTableModel();
    updateBranchTabs();
    ui->stackedWidget->setCurrentIndex(1);
    if (m_monitorSet->getBranchesCount() > 0)
    {
        QSharedPointer<Branch> branch = m_monitorSet->getBranches().first();
        branchTabClicked(branch->getName());
    }

    m_ioBlocked = 0;
    enableIOActions(true);
}

void MainWindow::handleFinishedSaveMonitorSet()
{
    QFuture<bool> future = watcherSaveMonitorSet.future();
    if (not future.isResultReadyAt(0))
    {
        return;
    }
    m_savePollTimer.stop();
    bool result = future.result();
    if (not result)
    {
        ui->statusBar->showMessage(
                    tr("MainWindow: Failed to save monitor set to file: %1.")
                    .arg(m_currentMonitorSetFile), 5000);
    }

    initializeBranchTableModel();

    m_ioBlocked = 0;
    enableIOActions(true);
}

void MainWindow::handleFinishedScanBranch()
{
    QFuture<QSharedPointer<Branch> > future = watcherScanBranch.future();
    if (not future.isResultReadyAt(0))
    {
        return;
    }
    m_scanPollTimer.stop();
    QSharedPointer<Branch> branch = future.result();
    m_monitorSet->addBranch(branch);

    QFileInfo fileInfo(m_currentMonitorSetFile);
    if (fileInfo.exists() && fileInfo.isFile())
    {
        MonitorSetWriter writer(m_currentMonitorSetFile);
        QFuture<bool> futureSave = QtConcurrent::run(writer, &MonitorSetWriter::write, m_monitorSet);
        watcherSaveMonitorSet.setFuture(futureSave);
        m_savePollTimer.start(10);
    }
    else
    {
        m_ioBlocked = 0;
        enableIOActions(true);
        initializeBranchTableModel();
    }
    updateBranchTabs();
    branchTabClicked(branch->getName());
}

void MainWindow::enableIOActions(bool enabled)
{
    ui->actionOpenMonitorSet->setEnabled(enabled);
    ui->actionNewMonitorSet->setEnabled(enabled);
    if (enabled)
    {
        updateTools();
        return;
    }
    ui->addBranchToolButton->setEnabled(enabled);
    ui->removeBranchToolButton->setEnabled(enabled);
    ui->updateBranchToolButton->setEnabled(enabled);
    ui->deleteTestrunToolButton->setEnabled(enabled);
}

void MainWindow::initializeBranchTableModel()
{
    m_branchTableModel->clear();

    if (not m_selectedBranch.isNull())
    {
        QStandardItem *rootItem = m_branchTableModel->invisibleRootItem();

        int columnCount = 1;
        QList<qint64> testrunKeys;
        QMap<qint64, QSharedPointer<Testrun> > runsMap;

        foreach (const QSharedPointer<Project> &project, m_selectedBranch->getProjects())
        {
            foreach (const QSharedPointer<Library> &library, project->getLibraries())
            {
                foreach (const QSharedPointer<Testcase> &testcase, library->getTestcases())
                {
                    columnCount = qMax(columnCount, testcase->getTestrunsCount() + 1);
                    // collect timestamps
                    QList<QSharedPointer<Testrun> > testrunsUnsorted = testcase->getTestruns();
                    foreach (const QSharedPointer<Testrun> &testrun, testrunsUnsorted)
                    {
                        qint64 timestamp = testrun->getTimestamp();
                        runsMap.insert(timestamp, testrun);
                    }
                }
            }
        }
        testrunKeys = runsMap.keys();
        runsMap.clear();

        m_branchTableModel->setColumnCount(columnCount);

        QIcon iconPassed(":/images/passed.png");
        QIcon iconFailed(":/images/failed.png");
        QIcon iconSkipped(":/images/skipped.png");
        QStringList headerLabels;
        headerLabels << "Project/Library/Testcase";
        QMap<int, qint64> timestamps;


        foreach (const QSharedPointer<Project> &project, m_selectedBranch->getProjects())
        {
            QString projectName = project->getName();
            QStandardItem *projectItem = new QStandardItem(projectName);
            QMap<int, QPair<int, QIcon> > iconsProject;
            QMap<int, int> passedProject;
            QMap<int, int> failedProject;
            QMap<int, int> skippedProject;

            foreach (const QSharedPointer<Library> &library, project->getLibraries())
            {
                QStandardItem *libraryItem = new QStandardItem(library->getName());
                libraryItem->setData(library->getLcovPath());
                QMap<int, QPair<int, QIcon> > iconsLibrary;
                QMap<int, int> passedLibrary;
                QMap<int, int> failedLibrary;
                QMap<int, int> skippedLibrary;

                foreach (const QSharedPointer<Testcase> &testcase, library->getTestcases())
                {
                    QStandardItem *testcaseItem = new QStandardItem(testcase->getName());
                    testcaseItem->setColumnCount(columnCount);
                    testcaseItem->setData(testcase->getTlogPath(), Qt::UserRole + 1);
                    testcaseItem->setData(testcase->getTlogStartLine(), Qt::UserRole + 2);
                    testcaseItem->setData(testcase->getTlogEndLine(), Qt::UserRole + 3);

                    QList<QStandardItem*> testcaseItems;
                    int column = 1;

                    QList<qint64> currentTestrunKeys;
                    runsMap.clear();
                    QList<QSharedPointer<Testrun> > testrunsUnsorted = testcase->getTestruns();
                    foreach (const QSharedPointer<Testrun> &testrun, testrunsUnsorted)
                    {
                        runsMap.insert(testrun->getTimestamp(), testrun);
                    }

                    currentTestrunKeys = runsMap.keys();
                    fillMissingRuns(testrunKeys, runsMap);
                    currentTestrunKeys = runsMap.keys();

                    while (not currentTestrunKeys.isEmpty())
                    {
                        qint64 runsMapKey = currentTestrunKeys.takeLast();
                        QSharedPointer<Testrun> testrun = runsMap.value(runsMapKey);
                        timestamps.insert(column, testrun->getTimestamp());

                        int failedTestrun = testrun->getFailed();
                        int skippedTestrun = testrun->getSkipped();
                        int passedTestrun = testrun->getPassed();
                        QString text = QString("%1 | %2 | %3")
                                .arg(passedTestrun).arg(failedTestrun).arg(skippedTestrun);
                        QIcon icon = iconPassed;
                        // build icon
                        if (passedTestrun >= 0 && failedTestrun >= 0 && skippedTestrun >= 0)
                        {
                            int severity = 0;
                            if (failedTestrun > 0)
                            {
                                icon = iconFailed;
                                severity = 2;
                            }
                            else if (skippedTestrun > 0)
                            {
                                icon = iconSkipped;
                                severity = 1;
                            }

                            if (passedTestrun >= 0)
                            {
                                increaseCounter(passedTestrun, column, passedProject);
                                increaseCounter(passedTestrun, column, passedLibrary);
                            }
                            if (failedTestrun >= 0)
                            {
                                increaseCounter(failedTestrun, column, failedProject);
                                increaseCounter(failedTestrun, column, failedLibrary);
                            }
                            if (skippedTestrun >= 0)
                            {
                                increaseCounter(skippedTestrun, column, skippedProject);
                                increaseCounter(skippedTestrun, column, skippedLibrary);
                            }

                            if (not iconsLibrary.contains(column))
                            {
                                iconsLibrary.insert(column, QPair<int, QIcon>(severity, icon));
                            }
                            else
                            {
                                QPair<int, QIcon> entry = iconsLibrary.value(column);
                                if (entry.first < severity)
                                {
                                    iconsLibrary.insert(column, QPair<int, QIcon>(severity, icon));
                                }
                            }
                            if (not iconsProject.contains(column))
                            {
                                iconsProject.insert(column, QPair<int, QIcon>(severity, icon));
                            }
                            else
                            {
                                QPair<int, QIcon> entry = iconsProject.value(column);
                                if (entry.first < severity)
                                {
                                    iconsProject.insert(column, QPair<int, QIcon>(severity, icon));
                                }
                            }
                        }
                        QStandardItem *testrunItem = 0;
                        if (failedTestrun + skippedTestrun + passedTestrun < 0)
                        {
                            testrunItem = new QStandardItem("n/a");
                            testrunItem->setTextAlignment(Qt::AlignCenter);
                        }
                        else
                        {
                            testrunItem = new QStandardItem(icon, text);
                            testrunItem->setTextAlignment(Qt::AlignRight);
                        }
                        if (failedTestrun > 0)
                        {
                            testrunItem->setBackground(QColor::fromRgb(240, 130, 130, 230));
                            QStringList failLogs = testrun->getFailLogs();
                            QString failLog = failLogs.join("\n");
                            testrunItem->setToolTip(failLog);
                        }
                        else if (skippedTestrun > 0)
                        {
                            testrunItem->setBackground(QColor::fromRgb(255, 221, 0, 230));
                        }
                        else if (passedTestrun > 0)
                        {
                            testrunItem->setBackground(QColor::fromRgb(130, 255, 130, 230));
                        }
                        else
                        {
                            testrunItem->setBackground(QColor::fromRgb(190, 190, 190, 230));
                        }

                        if (failedTestrun > 0)
                        {
                            QStringList failLogs = testrun->getFailLogs();
                            testrunItem->setData(failLogs.join("\n"));
                        }
                        testcaseItems << testrunItem;
                        ++column;
                    } // foreach testruns

                    testcaseItems.insert(0, testcaseItem);

                    libraryItem->appendRow(testcaseItems);
                } // foreach testcases

                appendFilledRow(projectItem, columnCount, libraryItem, iconsLibrary,
                                passedLibrary, failedLibrary, skippedLibrary);
            } // foreach libraries

            appendFilledRow(rootItem, columnCount, projectItem, iconsProject,
                            passedProject, failedProject, skippedProject);
        } // foreach projects

        foreach (int col, timestamps.keys())
        {
            qint64 timestamp = timestamps.value(col);
            QString timeLabel = QDateTime::fromMSecsSinceEpoch(timestamp).
                    toString("dd.MM. hh:mm");
            headerLabels << timeLabel;
        }
        m_branchTableModel->setHorizontalHeaderLabels(headerLabels);
        m_headerTimestamps = timestamps;

        ui->branchTestsTreeView->resizeColumnToContents(0);
    }

    updateTools();
}

void MainWindow::fillMissingRuns(const QList<qint64> testrunKeys,
                                 QMap<qint64, QSharedPointer<Testrun> > &runsMap)
{
    foreach (qint64 testrunKey, testrunKeys)
    {
        if (not runsMap.contains(testrunKey))
        {
            QSharedPointer<Testrun> testrunDummy(new Testrun());
            testrunDummy->withTimestamp(testrunKey)
                    .withResults(-1, -1, -1);
            runsMap.insert(testrunKey, testrunDummy);
        }
    }
}

void MainWindow::appendFilledRow(
        QStandardItem *parentItem, int columnCount,
        QStandardItem *firstItem, const QMap<int, QPair<int, QIcon> > &icons,
        QMap<int, int> &passed, QMap<int, int> &failed, QMap<int, int> &skipped)
{
    if (not parentItem || columnCount < 1 || not firstItem)
    {
        return;
    }

    QList<QStandardItem*> rowItems;
    rowItems << firstItem;
    firstItem->setBackground(QColor::fromRgb(171, 230, 242, 127));
    for (int c = 1; c < columnCount; ++c)
    {
        QStandardItem *columnItem = new QStandardItem("");
        QFont font = columnItem->font();
        font.setBold(true);
        columnItem->setFont(font);
        columnItem->setTextAlignment(Qt::AlignRight);
        columnItem->setBackground(QColor::fromRgb(190, 190, 190, 230));

        if (icons.contains(c))
        {
            QPair<int, QIcon> iconPair = icons.value(c);
            columnItem->setIcon(iconPair.second);

            if (passed.contains(c) && failed.contains(c) && skipped.contains(c))
            {
                QString text = QString("%1 | %2 | %3")
                        .arg(passed.value(c)).arg(failed.value(c)).arg(skipped.value(c));
                columnItem->setText(text);
                if (failed.value(c) > 0)
                {
                    columnItem->setBackground(QColor::fromRgb(240, 130, 130, 230));
                }
                else if (skipped.value(c) > 0)
                {
                    columnItem->setBackground(QColor::fromRgb(255, 221, 0, 230));
                }
                else if (passed.value(c) > 0)
                {
                    columnItem->setBackground(QColor::fromRgb(130, 255, 130, 230));
                }
            }
        }
        rowItems << columnItem;
    }

    if (rowItems.isEmpty())
    {
        return;
    }
    parentItem->appendRow(rowItems);
}

void MainWindow::increaseCounter(int increment, int column, QMap<int, int> &counters)
{
    int newValue = increment;
    if (counters.contains(column))
    {
        newValue += counters.value(column);
    }
    counters.insert(column, newValue);
}

void MainWindow::updateBranchTabs()
{
    m_selectedBranch.clear();
    QString selectedItem;
    QLayoutItem* item;
    while ((item = ui->branchTabsBarLayout->takeAt(0)))
    {
        QWidget* itemWidget = item->widget();
        QPushButton* tabButton = dynamic_cast<QPushButton*>(itemWidget);
        if (tabButton)
        {
            m_branchTabsSignalMapper.removeMappings(tabButton);
            if (tabButton->isChecked())
            {
                selectedItem = tabButton->text();
            }
        }
        delete itemWidget;
        delete item;
    }

    QList<QSharedPointer<Branch> > branches = m_monitorSet->getBranches();
    foreach (const QSharedPointer<Branch> &branch, branches)
    {
        QString branchName = branch->getName();
        QPushButton* tabButton = new QPushButton(branchName, ui->branchTabsBar);
        tabButton->setCheckable(true);
        connect(tabButton, SIGNAL(clicked()), &m_branchTabsSignalMapper, SLOT(map()));
        m_branchTabsSignalMapper.setMapping(tabButton, branchName);
        if (not selectedItem.isNull() && branchName == selectedItem)
        {
            tabButton->setChecked(true);
            m_selectedBranch = branch;
        }
        ui->branchTabsBarLayout->addWidget(tabButton);
    }
}

void MainWindow::branchTabClicked(const QString &text)
{
    if (not m_selectedBranch.isNull() && m_selectedBranch->getName() == text)
    {
        return; // branch is already selected
    }
    QLayoutItem* item;
    m_selectedBranch.clear();
    int tabs = ui->branchTabsBarLayout->count();
    for (int i = 0; i < tabs; ++i)
    {
        item = ui->branchTabsBarLayout->itemAt(i);

        QWidget* itemWidget = item->widget();
        QPushButton* tabButton = dynamic_cast<QPushButton*>(itemWidget);
        if (tabButton)
        {
            if (tabButton->text() == text)
            {
                tabButton->setChecked(true);
                QList<QSharedPointer<Branch> > branches = m_monitorSet->getBranches();
                foreach (const QSharedPointer<Branch> &branch, branches)
                {
                    if (branch->getName() == text)
                    {
                        m_selectedBranch = branch;
                        initializeBranchTableModel();
                    }
                }
            }
            else
            {
                tabButton->setChecked(false);
            }
        }
    }
}

void MainWindow::pushRecentMonitorSetFile(const QString &recentFile)
{
    if (recentFile.isNull() || recentFile.isEmpty())
    {
        return;
    }

    m_recentMonitorSetFilesMutex.lock();

    if (m_recentMonitorSetFiles.contains(recentFile))
    {
        m_recentMonitorSetFiles.removeAll(recentFile);
    }
    m_recentMonitorSetFiles.insert(0, recentFile);
    if (m_recentMonitorSetFiles.size() > 5)
    {
        m_recentMonitorSetFiles.removeLast();
    }
    QSettings settings;
    settings.beginWriteArray("recentFiles");
    for (int i = 0, max = m_recentMonitorSetFiles.size(); i < max; ++i)
    {
        settings.setArrayIndex(i);
        settings.setValue("recentFile", m_recentMonitorSetFiles.at(i));
    }
    settings.endArray();
    settings.sync();

    m_recentMonitorSetFilesMutex.unlock();

    updateRecentFilesMenu();
}

void MainWindow::openRecentMonitorSetFile()
{
    if (not m_ioBlocked.testAndSetAcquire(0, 1))
    {
        ui->statusBar->showMessage(
                    tr("MainWindow: Failed to open recent file as I/O operation is pending."),
                    5000);
        return;
    }
    enableIOActions(false);
    QAction* action = qobject_cast<QAction*>(sender());
    if (action)
    {
        QString fileName = action->data().toString();

        openMonitorSetFile(fileName);
    }
}

void MainWindow::itemClicked(QModelIndex index)
{
    m_selectedLibrary = 0;
    m_selectedTestcase = 0;
    m_selectedTestrun = -1;
    if (index.isValid())
    {
        int clickColumn = index.column();
        if (clickColumn > 0 && m_headerTimestamps.contains(clickColumn))
        {

            m_selectedTestrun = m_headerTimestamps.value(clickColumn);
        }
        QModelIndex firstColumnOfClick = index;
        if (index.column() > 0)
        {
            firstColumnOfClick = index.sibling(index.row(), 0);
        }
        QStandardItem* item = m_branchTableModel->itemFromIndex(firstColumnOfClick);
        if (not firstColumnOfClick.child(0, 0).isValid())
        {
            // is testcase row
            m_selectedTestcase = item;
        }
        else if (not firstColumnOfClick.child(0, 0).child(0, 0).isValid())
        {
            // is library row
            m_selectedLibrary = item;
        }
    }
    updateTools();
}

void MainWindow::itemDoubleClicked(QModelIndex index)
{
    itemClicked(index);
    if (m_selectedLibrary)
    {
        on_viewLcovToolButton_clicked();
    }
    else if (m_selectedTestcase)
    {
        on_viewTlogToolButton_clicked();
    }
}
