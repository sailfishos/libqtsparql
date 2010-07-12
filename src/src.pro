TEMPLATE = subdirs
SUBDIRS = sparql plugins

coverage {
	coverage.CONFIG = recursive
	coverage.recurse = sparql
	QMAKE_EXTRA_TARGETS += coverage
}

