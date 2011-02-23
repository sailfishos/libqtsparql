#!/bin/bash

tracker-import testdata_tracker_direct.ttl

if [ $? -ne 0 ] ; then
        echo "Could not import test data"
        exit $?
fi

./tst_qsparql_tracker_direct
er=$?

tracker-sparql -u -f clean_data_tracker_direct.rq

exit $er
