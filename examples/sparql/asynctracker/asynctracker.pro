include(../sparql-examples.pri)

SOURCES      += main.cpp
HEADERS      += main.cpp
#QT           += sparql # enable this later

# install # FIXME
target.path = $$EXAMPLES_DIR/sparql/asynctracker
sources.files = $$SOURCES *.h $$RESOURCES $$FORMS asynctracker.pro
sources.path = $$EXAMPLES_DIR/sparql/asynctracker
INSTALLS += target sources

