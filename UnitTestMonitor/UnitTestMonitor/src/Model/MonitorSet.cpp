/**
  * @file MonitorSet.cpp
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
#include "Model/MonitorSet.h"

#include <QUuid>
#include <QDir>

namespace Model
{

MonitorSet::MonitorSet()
    : m_id(QUuid::createUuid().toString())
{
}

MonitorSet::MonitorSet(const MonitorSet &other)
    : m_id(other.m_id)
{
}

QString MonitorSet::getId() const
{
    return m_id;
}

void MonitorSet::addBranch(QSharedPointer<Branch> branch)
{
    if (not branch.isNull())
    {
        QString path = branch->getPath();
        QDir dir(path);
        if (dir.exists())
        {
            m_branches.insert(path, branch);
        }
    }
}

void MonitorSet::removeBranch(QSharedPointer<Branch> branch)
{
    if (not branch.isNull())
    {
        QString path = branch->getPath();
        if (m_branches.contains(path))
        {
            m_branches.remove(path);
        }
    }
}

QList<QSharedPointer<Branch> > MonitorSet::getBranches() const
{
    return m_branches.values();
}

int MonitorSet::getBranchesCount() const
{
    return m_branches.size();
}

} // namespace Model
