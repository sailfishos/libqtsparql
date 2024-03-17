include(../../shared.pri)

TEMPLATE = subdirs
SUBDIRS = \
    qsparqlquery \
    qsparqlbinding \
    qsparqlresultrow \
    qsparql \
    qsparql_endpoint \
    qsparql_ntriples \
    qsparql_tracker_direct_concurrency \
    qsparql_tracker_direct_crashes \
    qsparql_virtuoso \
    qsparql_virtuoso_endpoint

# can be enabled if qml api is brought back
# SUBDIRS += qsparql_qmlbindings

# tests need to be migrated away from removed QTRACKER backend and use Tracker3 style sparql with graphs.
#    qsparql_api \
#    qsparql_tracker_direct \
#    qsparql_tracker_direct_sync \


contains(sparql-plugins, tracker_direct): SUBDIRS += qsparql_benchmark

QSPARQL_TESTS = \
    qsparql \
    qsparqlquery \
    qsparqlbinding \
    qsparql_tracker \
    qsparql_ntriples \
    qsparqlresultrow \
    qsparql_endpoint

# this skipped, needs a setup where connection fails
#     qsparql_tracker_direct_crashes \

# can be enabled when qml api is back, but needs also fixing
# qsparql_qmlbindings \

# skipped due to tracker migration as above
#    qsparql_api \
#    qsparql_tracker_direct \
#    qsparql_tracker_direct_sync \

check.CONFIG = recursive
check.recurse = $$QSPARQL_TESTS

memcheck.CONFIG = recursive
memcheck.recurse = $$QSPARQL_TESTS

QMAKE_EXTRA_TARGETS += check memcheck

testxml.target = tests.xml
install_testxml.files = qt5/$$testxml.target
install_testxml.CONFIG = no_check_exist
install_testxml.commands = \
        $$QMAKE_STREAM_EDITOR 's~@LIBDIR@~$$[QT_INSTALL_LIBS]~' qt5/$${testxml.target}.in > qt5/$$testxml.target
install_testxml.path = $$PREFIX/share/$$PACKAGENAME-tests/
INSTALLS += install_testxml
