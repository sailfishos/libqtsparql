include(../../shared.pri)

TEMPLATE = lib
CONFIG += create_pc create_prl
equals(QT_MAJOR_VERSION, 4): TARGET = QtSparql
equals(QT_MAJOR_VERSION, 5): TARGET = Qt5Sparql
DESTDIR = $$QTSPARQL_BUILD_TREE/lib
DEFINES += QT_BUILD_SPARQL_LIB
DEFINES += QT_NO_USING_NAMESPACE
QT += network

CONFIG += hide_symbols no_keywords

DEFINES += QT_NO_CAST_FROM_ASCII

install_headers.path = $$QTSPARQL_INSTALL_HEADERS
install_headers.files = 

include($$QTSPARQL_BUILD_TREE/include/$$TARGET/headers.pri)

include_headers.files = $$SYNCQT.HEADER_FILES
include_headers.path = $$QTSPARQL_INSTALL_HEADERS
INSTALLS += include_headers

class_headers.files = $$SYNCQT.HEADER_CLASSES
class_headers.path = $$QTSPARQL_INSTALL_HEADERS
INSTALLS += class_headers

# The following creates a mkspecs/features directory at make time, instead of at install time,
# so that it doesn't end up with root permissions in the build area
create_mkspecs_dir.commands = $$QMAKE_MKDIR $$QTSPARQL_BUILD_TREE/mkspecs/; $$QMAKE_MKDIR $$QTSPARQL_BUILD_TREE/mkspecs/features
create_mkspecs_dir.target = $$QTSPARQL_BUILD_TREE/mkspecs/features
QMAKE_EXTRA_TARGETS += create_mkspecs_dir
create_mkspecs_dirhook.depends = create_mkspecs_dir
create_mkspecs_dirhook.target = Makefile
QMAKE_EXTRA_TARGETS += create_mkspecs_dirhook

equals(QT_MAJOR_VERSION, 4): PRF_INPUT_FILE=$$QTSPARQL_SOURCE_TREE/mkspecs/features/qtsparql.prf.in
equals(QT_MAJOR_VERSION, 5): PRF_INPUT_FILE=$$QTSPARQL_SOURCE_TREE/mkspecs/features/qt5/qtsparql.prf.in
PRF_OUTPUT_FILE=$$QTSPARQL_BUILD_TREE/mkspecs/features/qtsparql.prf
install_prf.path = $$[QT_INSTALL_DATA]/mkspecs/features
install_prf.files = $$PRF_OUTPUT_FILE
install_prf.commands = \
    sed "\"s,QTSPARQL_INSTALL_HEADERS,$$QTSPARQL_INSTALL_HEADERS,g;s,QTSPARQL_INSTALL_LIBS,$$QTSPARQL_INSTALL_LIBS,g\"" $$PRF_INPUT_FILE > $$PRF_OUTPUT_FILE
install_prf.CONFIG = no_check_exist

include(kernel/kernel.pri)
include(drivers/drivers.pri)
include(models/models.pri)
include(declarative/declarative.pri)

INSTALLS += \
        target \
        install_headers \
        install_prf
target.path = $$QTSPARQL_INSTALL_LIBS

QMAKE_PKGCONFIG_PREFIX = $$QTSPARQL_INSTALL_PREFIX
equals(QT_MAJOR_VERSION, 4): QMAKE_PKGCONFIG_REQUIRES = QtCore QtNetwork
equals(QT_MAJOR_VERSION, 5): QMAKE_PKGCONFIG_REQUIRES = Qt5Core Qt5Network
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_CFLAGS = -I$$QTSPARQL_INSTALL_HEADERS
equals(QT_MAJOR_VERSION, 4): QMAKE_PKGCONFIG_NAME = QtSparql
equals(QT_MAJOR_VERSION, 5): QMAKE_PKGCONFIG_NAME = Qt5Sparql
QMAKE_PKGCONFIG_DESCRIPTION = "Library for accessing RDF stores."

coverage {
    LIBS += -lgcov
    QMAKE_CXXFLAGS += -ftest-coverage -fprofile-arcs -fno-elide-constructors
    QMAKE_EXTRA_TARGETS += coverage
    coverage.commands = mkdir -p ../../coverage &&
    coverage.commands += lcov -d . --capture --output-file ../../coverage/all.cov -b . &&
    coverage.commands += cd ../../coverage &&
    coverage.commands += lcov -e all.cov '*/*/src/sparql/*/*.cpp' -e all.cov '*/*/src/sparql/*/*.h' -o src.cov &&
    coverage.commands += genhtml -o . src.cov
}
