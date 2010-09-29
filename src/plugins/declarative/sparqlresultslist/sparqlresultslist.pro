TARGET	 = sparqlresultslist

# HEADERS		= ../../../sparql/models/qsparqlconnectionoptionswrapper_p.h \
#                 ../../../sparql/models/qsparqlresultslist_p.h

SOURCES		= main.cpp

qdeclarativesources.files += qmldir

LIBS += -lQtSparql

include(../declarativebase.pri)
QT += declarative
