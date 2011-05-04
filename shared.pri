INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtCore $$[QT_INSTALL_HEADERS]/QtXml $$[QT_INSTALL_HEADERS]/QtNetwork $$[QT_INSTALL_HEADERS]/QtDeclarative $$[QT_INSTALL_HEADERS]/QtTest $$[QT_INSTALL_HEADERS]/QtGui \
               $$QT_BUILD_TREE/include $$QT_BUILD_TREE/include/QtSparql/ \
               $$[QT_INSTALL_HEADERS]
QT = core

QMAKE_LIBDIR = $$QT_BUILD_TREE/lib
QMAKE_INCDIR_QT  = 
#isEmpty(PREFIX): PREFIX = /usr/local
isEmpty(PREFIX): PREFIX = $$QTSPARQL_INSTALL_PREFIX

# this will be in the .so name
VERSION = 0.0.26

# for documentation
DOC_VERSION = 0.0.26
LIBRARYNAME = QtSparql
PACKAGENAME = libqtsparql

