include(../sparql-examples.pri)

SOURCES       = main.cpp
#QT           += sparql # enable this later

# install FIXME
target.path = $$[QT_INSTALL_EXAMPLES]/sparql/bindingset
sources.files = $$SOURCES *.h $$RESOURCES $$FORMS bindingset.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/sparql/bindingset
INSTALLS += target sources
