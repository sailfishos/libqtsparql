TEMPLATE = subdirs
SUBDIRS = \
    qsparqlquery \
    qsparql \
    qsparql_tracker

check.CONFIG = recursive
check.recurse = qsparql qsparqlquery

QMAKE_EXTRA_TARGETS += check

