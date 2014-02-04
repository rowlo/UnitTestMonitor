/**
  * @file MonitorSetReader.h
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
#ifndef MONITORSETREADER_H
#define MONITORSETREADER_H

#include <QString>
#include <QSharedPointer>
#include <Model/MonitorSet.h>

class QXmlStreamReader;

class MonitorSetReader
{
public:
    MonitorSetReader(const QString &fileName);
    QSharedPointer<Model::MonitorSet> read();
protected:
    void readBranches(QXmlStreamReader* stream, QSharedPointer<Model::MonitorSet> result);
    void readProjects(QXmlStreamReader* stream, QSharedPointer<Model::Branch> result);
    void readLibraries(QXmlStreamReader* stream, QSharedPointer<Model::Project> result);
    void readTestcases(QXmlStreamReader* stream, QSharedPointer<Model::Library> result);
    void readTestruns(QXmlStreamReader* stream, QSharedPointer<Model::Testcase> result);
    bool readAttribute(QXmlStreamReader* stream, const QString &attributeName, QString &value);
private:
    QString m_fileName;
};

#endif // MONITORSETREADER_H
