/**
  * @file main.cpp
  *
  * @class none
  *
  * @copyright (c) 2014, Robert Wloch
  * @license LGPL v2.1
  *
  * @brief main function starting the Unit Test Monitor
  * @details The Unit Test Monitor gathers tlog and lcov information of projects.
  *
  * @author Robert Wloch, robert@rowlo.de
  *************************************************************************************************/
#include "MainWindow.h"
#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("CuteOpenSourceWorld");
    QCoreApplication::setOrganizationDomain("rowlo.de");
    QCoreApplication::setApplicationName("UnitTestMonitor");

    QString styleFilePath(":style/utm.qss");
    QFile styleFile(styleFilePath);
    if (styleFile.exists())
    {
        styleFile.open(QFile::ReadOnly | QIODevice::Text);
        qApp->setStyleSheet(QLatin1String(styleFile.readAll()));
    }

    MainWindow w;
    w.show();
    
    return a.exec();
}
