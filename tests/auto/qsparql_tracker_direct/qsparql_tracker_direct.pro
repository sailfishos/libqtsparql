include(../sparqltest.pri)
CONFIG += qt warn_on console depend_includepath testcase
QT += testlib
SOURCES  += tst_qsparql_tracker_direct.cpp

check.depends = $$TARGET
check.commands = ./tst_qsparql_tracker_direct_wrapper.sh

QMAKE_EXTRA_TARGETS += check

#QT = sparql # enable this later

