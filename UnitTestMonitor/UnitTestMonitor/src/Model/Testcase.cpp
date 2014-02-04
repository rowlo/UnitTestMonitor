/**
  * @file Testcase.cpp
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
#include "Model/Testcase.h"

namespace Model
{

Testcase::Testcase()
{
}

Testcase::Testcase(const Testcase &other)
    : m_tlogPath(other.m_tlogPath),
      m_name(other.m_name),
      m_tlogStartLine(other.m_tlogStartLine),
      m_tlogEndLine(other.m_tlogEndLine),
      m_testruns(other.m_testruns)
{
}

Testcase& Testcase::withTlogPath(const QString &tlogPath, const int startLine, const int endLine)
{
    m_tlogPath = tlogPath;
    m_tlogStartLine = startLine;
    m_tlogEndLine = endLine;
    return *this;
}

QString Testcase::getTlogPath() const
{
    return m_tlogPath;
}

int Testcase::getTlogStartLine() const
{
    return m_tlogStartLine;
}

int Testcase::getTlogEndLine() const
{
    return m_tlogEndLine;
}

Testcase& Testcase::withName(const QString &name)
{
    m_name = name;
    return *this;
}

QString Testcase::getName() const
{
    return m_name;
}

QSharedPointer<Testrun> Testcase::getTestrun(qint64 timestamp)
{
    QSharedPointer<Testrun> result;
    if (m_testruns.contains(timestamp))
    {
        result = m_testruns.value(timestamp);
    }
    return result;
}

void Testcase::deleteTestrun(qint64 timestamp)
{
    if (timestamp >= 0 && m_testruns.contains(timestamp))
    {
        m_testruns.remove(timestamp);
    }
}

void Testcase::addTestrun(QSharedPointer<Testrun> testrun)
{
    if (not testrun.isNull())
    {
        qint64 timestamp = testrun->getTimestamp();
        if (timestamp > 0)
        {
            m_testruns.insert(timestamp, testrun);
        }
    }
}

QList<QSharedPointer<Testrun> > Testcase::getTestruns() const
{
    return m_testruns.values();
}

int Testcase::getTestrunsCount() const
{
    return m_testruns.size();
}

} // namespace Model
