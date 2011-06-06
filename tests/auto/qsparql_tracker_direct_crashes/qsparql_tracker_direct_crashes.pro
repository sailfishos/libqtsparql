include(../sparqltest.pri)
CONFIG += qt warn_on console depend_includepath
QT += testlib
SOURCES  += tst_qsparql_tracker_direct_crashes.cpp

TEST_OPT = XDG_CACHE_HOME=bogus TRACKER_SPARQL_BACKEND=direct
TEST_EXE = ./tst_qsparql_tracker_direct_crashes

check.depends = $$TARGET
check.commands = $$TEST_OPT $$TEST_EXE waitForFinished_crashes_when_connection_opening_fails && \
                 $$TEST_OPT $$TEST_EXE syncExec_crashes_when_connection_opening_fails && \
                 $$TEST_OPT $$TEST_EXE syncExec_update_crashes_when_connection_opening_fails

memcheck.depends = $$TARGET
memcheck.commands = $$TEST_OPT $$VALGRIND $$VALGRIND_OPT $$TEST_EXE \
                    waitForFinished_crashes_when_connection_opening_fails &&\
                    $$TEST_OPT $$VALGRIND $$VALGRIND_OPT $$TEST_EXE \
                    syncExec_crashes_when_connection_opening_fails &&\
                    $$TEST_OPT $$VALGRIND $$VALGRIND_OPT $$TEST_EXE \
                    syncExec_update_crashes_when_connection_opening_fails

QMAKE_EXTRA_TARGETS += check memcheck

# this test is not ran when doing "make check", but it's inluded in the test package

#QT = sparql # enable this later
