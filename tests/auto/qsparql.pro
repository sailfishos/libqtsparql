TEMPLATE = subdirs
SUBDIRS = \
    qsparqlquery \
    qsparqlbinding \
    qsparql \
    qsparql_endpoint \
    qsparql_tracker \
    qsparql_tracker_direct \
    qsparql_tracker_signals \
    qsparql_virtuoso \
    qsparql_virtuoso_endpoint

check.CONFIG = recursive
check.recurse = qsparql qsparqlquery qsparqlbinding

QMAKE_EXTRA_TARGETS += check

