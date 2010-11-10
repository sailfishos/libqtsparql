include(../../../shared.pri)

TEMPLATE = subdirs

SUBDIRS += sparqlresultslist


# TODO: the qmldir file needs to be copied
# to $$QT_BUILD_TREE/imports/QSparql

qmldir.files = $$PWD/qmldir

isEmpty(QTSPARQL_INSTALL_IMPORTS) {
    qmldir.path = $$[QT_INSTALL_IMPORTS]/QtSparql
} else {
    qmldir.path = $$QTSPARQL_INSTALL_IMPORTS/QtSparql
}

INSTALLS += qmldir

mytarget.commands = @echo ImportsVariable $$QTSPARQL_INSTALL_IMPORTS
QMAKE_EXTRA_TARGETS += mytarget

coverage {
	coverage.CONFIG = recursive
	# coverage.recurse = sparqlresultslist
	QMAKE_EXTRA_TARGETS += coverage
}
