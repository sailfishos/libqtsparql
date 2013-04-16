include(../sparqltest.pri)
CONFIG += qt warn_on console depend_includepath testcase link_pkgconfig
PKGCONFIG = tracker-sparql-0.14
QT += testlib xml
SOURCES  += tst_qsparql_benchmark.cpp

#QT = sparql # enable this later
