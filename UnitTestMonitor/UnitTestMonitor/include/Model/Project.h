/**
  * @file Project.h
  *
  * @class Model::Project
  *
  * @copyright (c) 2014, Robert Wloch
  * @license LGPL v2.1
  *
  * @brief Model element representing a project folder within a branch.
  * @details A project is a collection of libraries.
  *
  * @author Robert Wloch, robert@rowlo.de
  *************************************************************************************************/
#ifndef PROJECT_H
#define PROJECT_H

#include <QString>
#include <QSharedPointer>
#include <QMap>
#include <Model/Library.h>

namespace Model
{

class Project
{
public:
    Project();
    Project(const Project &other);
    Project& withPath(const QString &path);
    Project& withName(const QString &name);
    QString getPath() const;
    QString getName() const;
    QSharedPointer<Library> getLibrary(const QString &name) const;
    void addLibrary(QSharedPointer<Library> library);
    QList<QSharedPointer<Library> > getLibraries() const;
    int getLibrariesCount() const;
private:
    QString m_path;
    QString m_name;
    QMap<QString, QSharedPointer<Library> > m_libraries;
};

} // namespace Model

#endif // PROJECT_H
