top_srcdir = $$PWD
INCLUDEPATH = $$top_srcdir/include $$top_srcdir/include/QtSparql/
QT = core
QMAKE_LIBDIR = $$top_srcdir/lib
isEmpty(PREFIX): PREFIX = /usr/local

# this will in the .so name
VERSION = 0.0.0

# for documentation
DOC_VERSION = 0.0.1
LIBRARYNAME = QtSparql
PACKAGENAME = libqtsparql

