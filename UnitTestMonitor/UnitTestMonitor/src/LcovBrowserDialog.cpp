/**
  * @file LcovBrowserDialog.cpp
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
#include "LcovBrowserDialog.h"
#include "ui_LcovBrowserDialog.h"

#include <QFileInfo>
#include <QSettings>
#include <QtWebKit>
#include <QWebView>
#include <QDebug>

LcovBrowserDialog::LcovBrowserDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LcovBrowserDialog)
{
    ui->setupUi(this);
    connect(this, SIGNAL(finished(int)), SLOT(storeGeometry()));
}

LcovBrowserDialog::~LcovBrowserDialog()
{
    delete ui;
}

void LcovBrowserDialog::storeGeometry()
{
    QSettings settings;
    settings.setValue("LcovBrowserDialog/size", size());
    settings.setValue("LcovBrowserDialog/pos", pos());
    settings.sync();
}

void LcovBrowserDialog::initializeForLcov(const QString &lcovPath)
{
    ui->descriptionLabel->setText("");
    ui->lcovBrowserView->setHtml("");

    QFileInfo fileInfo(lcovPath);
    if (not fileInfo.exists())
    {
        return;
    }

    ui->descriptionLabel->setText(lcovPath);
    ui->lcovBrowserView->load(QUrl::fromLocalFile(lcovPath));
}
