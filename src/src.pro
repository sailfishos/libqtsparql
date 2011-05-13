TEMPLATE = subdirs
SUBDIRS = sparql plugins
plugins.depends = sparql

coverage {
	coverage.CONFIG = recursive
	coverage.recurse = sparql plugins
	QMAKE_EXTRA_TARGETS += coverage
}

memcheck.command = echo 'nothing' > /dev/null
QMAKE_EXTRA_TARGETS += memcheck
