#####################################################################
# Main projectfile
#####################################################################

include(shared.pri)
CONFIG += ordered
TEMPLATE = subdirs

SUBDIRS = src tests examples

xclean.commands = rm -rf lib plugins include
xclean.depends = clean

QMAKE_EXTRA_TARGETS += xclean

include(doc/doc.pri)

# To measure code coverage:
# qmake CONFIG+=coverage
# run tests
# make coverage
coverage {
	coverage.CONFIG = recursive
	coverage.recurse = src
	QMAKE_EXTRA_TARGETS += coverage
}

