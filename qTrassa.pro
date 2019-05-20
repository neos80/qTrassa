#-------------------------------------------------
#
# Project created by QtCreator 2014-11-03T11:55:35
#
#-------------------------------------------------

QT       += core gui
CONFIG   += static

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

VERSION = 0.52
#QMAKE_TARGET_COMPANY = Kosorukov O.M.
#QMAKE_TARGET_PRODUCT = qTrassa
#QMAKE_TARGET_DESCRIPTION = App qTrassa
#QMAKE_TARGET_COPYRIGHT =Oleg M.Kosorukov 2019


TARGET = qTrassa
TEMPLATE = app

SOURCES += src\main.cpp\
        src\mainwindow.cpp \
    src\geo.cpp \
    src\comboboxdelegate.cpp \
    src\qlineeditdelegate.cpp \
    src\calcq.cpp \
    src\about.cpp

HEADERS  += src\mainwindow.h \
    src\geo.h \
    src\comboboxdelegate.h \
    src\qlineeditdelegate.h \
    src\calcq.h \
    src\about.h

FORMS    += form\mainwindow.ui \
    form\calcq.ui \
    form\about.ui

OTHER_FILES +=

RESOURCES += icon.qrc
RC_FILE = myapp.rc

CONFIG += release
CONFIG += static

QMAKE_LFLAGS += -static -static-libgcc
LIBS += -static-libgcc -lz

# Выбираем директорию сборки исполняемого файла
# в зависимости от режима сборки проекта
CONFIG(debug, debug|release) {
    DESTDIR = $$OUT_PWD/../GiroRelease/debug/
} else {
    DESTDIR = $$OUT_PWD/../bin/
}

# разделяем по директориям все выходные файлы проекта
MOC_DIR = common/moc
RCC_DIR = common/rcc
UI_DIR = common/ui
unix:OBJECTS_DIR =common/unix
win32:OBJECTS_DIR = common/win32
macx:OBJECTS_DIR = common/mac

#Сборка библиотек
isEmpty(TARGET_EXT) {
    win32 {
        TARGET_CUSTOM_EXT = .exe
    }
    macx {
        TARGET_CUSTOM_EXT = .app
    }
} else {
    TARGET_CUSTOM_EXT = $${TARGET_EXT}
}

win32 {
    DEPLOY_COMMAND = $$(QTDIR)/bin/windeployqt
}
macx {
    DEPLOY_COMMAND = $$(QTDIR)/bin/macdeployqt
}

CONFIG( debug, debug|release ) {
    # debug
    DEPLOY_TARGET = $$shell_quote($$shell_path($${OUT_PWD}/../GiroRelease/debug/$${TARGET}$${TARGET_CUSTOM_EXT}))
} else {
    # release
    DEPLOY_TARGET = $$shell_quote($$shell_path($${OUT_PWD}/../bin/$${TARGET}$${TARGET_CUSTOM_EXT}))
}

#  # Uncomment the following line to help debug the deploy command when running qmake
#  warning($${DEPLOY_COMMAND} $${DEPLOY_TARGET})

# Use += instead of = if you use multiple QMAKE_POST_LINKs
QMAKE_POST_LINK = $${DEPLOY_COMMAND} $${DEPLOY_TARGET}

