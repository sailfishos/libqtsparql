include(../sparql-examples.pri)

HEADERS += sparqlquerytext.h
SOURCES += main.cpp sparqlquerytext.cpp

#QT += sparql #enable this later
QT += network xml gui

# install
target.path = $$[QT_INSTALL_EXAMPLES]/sparql/dbpedia
sources.files = $$SOURCES *.h $$RESOURCES $$FORMS dbpedia.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/sparql/dbpedia
INSTALLS += target sources

