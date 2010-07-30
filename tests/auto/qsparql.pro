TEMPLATE = subdirs
SUBDIRS = \
    qsparqlquery \
    qsparqlbinding \
    qsparql \
    qsparql_endpoint \
    qsparql_tracker \
    qsparql_virtuoso

check.CONFIG = recursive
check.recurse = qsparql qsparqlquery qsparqlbinding

QMAKE_EXTRA_TARGETS += check

