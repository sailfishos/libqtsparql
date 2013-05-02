include(../../../shared.pri)
TEMPLATE = lib
CONFIG += qt plugin

equals(QT_MAJOR_VERSION, 4): PLUGIN_TYPE = declarative

target.path = $$QTSPARQL_INSTALL_IMPORTS/QtSparql

QT = core
equals(QT_MAJOR_VERSION, 4): QT *= declarative
equals(QT_MAJOR_VERSION, 5): {
    QT *= qml
    DEFINES *= QT_VERSION_5
}

INSTALLS += target
equals(QT_MAJOR_VERSION, 4): LIBS += -lQtSparql
equals(QT_MAJOR_VERSION, 5): LIBS += -lQt5Sparql
DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII

DESTDIR = $$QTSPARQL_BUILD_TREE/imports/QtSparql
