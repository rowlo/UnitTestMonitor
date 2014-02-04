#-------------------------------------------------
#
# Project created by QtCreator 2014-02-03T20:28:51
#
#-------------------------------------------------

QT       += core gui webkit xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets concurrent webkitwidgets

TARGET = UnitTestMonitor
DESTDIR = ../bin
TEMPLATE = app

SOURCES += src/main.cpp\
        src/MainWindow.cpp \
    src/TlogViewDialog.cpp \
    src/LcovBrowserDialog.cpp \
    src/Model/MonitorSet.cpp \
    src/Model/Branch.cpp \
    src/MonitorSetReader.cpp \
    src/Model/Project.cpp \
    src/Model/Testrun.cpp \
    src/Model/Testcase.cpp \
    src/Model/Library.cpp \
    src/BranchScanner.cpp \
    src/MonitorSetWriter.cpp \
    src/AboutDialog.cpp

INCLUDEPATH += include

HEADERS  += include/MainWindow.h \
    include/TlogViewDialog.h \
    include/LcovBrowserDialog.h \
    include/Model/MonitorSet.h \
    include/Model.h \
    include/Model/Branch.h \
    include/MonitorSetReader.h \
    include/Model/Project.h \
    include/Model/Testrun.h \
    include/Model/Testcase.h \
    include/Model/Library.h \
    include/BranchScanner.h \
    include/MonitorSetWriter.h \
    include/AboutDialog.h

FORMS    += form/MainWindow.ui \
    form/TlogViewDialog.ui \
    form/LcovBrowserDialog.ui \
    form/AboutDialog.ui

RESOURCES += \
    resources/UnitTestMonitor.qrc

OTHER_FILES += \
    resources/utm.qss
