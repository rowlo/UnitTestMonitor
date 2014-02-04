/**
  * @file Testcase.h
  *
  * @class Model::Project
  *
  * @copyright (c) 2014, Robert Wloch
  * @license LGPL v2.1
  *
  * @brief Model element representing a testcase class of a library.
  * @details A testcase has a collection of testruns.
  *
  * @author Robert Wloch, robert@rowlo.de
  *************************************************************************************************/
#ifndef TESTCASE_H
#define TESTCASE_H

#include <QString>
#include <QSharedPointer>
#include <QMap>
#include <Model/Testrun.h>

namespace Model
{

class Testcase
{
public:
    Testcase();
    Testcase(const Testcase &other);
    Testcase& withTlogPath(const QString &tlogPath, const int startLine, const int endLine);
    QString getTlogPath() const;
    Testcase& withName(const QString &name);
    QString getName() const;
    int getTlogStartLine() const;
    int getTlogEndLine() const;
    void addTestrun(QSharedPointer<Testrun> testrun);
    QSharedPointer<Testrun> getTestrun(qint64 timestamp);
    void deleteTestrun(qint64 timestamp);
    QList<QSharedPointer<Testrun> > getTestruns() const;
    int getTestrunsCount() const;
private:
    QString m_tlogPath;
    QString m_name;
    int m_tlogStartLine;
    int m_tlogEndLine;
    QMap<qint64, QSharedPointer<Testrun> > m_testruns;
};

} // namespace Model

#endif // TESTCASE_H
