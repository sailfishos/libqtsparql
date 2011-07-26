include(../sparqltest.pri)
CONFIG += qt warn_on console depend_includepath
QT += testlib

SOURCES  += tst_qsparqlquery.cpp

check.depends = $$TARGET
check.commands = ./tst_qsparqlquery

memcheck.depends = $$TARGET
memcheck.commands = $$VALGRIND $$VALGRIND_OPT ./tst_qsparqlquery

QMAKE_EXTRA_TARGETS += check memcheck

#QT = sparql # enable this later

