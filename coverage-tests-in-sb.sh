#!/bin/sh

./configure -qt-sparql-endpoint -qt-sparql-tracker -qt-sparql-tracker_direct
qmake CONFIG+=coverage
make || exit 1

make check || exit 1

tracker-control -r > /dev/null
tracker-import tests/auto/qsparql_tracker/testdata_tracker.ttl
tests/auto/qsparql_tracker/tst_qsparql_tracker || exit 1

tracker-control -r > /dev/null
tracker-import tests/auto/qsparql_tracker_direct/testdata_tracker_direct.ttl
tests/auto/qsparql_tracker_direct/tst_qsparql_tracker_direct || exit 1

tracker-control -r > /dev/null
tests/auto/qsparql_threading/tst_qsparql_threading concurrentTrackerQueries || exit 1
tests/auto/qsparql_threading/tst_qsparql_threading concurrentTrackerDirectQueries || exit 1
tests/auto/qsparql_threading/tst_qsparql_threading concurrentTrackerDirectInserts || exit 1

tracker-control -r > /dev/null
(cd tests/auto/qsparql_ntriples/ ; ./tst_qsparql_ntriples) || exit 1

make coverage
echo "Coverage results can be found in src/sparql/coverage/"
