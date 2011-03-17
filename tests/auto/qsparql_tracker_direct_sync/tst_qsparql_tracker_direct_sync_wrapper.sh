#!/bin/bash

tracker-import ../qsparql_tracker_direct/testdata_tracker_direct.ttl

if [ $? -ne 0 ] ; then
        echo "Could not import test data"
        exit $?
fi

./tst_qsparql_tracker_direct_sync
er=$?

tracker-sparql -u -f ../qsparql_tracker_direct/clean_data_tracker_direct.rq

exit $er
