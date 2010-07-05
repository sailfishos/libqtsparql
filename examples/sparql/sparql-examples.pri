include(../../shared.pri)

INCLUDEPATH += $$top_srcdir/include/QtSparql $$top_srcdir/include

LIBS += -lQtSparql
QMAKE_RPATHDIR = $$top_srcdir/lib $$QMAKE_RPATHDIR
