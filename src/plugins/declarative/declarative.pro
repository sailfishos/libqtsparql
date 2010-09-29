TEMPLATE = subdirs

SUBDIRS += sparqlresultslist

coverage {
	coverage.CONFIG = recursive
	# coverage.recurse = sparqlresultslist
	QMAKE_EXTRA_TARGETS += coverage
}
