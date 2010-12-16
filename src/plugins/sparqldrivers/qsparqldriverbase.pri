include(../../../shared.pri)
TEMPLATE = lib
CONFIG += qt plugin
QT = core
DESTDIR = $$QT_BUILD_TREE/plugins/sparqldrivers
LIBS += -lQtSparql

target.path = $$QTSPARQL_INSTALL_PLUGINS/sparqldrivers

INSTALLS        += target

DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII
