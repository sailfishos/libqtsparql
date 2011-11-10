#we need to explicitly set the include path for any Qt Includes now, add them here
#then we can add the base include path after we've included the headers from the build tree
INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtCore $$[QT_INSTALL_HEADERS]/QtXml $$[QT_INSTALL_HEADERS]/QtNetwork $$[QT_INSTALL_HEADERS]/QtDeclarative $$[QT_INSTALL_HEADERS]/QtTest $$[QT_INSTALL_HEADERS]/QtGui $$[QT_INSTALL_HEADERS]/QtDBus \
               $$QT_BUILD_TREE/include $$QT_BUILD_TREE/include/QtSparql/ \
               $$[QT_INSTALL_HEADERS]
QT = core

QMAKE_LIBDIR = $$QT_BUILD_TREE/lib
#if qt is installed in a non standard location (e.g ~/.local/) the order of the include paths is very important
#don't let qmake automatically generate the includes for Qt libs by setting this to nothing
QMAKE_INCDIR_QT  = 
#isEmpty(PREFIX): PREFIX = /usr/local
isEmpty(PREFIX): PREFIX = $$QTSPARQL_INSTALL_PREFIX

# this will be in the .so name
VERSION = 0.2.6

# for documentation
DOC_VERSION = 0.2.6
LIBRARYNAME = QtSparql
PACKAGENAME = libqtsparql

