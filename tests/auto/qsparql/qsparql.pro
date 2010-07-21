include(../sparqltest.pri)
CONFIG += qt warn_on console depend_includepath
QT += testlib
SOURCES  += tst_qsparql.cpp

check.depends = $$TARGET
check.commands = ./tst_qsparql

QMAKE_EXTRA_TARGETS += check

# QT = sparql # enable this later

