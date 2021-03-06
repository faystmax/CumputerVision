#-------------------------------------------------
#
# Project created by QtCreator 2018-02-14T23:58:13
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Gui
TEMPLATE = app
CONFIG += c++1z

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../linalg/release/ -llinalg
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../linalg/debug/ -llinalg
else:unix: LIBS += -L$$OUT_PWD/../linalg/ -llinalg

INCLUDEPATH += $$PWD/../linalg
DEPENDPATH += $$PWD/../linalg

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../linalg/release/liblinalg.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../linalg/debug/liblinalg.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../linalg/release/linalg.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../linalg/debug/linalg.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../linalg/liblinalg.a

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../ImageS/release/ -lImageS
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../ImageS/debug/ -lImageS
else:unix: LIBS += -L$$OUT_PWD/../ImageS/ -lImageS

INCLUDEPATH += $$PWD/../ImageS
DEPENDPATH += $$PWD/../ImageS
