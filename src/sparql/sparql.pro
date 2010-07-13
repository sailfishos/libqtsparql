include(../../shared.pri)

TEMPLATE = lib
CONFIG += create_pc create_prl
TARGET = QtSparql
DESTDIR = $$QT_BUILD_TREE/lib
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

QMAKE_PKGCONFIG_REQUIRES = QtCore QtNetwork
QMAKE_PKGCONFIG_DESTDIR = pkgconfig

coverage {
	LIBS += -lgcov
	QMAKE_CXXFLAGS += -ftest-coverage -fprofile-arcs -fno-elide-constructors
	QMAKE_EXTRA_TARGETS += coverage
	coverage.commands  = lcov -d . --capture --output-file all.cov -b . &&
	coverage.commands += lcov -e all.cov '*/*/src/sparql/*/*.cpp' -e all.cov '*/*/src/sparql/*/*.h' -o src.cov &&
	coverage.commands += genhtml -o coverage src.cov
}
