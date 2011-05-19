include(../../shared.pri)
LIBS += -lQtSparql
QMAKE_RPATHDIR = $$QT_BUILD_TREE/lib $$QMAKE_RPATHDIR
target.path = $$QTSPARQL_INSTALL_TESTS # to be changed when we are part of qt
INSTALLS += target
TARGET = tst_$$TARGET

VALGRIND = G_SLICE=always-malloc G_DEBUG=gc-friendly valgrind
VALGRIND_OPT = --tool=memcheck --leak-check=full --num-callers=20 \
               --suppressions=../memcheck-suppressions --error-exitcode=1000
