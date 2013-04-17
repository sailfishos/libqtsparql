include(../sparqltest.pri)
CONFIG += qt warn_on console depend_includepath
QT += testlib

SOURCES  += tst_qsparql_ntriples.cpp ../../../src/sparql/kernel/qsparqlntriples.cpp
HEADERS  += ../../../src/sparql/kernel/qsparqlntriples_p.h

check.depends = $$TARGET
check.commands = ./tst_qsparql_ntriples

memcheck.depends = $$TARGET
memcheck.commands = $$VALGRIND $$VALGRIND_OPT ./tst_qsparql_ntriples

QMAKE_EXTRA_TARGETS += check memcheck

#QT = sparql # enable this later

