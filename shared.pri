INCLUDEPATH += \
    $$QTSPARQL_BUILD_TREE/include \
    $$QTSPARQL_BUILD_TREE/include/Qt5Sparql/

QT = core

QMAKE_LIBDIR = $$QTSPARQL_BUILD_TREE/$$QTSPARQL_INSTALL_LIB
isEmpty(PREFIX): PREFIX = $$QTSPARQL_INSTALL_PREFIX

# this will be in the .so name
VERSION = 0.2.6

# for documentation
DOC_VERSION = 0.2.6
LIBRARYNAME = Qt5Sparql
PACKAGENAME = libqt5sparql
