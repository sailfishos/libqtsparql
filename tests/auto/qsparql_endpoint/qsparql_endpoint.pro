include(../sparqltest.pri)
CONFIG += qt warn_on console depend_includepath testcase
QT += testlib xml network
HEADERS += EndpointService.h \
           EndpointServer.h
SOURCES  += tst_qsparql_endpoint.cpp \
            EndpointService.cpp \
            EndpointServer.cpp

#QT = sparql # enable this later

