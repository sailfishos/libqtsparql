include(../sparqltest.pri)
CONFIG += qt warn_on console depend_includepath testcase
QT += testlib xml
SOURCES  += tst_qsparql_threading.cpp

CONFIG += debug
#QT = sparql # enable this later
