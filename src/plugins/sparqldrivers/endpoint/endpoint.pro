TARGET	 = qsparqlendpoint

HEADERS		= ../../../sparql/drivers/endpoint/qsparql_endpoint.h
SOURCES		= main.cpp \
		  ../../../sparql/drivers/endpoint/qsparql_endpoint.cpp

QT += xml
QT += network

unix: {
    LIBS *= $$QT_LFLAGS_ENDPOINT
    QMAKE_CXXFLAGS *= $$QT_CFLAGS_ENDPOINT
}

include(../qsparqldriverbase.pri)
