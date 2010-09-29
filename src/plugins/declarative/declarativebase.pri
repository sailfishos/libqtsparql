include(../../../shared.pri)
TEMPLATE = lib
CONFIG += qt plugin
QT = core declarative
INSTALLS        += target
LIBS += -lQtSparql
DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII
