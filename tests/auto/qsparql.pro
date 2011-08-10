include(../../shared.pri)

TEMPLATE = subdirs
SUBDIRS = \
    qsparqlquery \
    qsparqlbinding \
    qsparqlresultrow \
    qsparql \
    qsparql_endpoint \
    qsparql_ntriples \
    qsparql_threading \
    qsparql_tracker \
    qsparql_tracker_direct \
    qsparql_tracker_direct_concurrency \
    qsparql_tracker_direct_sync \
    qsparql_tracker_direct_crashes \
    qsparql_virtuoso \
    qsparql_virtuoso_endpoint \
    qsparql_api

contains(sparql-plugins, tracker_direct): SUBDIRS += qsparql_benchmark

check.CONFIG = recursive
check.recurse = qsparql_api qsparql qsparqlquery qsparqlbinding qsparql_tracker \
                qsparql_tracker_direct qsparql_tracker_direct_sync qsparql_ntriples \
                qsparql_tracker_direct_concurrency

memcheck.CONFIG = recursive
memcheck.recurse = qsparql_api qsparql_tracker qsparql_tracker_direct qsparql_direct_sync \
                   qsparql_direct_crashes

QMAKE_EXTRA_TARGETS += check memcheck


testdata.target = qsparql_threading/clean_data_threading.rq

testdata.files = $$testdata.target
testdata.path = $$PREFIX/share/$$PACKAGENAME-tests/
testxml.target = tests.xml
install_testxml.files = $$testxml.target
install_testxml.path = $$PREFIX/share/$$PACKAGENAME-tests/
INSTALLS += target install_testxml testdata
