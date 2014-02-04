/**
  * @file MonitorSet.h
  *
  * @class Model::MonitorSet
  *
  * @copyright (c) 2014, Robert Wloch
  * @license LGPL v2.1
  *
  * @brief Root model element of monitor set.
  * @details A monitor set is a collection of branch locations.
  *
  * @author Robert Wloch, robert@rowlo.de
  *************************************************************************************************/
#ifndef MONITORSET_H
#define MONITORSET_H

#include <QString>
#include <QMap>
#include <QList>
#include <QSharedPointer>
#include <Model/Branch.h>

namespace Model
{

class MonitorSet
{
public:
    MonitorSet();
    MonitorSet(const MonitorSet &other);
    QString getId() const;
    void addBranch(QSharedPointer<Branch> branch);
    void removeBranch(QSharedPointer<Branch> branch);
    QList<QSharedPointer<Branch> > getBranches() const;
    int getBranchesCount() const;
private:
    QString m_id;
    QMap<QString, QSharedPointer<Branch> > m_branches;
};

} // namespace Model

#endif // MONITORSET_H
