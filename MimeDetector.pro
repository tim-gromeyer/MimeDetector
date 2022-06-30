TARGET = detectmimetype

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x051208    # disables all the APIs deprecated before Qt 5.12.8

INCLUDEPATH += \
    src/

HEADERS += \
    src/about.h \
    src/mainwindow.h \
    src/mimetypemodel.h

SOURCES += \
    src/about.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/mimetypemodel.cpp

RESOURCES += \
    translations/translations.qrc

TRANSLATIONS += \
    translations/DetectMimeType_de_DE.ts


CONFIG += lrelease
CONFIG += embed_translations
CONFIG -= qtquickcompiler

VERSION = 0.0.0
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

# Only show qDebug() messages in debug mode
CONFIG(release, debug | release): DEFINES += QT_NO_DEBUG_OUTPUT

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /usr/bin
!isEmpty(target.path): INSTALLS += target
