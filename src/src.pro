TEMPLATE = subdirs
SUBDIRS = sparql plugins

coverage {
	coverage.CONFIG = recursive
	coverage.recurse = sparql plugins
	QMAKE_EXTRA_TARGETS += coverage
}

