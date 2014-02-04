/**
  * @file BranchScanner.h
  *
  * @class BranchScanner
  *
  * @copyright (c) 2014, Robert Wloch
  * @license LGPL v2.1
  *
  * @brief Scans a branch directory for unit test data
  * @details The branch scanner scans a directory structure for libraries with tests, tlog, and lcov
  *          reports.
  *
  * @author Robert Wloch, robert@rowlo.de
  *************************************************************************************************/
#ifndef BRANCHSCANNER_H
#define BRANCHSCANNER_H

#include <Model/Branch.h>
#include <Model/Library.h>
#include <QSharedPointer>
#include <QString>

class BranchScanner
{
public:
    BranchScanner();
    QSharedPointer<Model::Branch> scanBranch(const QString &path);
    QSharedPointer<Model::Branch> updateBranch(const QSharedPointer<Model::Branch> &branch);
protected:
    void analyzeTlog(const QString &tlogFilePath,
                     const QSharedPointer<Model::Library> &library,
                     qint64 timestamp);
};

#endif // BRANCHSCANNER_H
