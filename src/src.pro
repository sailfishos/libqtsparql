TEMPLATE = subdirs
SUBDIRS = sparql plugins
plugins.depends = sparql

coverage {
	coverage.CONFIG = recursive
	coverage.recurse = sparql plugins
	QMAKE_EXTRA_TARGETS += coverage
}

