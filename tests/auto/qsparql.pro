include(../../shared.pri)

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
    qsparql_tracker_direct_sync \
    qsparql_virtuoso \
    qsparql_virtuoso_endpoint \
    qsparql_benchmark

check.CONFIG = recursive
check.recurse = qsparql qsparqlquery qsparqlbinding

QMAKE_EXTRA_TARGETS += check


testdata.target = qsparql_tracker/testdata_tracker.ttl \
                  qsparql_tracker_direct/testdata_tracker_direct.ttl \
                  qsparql_tracker/clean_data_tracker.rq \
                  qsparql_tracker_direct/clean_data_tracker_direct.rq \
                  qsparql_threading/clean_data_threading.rq

testdata.files = $$testdata.target
testdata.path = $$PREFIX/share/$$PACKAGENAME-tests/
testxml.target = tests.xml
install_testxml.files = $$testxml.target
install_testxml.path = $$PREFIX/share/$$PACKAGENAME-tests/
INSTALLS += target install_testxml testdata
