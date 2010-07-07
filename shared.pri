top_srcdir = $$PWD
INCLUDEPATH = $$top_srcdir/include $$top_srcdir/include/QtSparql/
QT = core
QMAKE_LIBDIR = $$top_srcdir/lib
isEmpty(PREFIX): PREFIX = /usr/local

# for documentation
VERSION = 0.0.1
LIBRARYNAME = QtTracker

