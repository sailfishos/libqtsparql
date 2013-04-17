include(../../../shared.pri)
TEMPLATE = lib
CONFIG += qt plugin
QT = core
DESTDIR = $$QTSPARQL_BUILD_TREE/plugins/sparqldrivers
equals(QT_MAJOR_VERSION, 4): LIBS += -lQtSparql
equals(QT_MAJOR_VERSION, 5): LIBS += -lQt5Sparql

target.path = $$QTSPARQL_INSTALL_PLUGINS/sparqldrivers

INSTALLS        += target

DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII

CONFIG += hide_symbols
