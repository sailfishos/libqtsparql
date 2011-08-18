include(../sparqltest.pri)
CONFIG += qt warn_on console depend_includepath testcase
QT += testlib xml network
SOURCES  += tst_qsparql_endpoint.cpp

#QT = sparql # enable this later

