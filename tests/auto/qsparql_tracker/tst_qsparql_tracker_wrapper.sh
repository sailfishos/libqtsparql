#!/bin/bash

tracker-import testdata_tracker.ttl

if [ $? -ne 0 ] ; then
        echo "Could not import test data"
        exit $?
fi

./tst_qsparql_tracker
er=$?

tracker-sparql -u -f clean_data_tracker.rq

exit $er
