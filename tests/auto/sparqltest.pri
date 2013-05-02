include(../../shared.pri)
equals(QT_MAJOR_VERSION, 4): LIBS += -lQtSparql
equals(QT_MAJOR_VERSION, 5): {
    LIBS += -lQt5Sparql
    DEFINES *= QT_VERSION_5
}
QMAKE_RPATHDIR = $$QTSPARQL_BUILD_TREE/lib $$QMAKE_RPATHDIR
target.path = $$QTSPARQL_INSTALL_TESTS # to be changed when we are part of qt
INSTALLS += target
target.path = $$PREFIX/lib/$$PACKAGENAME-tests/
TARGET = tst_$$TARGET

VALGRIND = G_SLICE=always-malloc G_DEBUG=gc-friendly valgrind
VALGRIND_OPT = --tool=memcheck --leak-check=full --num-callers=20 \
               --suppressions=../memcheck-suppressions --error-exitcode=1000
