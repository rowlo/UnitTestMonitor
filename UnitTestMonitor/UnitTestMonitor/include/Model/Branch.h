/**
  * @file Branch.h
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
#ifndef BRANCH_H
#define BRANCH_H

#include <QString>
#include <QSharedPointer>
#include <QMap>
#include <Model/Project.h>

namespace Model
{

class Branch
{
public:
    Branch();
    Branch(const Branch &other);
    Branch& withPath(const QString &path);
    Branch& withName(const QString &name);
    QString getPath() const;
    QString getName() const;
    QSharedPointer<Project> getProject(const QString &name) const;
    void addProject(QSharedPointer<Project> project);
    QList<QSharedPointer<Project> > getProjects() const;
    int getProjectsCount() const;
private:
    QString m_path;
    QString m_name;
    QMap<QString, QSharedPointer<Project> > m_projects;
};

} // namespace Model

#endif // BRANCH_H
