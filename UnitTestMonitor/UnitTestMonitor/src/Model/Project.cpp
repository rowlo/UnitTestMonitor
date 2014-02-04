/**
  * @file Project.cpp
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
#include "Model/Project.h"

#include <QDir>

namespace Model
{

Project::Project()
{
}

Project::Project(const Project &other)
    : m_path(other.m_path),
      m_name(other.m_name),
      m_libraries(other.m_libraries)
{
}

Project& Project::withPath(const QString &path)
{
    m_path = path;
    return *this;
}

Project& Project::withName(const QString &name)
{
    m_name = name;
    return *this;
}

QString Project::getPath() const
{
    return m_path;
}

QString Project::getName() const
{
    return m_name;
}

QSharedPointer<Library> Project::getLibrary(const QString &name) const
{
    QSharedPointer<Library> result;
    if (m_libraries.contains(name))
    {
        result = m_libraries.value(name);
    }
    return result;
}

void Project::addLibrary(QSharedPointer<Library> library)
{
    if (not library.isNull())
    {
        QString folderName = library->getName();
        QDir dir(m_path);
        QString folderPath = QDir::cleanPath(dir.absoluteFilePath(folderName));
        QDir projectDir(folderPath);
        if (projectDir.exists())
        {
            library->withPath(folderPath);
            m_libraries.insert(folderName, library);
        }
    }
}

QList<QSharedPointer<Library> > Project::getLibraries() const
{
    return m_libraries.values();
}

int Project::getLibrariesCount() const
{
    return m_libraries.size();
}

} // namespace Model
