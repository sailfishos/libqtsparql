include(../sparql-examples.pri)

SOURCES       = main.cpp
#QT           += sparql # enable this later

# install FIXME
target.path = $$EXAMPLES_DIR/sparql/iteration
sources.files = $$SOURCES *.h $$RESOURCES $$FORMS iteration.pro
sources.path = $$EXAMPLES_DIR/sparql/iteration
INSTALLS += target sources
