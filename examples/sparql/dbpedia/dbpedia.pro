include(../sparql-examples.pri)

HEADERS += sparqlquerytext.h
SOURCES += main.cpp sparqlquerytext.cpp

#QT += sparql #enable this later
QT += network xml gui

# install
target.path = $$EXAMPLES_DIR/sparql/dbpedia
sources.files = $$SOURCES *.h $$RESOURCES $$FORMS dbpedia.pro
sources.path = $$EXAMPLES_DIR/sparql/dbpedia
INSTALLS += target sources

