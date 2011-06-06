include(../sparqltest.pri)
CONFIG += qt warn_on console depend_includepath
QT += testlib

SOURCES  += tst_qsparqlresultrow.cpp

check.depends = $$TARGET
check.commands = ./tst_qsparqlresultrow

memcheck.depends = $$TARGET
memcheck.commands = $$VALGRIND $$VALGRIND_OPT ./tst_qsparqlresultrow

QMAKE_EXTRA_TARGETS += check memcheck

#QT = sparql # enable this later
