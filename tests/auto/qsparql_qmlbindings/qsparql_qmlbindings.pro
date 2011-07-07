include(../sparqltest.pri)
CONFIG += qt warn_on console depend_includepath
QT += testlib declarative

SOURCES  += tst_qsparql_qmlbindings.cpp

check.depends = $$TARGET
check.commands = ./tst_qsparql_qmlbindings

# QMAKE_EXTRA_TARGETS += check

#QT = sparql # enable this later

