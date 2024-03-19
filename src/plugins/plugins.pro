TEMPLATE = subdirs
SUBDIRS = sparqldrivers declarative

coverage {
	coverage.CONFIG = recursive
	coverage.recurse = sparqldrivers
	QMAKE_EXTRA_TARGETS += coverage
}
