/**
  * @file AboutDialog.h
  *
  * @class AboutDialog
  *
  * @copyright (c) 2014, Robert Wloch
  * @license LGPL v2.1
  *
  * @brief About dialog of UnitTestMonitor.
  * @details The about dialog contains a little howto guide.
  *
  * @author Robert Wloch, robert@rowlo.de
  *************************************************************************************************/
#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit AboutDialog(QWidget *parent = 0);
    ~AboutDialog();
    
private:
    Ui::AboutDialog *ui;
};

#endif // ABOUTDIALOG_H
