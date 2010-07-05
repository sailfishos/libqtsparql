TARGET	 = qsparqltracker

HEADERS		= ../../../sparql/drivers/tracker/qsparql_tracker.h \
                  ../../../sparql/drivers/tracker/qsparql_tracker_p.h
SOURCES		= main.cpp \
		  ../../../sparql/drivers/tracker/qsparql_tracker.cpp

unix: {
    LIBS *= $$QT_LFLAGS_TRACKER
    QMAKE_CXXFLAGS *= $$QT_CFLAGS_TRACKER
}

include(../qsparqldriverbase.pri)
QT += dbus

