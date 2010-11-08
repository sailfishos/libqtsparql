TEMPLATE = subdirs

SUBDIRS += sparqlresultslist

# TODO: the qmldir file needs to be copied
# to $$QT_BUILD_TREE/imports/QSparql

qmldir.files = $$PWD/qmldir
qmldir.path = $$[QT_INSTALL_IMPORTS]/QtSparql
INSTALLS += qmldir

coverage {
	coverage.CONFIG = recursive
	# coverage.recurse = sparqlresultslist
	QMAKE_EXTRA_TARGETS += coverage
}
