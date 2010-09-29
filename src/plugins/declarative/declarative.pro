TEMPLATE = subdirs

SUBDIRS += resultslist

coverage {
	coverage.CONFIG = recursive
	# coverage.recurse = resultslist
	QMAKE_EXTRA_TARGETS += coverage
}
