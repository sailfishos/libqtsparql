HEADERS       = ../connection.h \
                customsparqlmodel.h 
SOURCES       = customsparqlmodel.cpp \
                main.cpp
QT           += sparql

# install
target.path = $$[QT_INSTALL_EXAMPLES]/sparql/querymodel
sources.files = $$SOURCES *.h $$RESOURCES $$FORMS querymodel.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/sparql/querymodel
INSTALLS += target sources

symbian: include($$QT_SOURCE_TREE/examples/symbianpkgrules.pri)
