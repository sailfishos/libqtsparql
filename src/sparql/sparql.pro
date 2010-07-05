include(../../shared.pri)

TEMPLATE = lib
TARGET = QtSparql
DESTDIR = $$top_srcdir/lib
DEFINES += QT_BUILD_SPARQL_LIB
DEFINES += QT_NO_USING_NAMESPACE
QT += network

unix:QMAKE_PKGCONFIG_REQUIRES = QtCore QtNetwork

DEFINES += QT_NO_CAST_FROM_ASCII

include(kernel/kernel.pri)
include(drivers/drivers.pri)
include(models/models.pri)
