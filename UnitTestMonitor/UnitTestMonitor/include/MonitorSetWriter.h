/**
  * @file MonitorSetWriter.h
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
#ifndef MONITORSETWRITER_H
#define MONITORSETWRITER_H

#include <QString>
#include <QSharedPointer>
#include <Model/MonitorSet.h>
#include <Model/Branch.h>
#include <Model/Project.h>
#include <Model/Library.h>
#include <Model/Testcase.h>
#include <Model/Testrun.h>

class QXmlStreamWriter;

class MonitorSetWriter
{
public:
    MonitorSetWriter(const QString &fileName);
    bool write(const QSharedPointer<Model::MonitorSet> &monitorSet);
protected:
    void writeBranches(QXmlStreamWriter* writer, QList<QSharedPointer<Model::Branch> > branches);
    void writeProjects(QXmlStreamWriter* writer, QList<QSharedPointer<Model::Project> > projects);
    void writeLibraries(QXmlStreamWriter* writer, QList<QSharedPointer<Model::Library> > libraries);
    void writeTestcases(
            QXmlStreamWriter* writer, QList<QSharedPointer<Model::Testcase> > testcases);
    void writeTestruns(QXmlStreamWriter* writer, QList<QSharedPointer<Model::Testrun> > testruns);
private:
    QString m_fileName;
};

#endif // MONITORSETWRITER_H
