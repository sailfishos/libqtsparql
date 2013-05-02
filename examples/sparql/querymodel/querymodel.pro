include(../sparql-examples.pri)
HEADERS       = customsparqlmodel.h
SOURCES       = customsparqlmodel.cpp \
                main.cpp
#QT           += sparql #enable this later
QT += gui
equals(QT_MAJOR_VERSION, 5): QT += widgets
# install # FIXME: install + package examples later
#target.path = $$EXAMPLES_DIR/sparql/querymodel
#sources.files = $$SOURCES *.h $$RESOURCES $$FORMS querymodel.pro
#sources.path = $$EXAMPLES_DIR/sparql/querymodel
#INSTALLS += target sources
