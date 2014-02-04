/**
  * @file Library.cpp
  *
  * @class Model::Library
  *
  * @copyright (c) 2014, Robert Wloch
  * @license LGPL v2.1
  *
  * @brief Model element representing a library folder within a project.
  * @details A library has a collection of testcases.
  *
  * @author Robert Wloch, robert@rowlo.de
  *************************************************************************************************/
#include "Model/Library.h"

#include <QDir>

namespace Model
{

Library::Library()
{
}

Library::Library(const Library &other)
    : m_path(other.m_path),
      m_name(other.m_name),
      m_lcovPath(other.m_lcovPath),
      m_testcases(other.m_testcases)
{
}

Library& Library::withPath(const QString &path)
{
    m_path = path;
    return *this;
}

QString Library::getPath() const
{
    return m_path;
}

Library& Library::withName(const QString &name)
{
    m_name = name;
    return *this;
}

QString Library::getName() const
{
    return m_name;
}

Library& Library::withLcovPath(const QString &path)
{
    m_lcovPath = path;
    return *this;
}

QString Library::getLcovPath() const
{
    return m_lcovPath;
}

QSharedPointer<Testcase> Library::getTestcase(const QString &name) const
{
    QSharedPointer<Testcase> result;
    if (m_testcases.contains(name))
    {
        result = m_testcases.value(name);
    }
    return result;
}

void Library::addTestcase(QSharedPointer<Testcase> testcase)
{
    if (not testcase.isNull())
    {
        QString name = testcase->getName();
        m_testcases.insert(name, testcase);
    }
}

QList<QSharedPointer<Testcase> > Library::getTestcases() const
{
    return m_testcases.values();
}

int Library::getTestcasesCount() const
{
    return m_testcases.size();
}

} // namespace Model
