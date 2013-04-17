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
    qsparql_api \
    qsparql_qmlbindings

contains(sparql-plugins, tracker_direct): SUBDIRS += qsparql_benchmark

QSPARQL_TESTS = qsparql qsparqlquery qsparqlbinding qsparql_api qsparql_tracker \
                qsparql_tracker_direct qsparql_tracker_direct_sync qsparql_ntriples \
                qsparql_tracker_direct_crashes qsparql_threading \
                qsparqlresultrow qsparql_qmlbindings qsparql_endpoint

check.CONFIG = recursive
check.recurse = $$QSPARQL_TESTS

memcheck.CONFIG = recursive
memcheck.recurse = $$QSPARQL_TESTS

QMAKE_EXTRA_TARGETS += check memcheck

testxml.target = tests.xml
equals(QT_MAJOR_VERSION, 4): install_testxml.files = $$testxml.target
equals(QT_MAJOR_VERSION, 5): install_testxml.files = qt5/$$testxml.target
install_testxml.path = $$PREFIX/share/$$PACKAGENAME-tests/
INSTALLS += target install_testxml
