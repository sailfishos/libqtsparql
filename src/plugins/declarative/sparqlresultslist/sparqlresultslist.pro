TARGET	 = sparqlresultslist
SOURCES		= plugin.cpp

DESTDIR = $$QT_BUILD_TREE/imports/QtSparql
target.path = $$[QT_INSTALL_IMPORTS]/QtSparql

include(../declarativebase.pri)
