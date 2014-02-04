/**
  * @file Branch.cpp
  *
  * @class Model::Branch
  *
  * @copyright (c) 2014, Robert Wloch
  * @license LGPL v2.1
  *
  * @brief Model element representing a branch directory.
  * @details A branch is a collection of projects
  *
  * @author Robert Wloch, robert@rowlo.de
  *************************************************************************************************/
#include "Model/Branch.h"

#include <QDir>

namespace Model
{

Branch::Branch()
{
}

Branch::Branch(const Branch &other)
    : m_path(other.m_path),
      m_name(other.m_name),
      m_projects(other.m_projects)
{
}

Branch& Branch::withPath(const QString &path)
{
    m_path = path;
    return *this;
}

Branch& Branch::withName(const QString &name)
{
    m_name = name;
    return *this;
}

QString Branch::getPath() const
{
    return m_path;
}

QString Branch::getName() const
{
    return m_name;
}

QSharedPointer<Project> Branch::getProject(const QString &name) const
{
    QSharedPointer<Project> result;
    if (m_projects.contains(name))
    {
        result = m_projects.value(name);
    }
    return result;
}

void Branch::addProject(QSharedPointer<Project> project)
{
    if (not project.isNull())
    {
        QString folderName = project->getName();
        QDir dir(m_path);
        QString folderPath = QDir::cleanPath(dir.absoluteFilePath(folderName));
        QDir projectDir(folderPath);
        if (projectDir.exists())
        {
            project->withPath(folderPath);
            m_projects.insert(folderName, project);
        }
    }
}

QList<QSharedPointer<Project> > Branch::getProjects() const
{
    return m_projects.values();
}

int Branch::getProjectsCount() const
{
    return m_projects.size();
}

} // namespace Model
