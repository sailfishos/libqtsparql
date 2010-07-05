TEMPLATE = subdirs

SUBDIRS = tracker endpoint
# FIXME: conditionally build drivers
#contains(sparql-plugins, tracker)	: SUBDIRS += tracker
#contains(sparql-plugins, virtuoso)	: SUBDIRS += virtuoso
#contains(sparql-plugins, endpoint)	: SUBDIRS += endpoint
