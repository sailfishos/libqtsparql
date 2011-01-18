TEMPLATE = subdirs
SUBDIRS = \
    qsparqlquery \
    qsparqlbinding \
    qsparql \
    qsparql_endpoint \
    qsparql_ntriples \
    qsparql_threading \
    qsparql_tracker \
    qsparql_tracker_direct \
    qsparql_virtuoso \
    qsparql_virtuoso_endpoint \
    qsparql_benchmark

check.CONFIG = recursive
check.recurse = qsparql qsparqlquery qsparqlbinding

QMAKE_EXTRA_TARGETS += check

