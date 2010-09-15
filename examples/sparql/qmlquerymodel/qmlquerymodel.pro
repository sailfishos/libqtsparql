include(../sparql-examples.pri)

CONFIG += qt plugin

HEADERS += sparqlresultmodel.h
SOURCES += sparqlresultmodel.cpp main.cpp

#QT += sparql #enable this later
QT += network xml gui declarative

# install
target.path = $$EXAMPLES_DIR/sparql/qmlquerymodel
sources.files = $$SOURCES *.h $$RESOURCES $$FORMS qmlquerymodel.pro
sources.path = $$EXAMPLES_DIR/sparql/qmlquerymodel
INSTALLS += target sources
