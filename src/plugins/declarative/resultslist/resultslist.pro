TARGET	 = sparqlresultslist

HEADERS		= ../../../sparql/models/qsparqlconnectionoptionswrapper_p.h \
                ../../../sparql/models/qsparqlresultslist_p.h

SOURCES		= main.cpp \
                ../../../sparql/models/qsparqlconnectionoptionswrapper.cpp \
                ../../../sparql/models/qsparqlresultslist.cpp

include(../declarativebase.pri)
QT += declarative
