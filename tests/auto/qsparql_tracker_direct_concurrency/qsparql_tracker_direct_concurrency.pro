include(../sparqltest.pri)
CONFIG += qt warn_on console depend_includepath
QT += testlib
HEADERS += ../utils/testdata.h querytester.h updatetester.h resultchecker.h
SOURCES  += tst_qsparql_tracker_direct_concurrency.cpp \
            querytester.cpp updatetester.cpp resultchecker.cpp \
        ../utils/testdata.cpp

check.depends = $$TARGET
check.commands = ./tst_qsparql_tracker_direct_concurrency

QMAKE_EXTRA_TARGETS += check

#QT = sparql # enable this later
