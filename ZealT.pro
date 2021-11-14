#-------------------------------------------------
#
# Project created by QtCreator 2021-07-30T16:28:03
#
#-------------------------------------------------

QT       += core gui network  concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ZealT
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        core/application.cpp \
        core/applicationsingleton.cpp \
        core/filemanager.cpp \
        core/networkaccessmanager.cpp \
        core/settings.cpp \
        main.cpp \
        registry/docset.cpp \
        registry/docsetmetadata.cpp \
        registry/docsetregistry.cpp \
        registry/listmodel.cpp \
        registry/searchmodel.cpp \
        registry/searchquery.cpp \
        ui/aboutdialog.cpp \
        ui/docsetlistitemdelegate.cpp \
        ui/docsetsdialog.cpp \
        ui/mainwindow.cpp \
        ui/progressitemdelegate.cpp \
        ui/qxtglobalshortcut/qxtglobalshortcut.cpp \
        ui/qxtglobalshortcut/qxtglobalshortcut_win.cpp \
        ui/searchitemdelegate.cpp \
        ui/settingsdialog.cpp \
        ui/webbridge.cpp \
        ui/widgets/searchedit.cpp \
        ui/widgets/searchtoolbar.cpp \
        ui/widgets/shortcutedit.cpp \
        ui/widgets/toolbarframe.cpp \
        util/plist.cpp \
        util/sqlitedatabase.cpp \
        util/version.cpp

HEADERS += \
        core/application.h \
        core/applicationsingleton.h \
        core/filemanager.h \
        core/networkaccessmanager.h \
        core/settings.h \
        registry/cancellationtoken.h \
        registry/docset.h \
        registry/docsetmetadata.h \
        registry/docsetregistry.h \
        registry/itemdatarole.h \
        registry/listmodel.h \
        registry/searchmodel.h \
        registry/searchquery.h \
        registry/searchresult.h \
        ui/aboutdialog.h \
        ui/docsetlistitemdelegate.h \
        ui/docsetsdialog.h \
        ui/mainwindow.h \
        ui/progressitemdelegate.h \
        ui/qxtglobalshortcut/qxtglobalshortcut.h \
        ui/qxtglobalshortcut/qxtglobalshortcut_p.h \
        ui/searchitemdelegate.h \
        ui/settingsdialog.h \
        ui/webbridge.h \
        ui/widgets/searchedit.h \
        ui/widgets/searchtoolbar.h \
        ui/widgets/shortcutedit.h \
        ui/widgets/toolbarframe.h \
        util/plist.h \
        util/sqlitedatabase.h \
        util/version.h

FORMS += \
        ui/aboutdialog.ui \
        ui/docsetsdialog.ui \
        ui/mainwindow.ui \
        ui/settingsdialog.ui

RESOURCES += \
        resources/zeal.qrc


# LIBS += -L"$$PWD/libarchive/lib" -larchive.lib
# LIBS += -l"$$PWD/libarchive/lib/archive.lib"
LIBS += "$$PWD/libarchive/lib/archive.lib"
# LIBS += -L"$$PWD/libarchive/lib" -larchive_static

INCLUDEPATH += libarchive/include
DEPENDPATH += libarchive/include

# LIBS += -L"$$PWD/sqlite/lib" -lsqlite3.lib
# LIBS += -l"$$PWD/sqlite/lib/sqlite3.lib"
LIBS += "$$PWD/sqlite/lib/sqlite3.lib"

INCLUDEPATH += sqlite/include
DEPENDPATH += sqlite/include
