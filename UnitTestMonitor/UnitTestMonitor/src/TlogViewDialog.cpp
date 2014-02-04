/**
  * @file TlogViewDialog.cpp
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
#include "TlogViewDialog.h"
#include "ui_TlogViewDialog.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QSettings>

TlogViewDialog::TlogViewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TlogViewDialog)
{
    ui->setupUi(this);
    connect(this, SIGNAL(finished(int)), SLOT(storeGeometry()));
}

TlogViewDialog::~TlogViewDialog()
{
    delete ui;
}

void TlogViewDialog::storeGeometry()
{
    QSettings settings;
    settings.setValue("TlogViewDialog/size", size());
    settings.setValue("TlogViewDialog/pos", pos());
    settings.sync();
}

void TlogViewDialog::initializeForTlog(
        const QString &tlogPath, const QString &tlogStartLine, const QString &tlogEndLine)
{
    ui->descriptionLabel->setText("");
    ui->tlogTextEdit->setText("");

    int tlogStart = tlogStartLine.toInt();
    int tlogEnd = tlogEndLine.toInt();
    if (tlogStart < 0 || tlogEnd < 0 || tlogEnd <= tlogStart)
    {
        return;
    }

    QFileInfo tlogFileInfo(tlogPath);
    if (not tlogFileInfo.exists())
    {
        return;
    }

    QFile tlogFile(tlogPath);
    if (not tlogFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    QTextStream in(&tlogFile);
    int lineNumber = 0;
    QStringList logLines;
    while (not in.atEnd())
    {
        QString line = in.readLine();
        if (lineNumber >= tlogStart && lineNumber <= tlogEnd)
        {
            logLines.append(line);
        }
        ++lineNumber;
    }
    QString log = logLines.join("\n");
    ui->descriptionLabel->setText(tlogPath);
    ui->tlogTextEdit->setText(log);
}
