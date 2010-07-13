include(../../shared.pri)
LIBS += -lQtSparql
QMAKE_RPATHDIR = $$QT_BUILD_TREE/lib $$QMAKE_RPATHDIR
target.path = $$PREFIX/lib/qsparql-tests # to be changed when we are part of qt
INSTALLS += target
TARGET = tst_$$TARGET