include(../sparqltest.pri)
CONFIG += qt warn_on console depend_includepath testcase link_pkgconfig
PKGCONFIG = tracker-sparql-2.0
QT += testlib xml
SOURCES  += tst_qsparql_benchmark.cpp

#QT = sparql # enable this later
