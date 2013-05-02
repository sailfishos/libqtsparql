#####################################################################
# Main projectfile
#####################################################################

include(shared.pri)

lessThan(QT_MAJOR_VERSION, 5) {
    lessThan(QT_MAJOR_VERSION, 4) | lessThan(QT_MINOR_VERSION, 7) {
       error("QtSparql requires Qt 4.7 or newer but Qt $$[QT_VERSION] was detected.")
    }
}

TEMPLATE = subdirs

CONFIG += ordered
SUBDIRS = src tests examples

xclean.commands = rm -rf lib plugins include
xclean.depends = clean

include(doc/doc.pri)

check.CONFIG = recursive
check.recurse = tests

memcheck.CONFIG = recursive
memcheck.recurse = src tests

QMAKE_EXTRA_TARGETS += xclean memcheck

# To measure code coverage:
# 1) enable the coverage target - that implies building the plugins into the libqtsparql.so
# ./configure --enable-coverage
# 2) compile project
# make
# 3) run tests
# make check (unit tests)
# ...
# 4) create coverage reports
# make coverage
coverage {
    coverage.command = rm -rf coverage
    coverage.CONFIG = recursive
    coverage.recurse = src
    QMAKE_EXTRA_TARGETS += coverage
    clean.commands += rm -rf coverage src/sparql/*.gcno src/sparql/*.gcda src/sparql/*.gcov
    QMAKE_EXTRA_TARGETS += clean
}
