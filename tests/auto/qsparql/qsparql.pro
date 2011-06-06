include(../sparqltest.pri)
CONFIG += qt warn_on console depend_includepath
QT += testlib
SOURCES  += tst_qsparql.cpp

check.depends = $$TARGET
check.commands = ./tst_qsparql

memcheck.depends = $$TARGET
memcheck.commands = $$VALGRIND $$VALGRIND_OPT ./tst_qsparql

QMAKE_EXTRA_TARGETS += check memcheck

# QT = sparql # enable this later

