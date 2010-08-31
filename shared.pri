INCLUDEPATH = $$QT_BUILD_TREE/include $$QT_BUILD_TREE/include/QtSparql/
QT = core

QMAKE_LIBDIR = $$QT_BUILD_TREE/lib

isEmpty(PREFIX): PREFIX = /usr/local

# this will in the .so name
VERSION = 0.0.0

# for documentation
DOC_VERSION = 0.0.4
LIBRARYNAME = QtSparql
PACKAGENAME = libqtsparql

