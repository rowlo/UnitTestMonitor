/**
  * @file MonitorSetWriter.cpp
  *
  * @class MonitorSetWriter
  *
  * @copyright (c) 2014, Robert Wloch
  * @license LGPL v2.1
  *
  * @brief Writes utm files from a monitor set model
  * @details The MonitorSetWriter creates an utm file from a MonitorSet model.
  *
  * @author Robert Wloch, robert@rowlo.de
  *************************************************************************************************/
#include "MonitorSetWriter.h"

#include <QSharedPointer>
#include <QFile>
#include <QFileInfo>
#include <QXmlStreamWriter>
#include <QDebug>

using Model::MonitorSet;
using Model::Branch;
using Model::Project;
using Model::Library;
using Model::Testcase;
using Model::Testrun;

MonitorSetWriter::MonitorSetWriter(const QString &fileName)
    : m_fileName(fileName)
{
}

bool MonitorSetWriter::write(const QSharedPointer<MonitorSet> &monitorSet)
{
    Q_UNUSED(monitorSet)
    QFile file(m_fileName);
    if (not file.open(QIODevice::WriteOnly) || not m_fileName.endsWith(".utm"))
    {
        qDebug() << "Cannot write to file: " << m_fileName;
        return false;
    }

    QXmlStreamWriter stream(&file);
    stream.setAutoFormatting(true);
    stream.writeStartDocument("1.0");

    stream.writeStartElement("utm");
    writeBranches(&stream, monitorSet->getBranches());
    stream.writeEndElement(); // utm
    stream.writeEndDocument();

    file.close();
    return true;
}

void MonitorSetWriter::writeBranches(
        QXmlStreamWriter *writer, QList<QSharedPointer<Branch> > branches)
{
    if (not writer)
    {
        return;
    }
    foreach (const QSharedPointer<Branch> &branch, branches)
    {
        QString path = branch->getPath();
        QString name = branch->getName();
        if (path.isNull() || path.isEmpty())
        {
            continue;
        }
        if (name.isNull() || name.isEmpty())
        {
            name = "";
        }
        writer->writeStartElement("branch");
        writer->writeAttribute("name", name);
        writer->writeAttribute("path", path);
        writeProjects(writer, branch->getProjects());
        writer->writeEndElement(); // branch
    }
}

void MonitorSetWriter::writeProjects(
        QXmlStreamWriter *writer, QList<QSharedPointer<Project> > projects)
{
    if (not writer)
    {
        return;
    }
    foreach (const QSharedPointer<Project> &project, projects)
    {
        QString path = project->getPath();
        QString name = project->getName();
        if (path.isNull() || path.isEmpty())
        {
            continue;
        }
        if (name.isNull() || name.isEmpty())
        {
            name = "";
        }
        writer->writeStartElement("project");
        writer->writeAttribute("name", name);
        writer->writeAttribute("path", path);
        writeLibraries(writer, project->getLibraries());
        writer->writeEndElement(); // project
    }
}

void MonitorSetWriter::writeLibraries(
        QXmlStreamWriter *writer, QList<QSharedPointer<Library> > libraries)
{
    if (not writer)
    {
        return;
    }
    foreach (const QSharedPointer<Library> &library, libraries)
    {
        QString path = library->getPath();
        QString name = library->getName();
        QString lcovPath = library->getLcovPath();
        if (path.isNull() || path.isEmpty() || lcovPath.isNull() || lcovPath.isEmpty())
        {
            continue;
        }
        if (name.isNull() || name.isEmpty())
        {
            name = "";
        }
        writer->writeStartElement("library");
        writer->writeAttribute("name", name);
        writer->writeAttribute("path", path);
        writer->writeAttribute("lcovPath", lcovPath);
        writeTestcases(writer, library->getTestcases());
        writer->writeEndElement(); // library
    }
}

void MonitorSetWriter::writeTestcases(
        QXmlStreamWriter *writer, QList<QSharedPointer<Testcase> > testcases)
{
    if (not writer)
    {
        return;
    }
    foreach (const QSharedPointer<Testcase> &testcase, testcases)
    {
        if (testcase->getTestrunsCount() == 0)
        {
            // RWL: don't write testcase without any testruns
            continue;
        }
        QString path = testcase->getTlogPath();
        int startLine = testcase->getTlogStartLine();
        int endLine = testcase->getTlogEndLine();
        QString name = testcase->getName();
        if (path.isNull() || path.isEmpty())
        {
            continue;
        }
        if (name.isNull() || name.isEmpty())
        {
            name = "";
        }
        writer->writeStartElement("testcase");
        writer->writeAttribute("name", name);
        writer->writeAttribute("tlogPath", path);
        writer->writeAttribute("tlogStartLine", QString("%1").arg(startLine));
        writer->writeAttribute("tlogEndLine", QString("%1").arg(endLine));
        writeTestruns(writer, testcase->getTestruns());
        writer->writeEndElement(); // testcase
    }
}

void MonitorSetWriter::writeTestruns(
        QXmlStreamWriter *writer, QList<QSharedPointer<Testrun> > testruns)
{
    if (not writer)
    {
        return;
    }
    foreach (const QSharedPointer<Testrun> &testrun, testruns)
    {
        qint32 passed = testrun->getPassed();
        qint32 failed = testrun->getFailed();
        qint32 skipped = testrun->getSkipped();
        qint64 timestamp = testrun->getTimestamp();
        QList<QString> failLogs = testrun->getFailLogs();
        writer->writeStartElement("testrun");
        writer->writeAttribute("passed", QString("%1").arg(passed));
        writer->writeAttribute("failed", QString("%1").arg(failed));
        writer->writeAttribute("skipped", QString("%1").arg(skipped));
        writer->writeAttribute("timestamp", QString("%1").arg(timestamp));
        if (failLogs.size() > 0)
        {
            foreach (const QString &failLog, failLogs)
            {
                writer->writeTextElement("failLog", failLog);
            }
        }
        writer->writeEndElement(); // testrun
    }
}
