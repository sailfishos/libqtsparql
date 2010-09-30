TARGET	 = sparqlresultslist
SOURCES		= plugin.cpp

DESTDIR = $$QT_BUILD_TREE/imports/QSparql
target.path = $$QTSPARQL_INSTALL_LIBS/imports/QSparql

include(../declarativebase.pri)
