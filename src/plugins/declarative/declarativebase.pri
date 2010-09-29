include(../../../shared.pri)
TEMPLATE = lib
CONFIG += qt plugin
QT = core
DESTDIR = $$QT_BUILD_TREE/plugins/declarative

isEmpty($$QTSPARQL_INSTALL_PLUGINS) {
    target.path = $$[QT_INSTALL_PLUGINS]/declarative
} else {
    target.path = $$QTSPARQL_INSTALL_PLUGINS/declarative
}
INSTALLS        += target

DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII
