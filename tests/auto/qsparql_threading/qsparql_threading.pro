include(../sparqltest.pri)
CONFIG += qt warn_on console depend_includepath
QT += testlib xml
SOURCES  += tst_qsparql_threading.cpp

#QT = sparql # enable this later


check.depends = $$TARGET
check.commands = ./tst_qsparql_threading concurrentTrackerQueries && \
                 ./tst_qsparql_threading concurrentTrackerDirectQueries && \
                 ./tst_qsparql_threading concurrentTrackerDirectInserts || \
                 ( i=$? && tracker-sparql -u -f ./clean_data_threading.rq && exit $i)

memcheck.depends = $$TARGET
memcheck.commands = $$VALGRIND $$VALGRIND_OPT \
                    ./tst_qsparql_threading concurrentTrackerQueries && \
                    $$VALGRIND $$VALGRIND_OPT \
                    ./tst_qsparql_threading concurrentTrackerDirectQueries && \
                    $$VALGRIND $$VALGRIND_OPT \
                    ./tst_qsparql_threading concurrentTrackerDirectInserts || \
                    ( i=$? && tracker-sparql -u -f ./clean_data_threading.rq && exit $i)

QMAKE_EXTRA_TARGETS += check memcheck