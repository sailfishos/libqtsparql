HEADERS += sparqlquerytext.h
SOURCES += main.cpp sparqlquerytext.cpp

QT += sparql
QT += network
QT += xml

# install
target.path = $$[QT_INSTALL_EXAMPLES]/sparql/dbpedia
sources.files = $$SOURCES *.h $$RESOURCES $$FORMS dbpedia.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/sparql/dbpedia
INSTALLS += target sources

symbian: include($$QT_SOURCE_TREE/examples/symbianpkgrules.pri)
