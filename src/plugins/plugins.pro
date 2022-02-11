TEMPLATE = subdirs
SUBDIRS = sparqldrivers
# TODO: revisit the qml api if ever needed
#declarative

coverage {
	coverage.CONFIG = recursive
	coverage.recurse = sparqldrivers
	QMAKE_EXTRA_TARGETS += coverage
}
