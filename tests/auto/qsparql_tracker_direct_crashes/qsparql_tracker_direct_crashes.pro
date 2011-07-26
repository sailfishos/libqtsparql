include(../sparqltest.pri)
CONFIG += qt warn_on console depend_includepath
QT += testlib
SOURCES  += tst_qsparql_tracker_direct_crashes.cpp

TEST_ENV = XDG_CACHE_HOME=bogus TRACKER_SPARQL_BACKEND=direct
TEST_EXE = ./tst_qsparql_tracker_direct_crashes

check.depends = $$TARGET
check.commands = $$TEST_ENV $$TEST_EXE

memcheck.depends = $$TARGET
#memcheck.commands = $$TEST_ENV $$VALGRIND $$VALGRIND_OPT $$TEST_EXE
memcheck.commands = @echo $$TARGET: memcheck is disabled for until leak in tracker is fixed

QMAKE_EXTRA_TARGETS += check memcheck

#QT = sparql # enable this later
