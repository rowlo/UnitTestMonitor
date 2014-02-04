/**
  * @file Library.h
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
#ifndef LIBRARY_H
#define LIBRARY_H

#include <QString>
#include <QSharedPointer>
#include <QMap>
#include <Model/Testcase.h>

namespace Model
{

class Library
{
public:
    Library();
    Library(const Library &other);
    Library& withPath(const QString &path);
    Library& withName(const QString &name);
    Library& withLcovPath(const QString &path);
    QString getPath() const;
    QString getName() const;
    QString getLcovPath() const;
    QSharedPointer<Testcase> getTestcase(const QString &name) const;
    void addTestcase(QSharedPointer<Testcase> testcase);
    QList<QSharedPointer<Testcase> > getTestcases() const;
    int getTestcasesCount() const;
private:
    QString m_path;
    QString m_name;
    QString m_lcovPath;
    QMap<QString, QSharedPointer<Testcase> > m_testcases;
};

} // namespace Model

#endif // LIBRARY_H
