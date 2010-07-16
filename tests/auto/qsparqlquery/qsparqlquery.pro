include(../sparqltest.pri)
CONFIG += qt warn_on console depend_includepath
QT += testlib

SOURCES  += tst_qsparqlquery.cpp

check.depends = $$TARGET
check.commands = ./tst_qsparqlquery

QMAKE_EXTRA_TARGETS += check

#QT = sparql # enable this later

