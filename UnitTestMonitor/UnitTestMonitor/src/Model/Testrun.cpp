/**
  * @file Testrun.cpp
  *
  * @class Model::Testrun
  *
  * @copyright (c) 2014, Robert Wloch
  * @license LGPL v2.1
  *
  * @brief Model element representing a testrun of a testcase.
  * @details A testrun is a quadrupel of timestamp, passed, failed, and skipped tests.
  *
  * @author Robert Wloch, robert@rowlo.de
  *************************************************************************************************/
#include "Model/Testrun.h"

namespace Model
{

Testrun::Testrun()
{
}

Testrun::Testrun(const Testrun &other)
    : m_timestamp(other.m_timestamp),
      m_passed(other.m_passed),
      m_failed(other.m_failed),
      m_skipped(other.m_skipped),
      m_failLogs(other.m_failLogs)
{
}

Testrun& Testrun::withTimestamp(const qint64 timestamp)
{
    m_timestamp = timestamp;
    return *this;
}

Testrun& Testrun::withResults(const qint32 passed, const qint32 failed, const qint32 skipped)
{
    m_passed = passed;
    m_failed = failed;
    m_skipped = skipped;
    return *this;
}

Testrun& Testrun::withFailLog(const QString &failLog)
{
    if (not m_failLogs.contains(failLog))
    {
        m_failLogs.append(failLog);
    }
    return *this;
}

qint64 Testrun::getTimestamp() const
{
    return m_timestamp;
}

qint32 Testrun::getPassed() const
{
    return m_passed;
}

qint32 Testrun::getFailed() const
{
    return m_failed;
}

qint32 Testrun::getSkipped() const
{
    return m_skipped;
}

QList<QString> Testrun::getFailLogs() const
{
    return m_failLogs;
}

} // namespace Model
