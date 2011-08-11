include(../sparqltest.pri)
CONFIG += qt warn_on console depend_includepath
QT += testlib
HEADERS += ../tracker_direct_common.h ../utils/testdata.h
SOURCES  += tst_qsparql_tracker_direct.cpp ../tracker_direct_common.cpp \
            ../utils/testdata.cpp

check.depends = $$TARGET
check.commands = ./tst_qsparql_tracker_direct

memcheck.depends = $$TARGET
memcheck.commands = $$VALGRIND $$VALGRIND_OPT ./tst_qsparql_tracker_direct

QMAKE_EXTRA_TARGETS += check memcheck

#QT = sparql # enable this later
