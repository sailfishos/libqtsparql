include(../../shared.pri)
LIBS += -lQtSparql
QMAKE_RPATHDIR = $$QT_BUILD_TREE/lib $$QMAKE_RPATHDIR
target.path = $$PREFIX/share/qsparql-tests # to be changed when we are part of qt
