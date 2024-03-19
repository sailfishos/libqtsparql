include(../../../shared.pri)

TEMPLATE = lib
CONFIG += qt plugin

qmldir.files = $$PWD/qmldir
qmldir.path = $$QTSPARQL_INSTALL_IMPORTS/QtSparql
INSTALLS += qmldir

#copy2build.target = $$QTSPARQL_BUILD_TREE/imports/QtSparql/qmldir
#copy2build.commands = $$QMAKE_COPY $$PWD/qmldir $$QTSPARQL_BUILD_TREE/imports/QtSparql
#copy2build.depends = $$PWD/qmldir
#QMAKE_EXTRA_TARGETS += copy2build

# This doesn't seem to do anythig
# copy2build.CONFIG += target_predeps
#
# So to get the qmldir copied into the build area, before doing
# anything else we need this trickery.
#copy2buildhook.depends = copy2build
#copy2buildhook.target = Makefile
#QMAKE_EXTRA_TARGETS += copy2buildhook

coverage {
	coverage.CONFIG = recursive
	# coverage.recurse = sparqlresultslist
	QMAKE_EXTRA_TARGETS += coverage
}

target.path = $$QTSPARQL_INSTALL_IMPORTS/QtSparql

QT = core qml

INSTALLS += target
LIBS += -lQt5Sparql
DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII

DESTDIR = $$QTSPARQL_BUILD_TREE/imports/QtSparql

TARGET = sparqlplugin

SOURCES += \
    plugin.cpp \
    declarativesparqlconnection.cpp \
    declarativesparqllistmodel.cpp

HEADERS += \
    declarativesparqlconnection.h \
    declarativesparqlconnectionoptions.h \
    declarativesparqllistmodel.h

