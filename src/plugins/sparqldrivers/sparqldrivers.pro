TEMPLATE = subdirs

contains(sparql-plugins, tracker)	: SUBDIRS += tracker
contains(sparql-plugins, tracker_direct): SUBDIRS += tracker_direct
contains(sparql-plugins, virtuoso)	: SUBDIRS += virtuoso
contains(sparql-plugins, endpoint)	: SUBDIRS += endpoint

coverage {
	coverage.CONFIG = recursive
	coverage.recurse = tracker endpoint
	QMAKE_EXTRA_TARGETS += coverage
}
