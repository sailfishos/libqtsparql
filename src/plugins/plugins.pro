TEMPLATE = subdirs
SUBDIRS = sparqldrivers

coverage {
	coverage.CONFIG = recursive
	coverage.recurse = sparqldrivers
	QMAKE_EXTRA_TARGETS += coverage
}
