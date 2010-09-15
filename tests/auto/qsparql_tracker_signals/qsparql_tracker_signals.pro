include(../sparqltest.pri)
CONFIG += qt warn_on console depend_includepath testcase
QT += testlib
SOURCES  += tst_qsparql_tracker_signals.cpp

QMAKE_LIBDIR += $$QT_BUILD_TREE/plugins/sparqldrivers
QMAKE_RPATHDIR += $$QT_BUILD_TREE/plugins/sparqldrivers

LIBS += -lqsparqltracker

#QT = sparql # enable this later
