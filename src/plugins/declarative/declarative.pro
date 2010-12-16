include(../../../shared.pri)

TEMPLATE = subdirs

SUBDIRS += sparqlresultslist


# TODO: the qmldir file needs to be copied
# to $$QT_BUILD_TREE/imports/QSparql

qmldir.files = $$PWD/qmldir
qmldir.path = $$QTSPARQL_INSTALL_IMPORTS/QtSparql

INSTALLS += qmldir

copy2build.commands = $$QMAKE_COPY $$PWD/qmldir $$QT_BUILD_TREE/imports/QtSparql
QMAKE_EXTRA_TARGETS += copy2build

coverage {
	coverage.CONFIG = recursive
	# coverage.recurse = sparqlresultslist
	QMAKE_EXTRA_TARGETS += coverage
}
