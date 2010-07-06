include(../../../shared.pri)
TEMPLATE = lib
CONFIG += qt plugin
QT = core
DESTDIR = $$top_srcdir/plugins/sparqldrivers

target.path     += $$[QT_INSTALL_PLUGINS]/sparqldrivers
INSTALLS        += target

DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII
