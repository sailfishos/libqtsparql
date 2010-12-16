include(../../../shared.pri)
TEMPLATE = lib
CONFIG += qt plugin

PLUGIN_TYPE = declarative

target.path = $$QTSPARQL_INSTALL_IMPORTS/QtSparql

QT = core declarative

INSTALLS += target
LIBS += -lQtSparql
DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII
