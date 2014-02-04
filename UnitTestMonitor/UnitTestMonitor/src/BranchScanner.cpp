/**
  * @file BranchScanner.cpp
  *
  * @class BranchScanner
  *
  * @copyright (c) 2014, Robert Wloch
  * @license LGPL v2.1
  *
  * @brief Scans a branch directory for unit test data
  * @details The branch scanner scans a directory structure for libraries with tests, tlog, and lcov
  *          reports.
  *
  * @author Robert Wloch, robert@rowlo.de
  *************************************************************************************************/
#include "BranchScanner.h"

#include <QFileInfo>
#include <QFile>
#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QTextStream>

#include <Model/Project.h>
#include <Model/Testcase.h>
#include <Model/Testrun.h>

using Model::Branch;
using Model::Project;
using Model::Library;
using Model::Testcase;
using Model::Testrun;

BranchScanner::BranchScanner()
{
}

QSharedPointer<Branch> BranchScanner::scanBranch(const QString &path)
{
    QFileInfo fileInfo(path);
    if (not (fileInfo.isDir() && fileInfo.exists()))
    {
        return QSharedPointer<Branch>();
    }

    QSharedPointer<Branch> result(new Branch());

    QDir parentDir(path);
    parentDir.cdUp();
    QFileInfo parentFileInfo(parentDir.absolutePath());
    result->withPath(path)
            .withName(QString("%1/%2").arg(parentFileInfo.fileName()).arg(fileInfo.fileName()));

    return updateBranch(result);
}

QSharedPointer<Branch> BranchScanner::updateBranch(const QSharedPointer<Branch> &branch)
{
    QSharedPointer<Branch> result;
    if (branch.isNull())
    {
        return result;
    }
    result = branch;

    QString path = result->getPath();
    QDir branchDir(path);
    QFileInfoList branchEntries = branchDir.entryInfoList(QDir::Dirs);

    qint64 scanTimestamp = QDateTime::currentMSecsSinceEpoch();

    foreach (const QFileInfo &branchEntry, branchEntries)
    {
        QString projectPath = branchEntry.absoluteFilePath();
        QString projectName = branchEntry.fileName();
        if (projectName == "." || projectName == ".." || projectName == "_")
        {
            continue;
        }
        QSharedPointer<Project> project = result->getProject(projectName);

        QDir projectDir(projectPath);
        QFileInfoList projectEntries = projectDir.entryInfoList(QDir::Dirs);
        foreach (const QFileInfo &projectEntry, projectEntries)
        {
            QSharedPointer<Library> library;
            QString libraryName = projectEntry.fileName();
            if (libraryName.endsWith("Test") || libraryName == "." || libraryName == ".."
                    || libraryName == "_")
            {
                continue;
            }
            if (not project.isNull())
            {
                library = project->getLibrary(libraryName);
            }
            QString libraryPath = projectEntry.absoluteFilePath();

            QString tlogFilePath = QString("%1/_/tests/%2/tlog").arg(projectPath).arg(libraryName);
            QString lcovFilePath =
                    QString("%1/_/testcoverage/%2/index.html").arg(projectPath).arg(libraryName);

            QFileInfo libraryUnitTestTlogFileInfo(tlogFilePath);
            QFileInfo libraryUnitTestLcovFileInfo(lcovFilePath);
            if (libraryUnitTestTlogFileInfo.isFile() && libraryUnitTestTlogFileInfo.exists()&&
                    libraryUnitTestLcovFileInfo.isFile() && libraryUnitTestLcovFileInfo.exists())
            {
                if (project.isNull())
                {
                    project = QSharedPointer<Project>(new Project());
                    project->withName(projectName).withPath(projectPath);
                    result->addProject(project);
                }
                if (library.isNull())
                {
                    library = QSharedPointer<Library>(new Library());
                    library->withName(libraryName).withPath(libraryPath).withLcovPath(lcovFilePath);
                    project->addLibrary(library);
                }
                analyzeTlog(tlogFilePath, library, scanTimestamp);
            }
        }
    }

    return result;
}

void BranchScanner::analyzeTlog(const QString &tlogFilePath,
                                const QSharedPointer<Library> &library,
                                qint64 timestamp)
{
    QFileInfo tlogFileInfo(tlogFilePath);
    if (library.isNull() || not tlogFileInfo.exists())
    {
        return;
    }

    QFile tlogFile(tlogFilePath);
    if (not tlogFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    QString patternTestCaseBeginStartOfLine("********* Start ");
    QString patternTestCaseBeginEndOfLine(" *********");
    QString patternTestCaseEndStartOfLine("********* Finished testing of ");
    QString patternTestCaseEndEndOfLine(" *********");
    // "Totals: 9 passed, 0 failed, 0 skipped"
    QString patternTestCaseSummaryStartOfLine("Totals: ");
    QString patternTestCaseSummaryPassed(" passed");
    QString patternTestCaseSummaryFailed(" failed");
    QString patternTestCaseSummarySkipped(" skipped");
    // Fails
    QString patternTestCaseFailBeginStartOfLine("FAIL!");
    QString patternTestCaseFailEndStartOfLine("   Loc:");

    QSharedPointer<Testcase> testcase;
    QTextStream in(&tlogFile);
    int lineNumber = 0, testcaseStartLine = 0, testcaseEndLine;
    QStringList failLog;
    bool inFailLogOutput = false;
    while (not in.atEnd())
    {
        QString line = in.readLine();
        ++lineNumber;

        if (line.startsWith(patternTestCaseBeginStartOfLine) &&
                line.endsWith(patternTestCaseBeginEndOfLine))
        {
            testcaseStartLine = lineNumber - 1;
            failLog.clear();
            QString testcaseName = line.replace(patternTestCaseBeginStartOfLine, "").
                    replace(patternTestCaseBeginEndOfLine, "")
                    .split("::").last();
            testcase = library->getTestcase(testcaseName);
            if (testcase.isNull())
            {
                testcase = QSharedPointer<Testcase>(new Testcase());
                testcase->withName(testcaseName);
                library->addTestcase(testcase);
            }
        }

        if (line.startsWith(patternTestCaseFailBeginStartOfLine))
        {
            inFailLogOutput = true;
        }
        if (inFailLogOutput)
        {
            failLog << line;
            if (line.startsWith(patternTestCaseFailEndStartOfLine))
            {
                inFailLogOutput = false;
            }
        }

        if (line.startsWith(patternTestCaseSummaryStartOfLine))
        {
            QString keyNumbers = line.replace(patternTestCaseSummaryStartOfLine, "")
                    .replace(patternTestCaseSummaryPassed, "")
                    .replace(patternTestCaseSummaryFailed, "")
                    .replace(patternTestCaseSummarySkipped, "");

            QStringList numbers = keyNumbers.split(",");
            qint32 passed = 0, failed = 0, skipped = 0;
            if (numbers.size() == 3)
            {
                passed = numbers.at(0).trimmed().toInt();
                failed = numbers.at(1).trimmed().toInt();
                skipped = numbers.at(2).trimmed().toInt();
            }
            if (testcase->getTestrun(timestamp).isNull())
            {
                            QString failLogAsString("");
                            if (failed > 0)
                            {
                                failLogAsString = failLog.join("\n");
                            }

                            QSharedPointer<Testrun> testrun(new Testrun());
                            testrun->withFailLog(failLogAsString)
                                    .withResults(passed, failed, skipped)
                                    .withTimestamp(timestamp);
                            testcase->addTestrun(testrun);
            }
        }

        if (line.startsWith(patternTestCaseEndStartOfLine) &&
                line.endsWith(patternTestCaseEndEndOfLine))
        {
            testcaseEndLine = lineNumber - 1;
            testcase->withTlogPath(tlogFilePath, testcaseStartLine, testcaseEndLine);
        }
    }
}
