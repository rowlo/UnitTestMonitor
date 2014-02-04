/**
  * @file Testrun.h
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
#ifndef TESTRUN_H
#define TESTRUN_H

#include <QList>
#include <QString>

namespace Model
{

class Testrun
{
public:
    Testrun();
    Testrun(const Testrun &other);
    Testrun& withTimestamp(const qint64 timestamp);
    Testrun& withResults(const qint32 passed, const qint32 failed, const qint32 skipped);
    Testrun& withFailLog(const QString &failLog);
    qint64 getTimestamp() const;
    qint32 getPassed() const;
    qint32 getFailed() const;
    qint32 getSkipped() const;
    QList<QString> getFailLogs() const;
private:
    qint64 m_timestamp;
    qint32 m_passed;
    qint32 m_failed;
    qint32 m_skipped;
    QList<QString> m_failLogs;
};

} // namespace Model

#endif // TESTRUN_H
