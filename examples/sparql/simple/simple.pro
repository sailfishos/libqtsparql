include(../sparql-examples.pri)

SOURCES       = main.cpp
#QT           += sparql # enable this later

# install # FIXME
target.path = $$[QT_INSTALL_EXAMPLES]/sparql/simple
sources.files = $$SOURCES *.h $$RESOURCES $$FORMS simple.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/sparql/simple
INSTALLS += target sources
