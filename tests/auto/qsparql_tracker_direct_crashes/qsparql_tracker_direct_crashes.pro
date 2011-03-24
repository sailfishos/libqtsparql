include(../sparqltest.pri)
CONFIG += qt warn_on console depend_includepath testcase
QT += testlib
SOURCES  += tst_qsparql_tracker_direct_crashes.cpp

# this test is not ran when doing "make check", but it's inluded in the test package

#QT = sparql # enable this later
