include(../sparqltest.pri)
CONFIG += qt warn_on console depend_includepath
QT += testlib
SOURCES  += tst_qsparql_tracker_direct_crashes.cpp

TEST_ENV = XDG_CACHE_HOME=bogus TRACKER_SPARQL_BACKEND=direct
TEST_EXE = ./tst_qsparql_tracker_direct_crashes
TEST_CASES = waitForFinished_crashes_when_connection_opening_fails \
             syncExec_crashes_when_connection_opening_fails \
             syncExec_update_crashes_when_connection_opening_fails

check.depends = $$TARGET
check.commands = $$TEST_ENV $$TEST_EXE $$TEST_CASES

memcheck.depends = $$TARGET
memcheck.commands = $$TEST_ENV $$VALGRIND $$VALGRIND_OPT $$TEST_EXE $$TEST_CASES

QMAKE_EXTRA_TARGETS += check memcheck

#QT = sparql # enable this later
