include(../../shared.pri)

TEMPLATE = lib
CONFIG += create_pc create_prl
TARGET = QtSparql
DESTDIR = $$top_srcdir/lib
DEFINES += QT_BUILD_SPARQL_LIB
DEFINES += QT_NO_USING_NAMESPACE
QT += network

DEFINES += QT_NO_CAST_FROM_ASCII

install_headers.path = $$PREFIX/include/QtSparql
install_headers.files = 

include(kernel/kernel.pri)
include(drivers/drivers.pri)
include(models/models.pri)

INSTALLS += \
        target \
        install_headers
target.path = $$PREFIX/lib

DESTDIR = ../../lib

QMAKE_PKGCONFIG_REQUIRES = QtCore QtNetwork
QMAKE_PKGCONFIG_DESTDIR = pkgconfig

