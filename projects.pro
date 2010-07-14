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
# 1) build the plugins into the libqtsparql.so
# ./configure -qt-sparql-endpoint -qt-sparql-tracker
# 2) compile with coverage flags
# qmake CONFIG+=coverage
# make
# 3) run tests
# 4) create coverage reports
# make coverage
coverage {
	coverage.CONFIG = recursive
	coverage.recurse = src
	QMAKE_EXTRA_TARGETS += coverage
}

