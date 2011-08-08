include(../sparqltest.pri)
CONFIG += qt warn_on console depend_includepath
QT += testlib
HEADERS += ../tracker_direct_common.h updatetester.h resultchecker.h
SOURCES  += tst_qsparql_tracker_direct_concurrency.cpp \
            updatetester.cpp resultchecker.cpp \
        ../tracker_direct_common.cpp

check.depends = $$TARGET
check.commands = ./tst_qsparql_tracker_direct_concurrency

memcheck.depends = $$TARGET
memcheck.commands = $$VALGRIND $$VALGRIND_OPT ./tst_qsparql_tracker_direct_concurrency

QMAKE_EXTRA_TARGETS += check memcheck

#QT = sparql # enable this later
