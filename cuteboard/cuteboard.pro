QT += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app

CONFIG += c++11
#CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

!CONFIG(debug, debug|release) {
    DEFINES += QT_NO_DEBUG_OUTPUT QT_NO_WARNING_OUTPUT
}

SOURCES += main.cpp \
    cuteboard.cpp \
    cuteboarddclient.cpp \
    crypt.cpp \
    external/twofish/TWOFISH2.C \
    settingsdialog.cpp \
    previewwidget.cpp

HEADERS += \
    cuteboard.h \
    cuteboarddclient.h \
    crypt.h \
    settingsdialog.h \
    previewwidget.h

RESOURCES += \
    resources/resources.qrc

FORMS += \
    settingsdialog.ui
