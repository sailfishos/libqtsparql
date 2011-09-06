include(../sparqltest.pri)
CONFIG += qt warn_on console depend_includepath
QT += testlib xml
SOURCES  += tst_qsparql_threading.cpp

#QT = sparql # enable this later

TEST_EXE = ./tst_qsparql_threading
TEST_CASES = concurrentTrackerQueries

check.depends = $$TARGET
check.commands = $$TEST_EXE $$TEST_CASES

memcheck.depends = $$TARGET
memcheck.commands = $$VALGRIND $$VALGRIND_OPT \
                      $$TEST_EXE $$TEST_CASES

QMAKE_EXTRA_TARGETS += check memcheck
