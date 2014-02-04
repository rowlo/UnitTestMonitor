/**
  * @file LcovBrowserDialog.h
  *
  * @class LcovBrowserDialog
  *
  * @copyright (c) 2014, Robert Wloch
  * @license LGPL v2.1
  *
  * @brief Dialog showing the Lcov html report of a unit test library
  * @details The LcovBrowserDialog shows the Lcov html report of a unit test library.
  *
  * @author Robert Wloch, robert@rowlo.de
  *************************************************************************************************/
#ifndef LCOVBROWSERDIALOG_H
#define LCOVBROWSERDIALOG_H

#include <QDialog>

namespace Ui {
class LcovBrowserDialog;
}

class LcovBrowserDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit LcovBrowserDialog(QWidget *parent = 0);
    ~LcovBrowserDialog();
    void initializeForLcov(const QString &lcovPath);
protected slots:
    void storeGeometry();
    
private:
    Ui::LcovBrowserDialog *ui;
};

#endif // LCOVBROWSERDIALOG_H
