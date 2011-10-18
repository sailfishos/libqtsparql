include(../sparqltest.pri)
CONFIG += qt warn_on console depend_includepath
QT += testlib xml network
HEADERS += EndpointService.h \
           EndpointServer.h
SOURCES  += tst_qsparql_endpoint.cpp \
            EndpointService.cpp \
            EndpointServer.cpp

#QT = sparql # enable this later

check.commands = ./tst_qsparql_endpoint

memcheck.depends = $$TARGET
memcheck.commands = echo '-'

QMAKE_EXTRA_TARGETS += check memcheck

check.depends = $$TARGET
