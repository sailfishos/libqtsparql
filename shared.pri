#we need to explicitly set the include path for any Qt Includes now, add them here
#then we can add the base include path after we've included the headers from the build tree
equals(QT_MAJOR_VERSION, 4) {
    INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtCore $$[QT_INSTALL_HEADERS]/QtXml $$[QT_INSTALL_HEADERS]/QtNetwork $$[QT_INSTALL_HEADERS]/QtDeclarative $$[QT_INSTALL_HEADERS]/QtTest $$[QT_INSTALL_HEADERS]/QtGui $$[QT_INSTALL_HEADERS]/QtDBus \
                   $$QTSPARQL_BUILD_TREE/include $$QTSPARQL_BUILD_TREE/include/QtSparql/ \
                   $$[QT_INSTALL_HEADERS]
}
equals(QT_MAJOR_VERSION, 5) {
    INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtCore $$[QT_INSTALL_HEADERS]/QtXml $$[QT_INSTALL_HEADERS]/QtNetwork $$[QT_INSTALL_HEADERS]/QtQml $$[QT_INSTALL_HEADERS]/QtQuick $$[QT_INSTALL_HEADERS]/QtTest $$[QT_INSTALL_HEADERS]/QtGui $$[QT_INSTALL_HEADERS]/QtDBus \
                   $$QTSPARQL_BUILD_TREE/include $$QTSPARQL_BUILD_TREE/include/Qt5Sparql/ \
                   $$[QT_INSTALL_HEADERS]
}

QT = core

QMAKE_LIBDIR = $$QTSPARQL_BUILD_TREE/lib
#if qt is installed in a non standard location (e.g ~/.local/) the order of the include paths is very important
#don't let qmake automatically generate the includes for Qt libs by setting this to nothing
QMAKE_INCDIR_QT  = 
#isEmpty(PREFIX): PREFIX = /usr/local
isEmpty(PREFIX): PREFIX = $$QTSPARQL_INSTALL_PREFIX

# this will be in the .so name
VERSION = 0.2.6

# for documentation
DOC_VERSION = 0.2.6
equals(QT_MAJOR_VERSION, 4) {
    LIBRARYNAME = QtSparql
    PACKAGENAME = libqtsparql
}
equals(QT_MAJOR_VERSION, 5) {
    LIBRARYNAME = Qt5Sparql
    PACKAGENAME = libqt5sparql
}

