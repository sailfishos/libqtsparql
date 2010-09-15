include(../sparql-examples.pri)
HEADERS       = ../connection.h \
                customsparqlmodel.h 
SOURCES       = customsparqlmodel.cpp \
                main.cpp
#QT           += sparql #enable this later
QT += gui
# install
target.path = $$EXAMPLES_DIR/sparql/querymodel
sources.files = $$SOURCES *.h $$RESOURCES $$FORMS querymodel.pro
sources.path = $$EXAMPLES_DIR/sparql/querymodel
INSTALLS += target sources

symbian: include($$QT_SOURCE_TREE/examples/symbianpkgrules.pri)
