TARGET = qsparqlvirtuoso

CONFIG += debug_and_release

HEADERS	= ../../../sparql/drivers/virtuoso/qsparql_virtuoso_p.h
SOURCES	= main.cpp \
          ../../../sparql/drivers/virtuoso/qsparql_virtuoso.cpp

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += libiodbc
}

win32 {
    !win32-borland:LIBS	*= -lodbc32
    win32-borland:LIBS	*= $(BCB)/lib/PSDK/odbc32.lib
}

include(../qsparqldriverbase.pri)
