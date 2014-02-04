/**
  * @file MonitorSetReader.cpp
  *
  * @class MonitorSetReader
  *
  * @copyright (c) 2014, Robert Wloch
  * @license LGPL v2.1
  *
  * @brief Reads utm files and creates a monitor set model
  * @details The MonitorSetReader parses a utm file and instanciates a MonitorSet model.
  *
  * @author Robert Wloch, robert@rowlo.de
  *************************************************************************************************/
#include "MonitorSetReader.h"

#include <QSharedPointer>
#include <QFile>
#include <QFileInfo>
#include <QXmlStreamReader>
#include <QDebug>

using Model::MonitorSet;
using Model::Branch;
using Model::Project;
using Model::Library;
using Model::Testcase;
using Model::Testrun;

MonitorSetReader::MonitorSetReader(const QString &fileName)
    : m_fileName(fileName)
{
}

QSharedPointer<MonitorSet> MonitorSetReader::read()
{
    QSharedPointer<MonitorSet> result(new MonitorSet());
    QFileInfo fileInfo(m_fileName);
    QFile file(m_fileName);
    if (not fileInfo.exists() || not fileInfo.isFile() || not m_fileName.endsWith(".utm")
            || not file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Cannot to read from file: " << m_fileName;
        return result;
    }

    QXmlStreamReader stream(&file);
    while (not stream.atEnd())
    {
        stream.readNext();
        if (stream.isStartDocument() && stream.documentVersion().compare("1.0") != 0)
        {
            qDebug() << "Expected file version 1.0 instead of " << stream.documentVersion();
            break;
        }

        if (stream.isStartElement() && stream.name() == "utm")
        {
            readBranches(&stream, result);
        }
    }
    if (stream.hasError())
    {
        qDebug() << "Failed to read from file: " << m_fileName;
    }
    return result;
}

void MonitorSetReader::readBranches(QXmlStreamReader* stream, QSharedPointer<MonitorSet> result)
{
    while (not stream->atEnd())
    {
        stream->readNext();
        if (stream->isEndElement() && stream->name() == "utm")
        {
            break;
        }
        if (stream->isStartElement() && stream->name() == "branch")
        {
            QString name;
            QString path;
            if (not readAttribute(stream, "name", name) || not readAttribute(stream, "path", path))
            {
                break;
            }

            QSharedPointer<Branch> branch(new Branch());
            branch->withName(name).withPath(path);
            result->addBranch(branch);

            readProjects(stream, branch);
        }
    }
}

void MonitorSetReader::readProjects(QXmlStreamReader* stream, QSharedPointer<Branch> result)
{
    while (not stream->atEnd())
    {
        stream->readNext();
        if (stream->isEndElement() && stream->name() == "branch")
        {
            break;
        }
        if (stream->isStartElement() && stream->name() == "project")
        {
            QString name;
            QString path;
            if (not readAttribute(stream, "name", name) || not readAttribute(stream, "path", path))
            {
                break;
            }

            QSharedPointer<Project> project(new Project());
            project->withName(name).withPath(path);
            result->addProject(project);

            readLibraries(stream, project);
        }
    }
}

void MonitorSetReader::readLibraries(QXmlStreamReader* stream, QSharedPointer<Project> result)
{
    while (not stream->atEnd())
    {
        stream->readNext();
        if (stream->isEndElement() && stream->name() == "project")
        {
            break;
        }
        if (stream->isStartElement() && stream->name() == "library")
        {
            QString name;
            QString path;
            QString lcovPath;
            if (not readAttribute(stream, "name", name) || not readAttribute(stream, "path", path)
                    || not readAttribute(stream, "lcovPath", lcovPath))
            {
                break;
            }

            QSharedPointer<Library> library(new Library());
            library->withName(name).withPath(path).withLcovPath(lcovPath);
            result->addLibrary(library);

            readTestcases(stream, library);
        }
    }
}

void MonitorSetReader::readTestcases(QXmlStreamReader* stream, QSharedPointer<Library> result)
{
    while (not stream->atEnd())
    {
        stream->readNext();
        if (stream->isEndElement() && stream->name() == "library")
        {
            break;
        }
        if (stream->isStartElement() && stream->name() == "testcase")
        {
            QString name;
            QString tlogPath;
            QString tlogStartLine;
            QString tlogEndLine;
            int startLine = 0, endLine = 0;
            if (not readAttribute(stream, "name", name) ||
                    not readAttribute(stream, "tlogPath", tlogPath) ||
                    not readAttribute(stream, "tlogStartLine", tlogStartLine) ||
                    not readAttribute(stream, "tlogEndLine", tlogEndLine))
            {
                break;
            }
            startLine = tlogStartLine.toInt();
            endLine = tlogEndLine.toInt();

            QSharedPointer<Testcase> testcase(new Testcase());
            testcase->withName(name).withTlogPath(tlogPath, startLine, endLine);
            // RWL: used to add testcase always
            //result->addTestcase(testcase);

            readTestruns(stream, testcase);
            // RWL: now, add only testcases with test runs
            if (testcase->getTestrunsCount() > 0)
            {
                result->addTestcase(testcase);
            }
        }
    }
}

void MonitorSetReader::readTestruns(QXmlStreamReader* stream, QSharedPointer<Testcase> result)
{
    QSharedPointer<Testrun> testrun;
    while (not stream->atEnd())
    {
        stream->readNext();
        if (stream->isEndElement() && stream->name() == "testcase")
        {
            break;
        }
        if (stream->isStartElement() && stream->name() == "testrun")
        {
            QString passedString, failedString, skippedString, timestampString;
            qint32 passed = 0, failed = 0, skipped = 0;
            qint64 timestamp = 0;
            if (not readAttribute(stream, "passed", passedString) ||
                    not readAttribute(stream, "failed", failedString) ||
                    not readAttribute(stream, "skipped", skippedString) ||
                    not readAttribute(stream, "timestamp", timestampString))
            {
                break;
            }
            passed = passedString.toInt();
            skipped = skippedString.toInt();
            failed = failedString.toInt();
            timestamp = timestampString.toLong();

            testrun = QSharedPointer<Testrun>(new Testrun());
            testrun->withTimestamp(timestamp).
                    withResults(passed, failed, skipped);
        }
        if (stream->isStartElement() && stream->name() == "failLog" && not testrun.isNull())
        {
            QString failLog = stream->readElementText();
            if (not stream->hasError())
            {
                testrun->withFailLog(failLog);
            }
        }

        if (stream->isEndElement() && stream->name() == "testrun" && not testrun.isNull())
        {
            result->addTestrun(testrun);
        }
    }
}

bool MonitorSetReader::readAttribute(
        QXmlStreamReader* stream, const QString &attributeName, QString &value)
{
    if (stream && not attributeName.isNull() && not attributeName.isEmpty())
    {
        QXmlStreamAttributes attributes = stream->attributes();
        if (attributes.hasAttribute(attributeName))
        {
            value = attributes.value(attributeName).toString();
            return true;
        }
    }
    return false;
}
