/**
  * @file AboutDialog.cpp
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
#include "include/AboutDialog.h"
#include "ui_AboutDialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
