TARGET	   = QtSparql
QPRO_PWD   = $$PWD
QT         = core
QT         += network
DEFINES += QT_BUILD_SPARQL_LIB
DEFINES += QT_NO_USING_NAMESPACE
win32-msvc*|win32-icc:QMAKE_LFLAGS += /BASE:0x62000000

unix:QMAKE_PKGCONFIG_REQUIRES = QtCore

include(../qbase.pri)

DEFINES += QT_NO_CAST_FROM_ASCII
PRECOMPILED_HEADER = ../corelib/global/qt_pch.h
SPARQL_P = sparql

include(kernel/kernel.pri)
include(drivers/drivers.pri)
include(models/models.pri)

symbian: {
    TARGET.UID3=0x2001E61D

    # Workaroud for problems with paging this dll
    MMP_RULES -= PAGED
    MMP_RULES *= UNPAGED
}
