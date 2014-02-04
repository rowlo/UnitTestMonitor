/**
  * @file TlogViewDialog.h
  *
  * @class TlogViewDialog
  *
  * @copyright (c) 2014, Robert Wloch
  * @license LGPL v2.1
  *
  * @brief Dialog showing the tlog fraction of a testcase
  * @details The TlogViewDialog shows only the lines of a testcase.
  *
  * @author robert@rowlo.de
  *************************************************************************************************/
#ifndef TLOGVIEWDIALOG_H
#define TLOGVIEWDIALOG_H

#include <QDialog>

namespace Ui {
class TlogViewDialog;
}

class TlogViewDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit TlogViewDialog(QWidget *parent = 0);
    ~TlogViewDialog();
    void initializeForTlog(
            const QString &tlogPath, const QString &tlogStartLine, const QString &tlogEndLine);
protected slots:
    void storeGeometry();

private:
    Ui::TlogViewDialog *ui;
};

#endif // TLOGVIEWDIALOG_H
