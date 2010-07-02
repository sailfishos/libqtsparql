SOURCES       = main.cpp
QT           += sparql
QT           -= gui

# install
target.path = $$[QT_INSTALL_EXAMPLES]/sparql/bindingset
sources.files = $$SOURCES *.h $$RESOURCES $$FORMS bindingset.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/sparql/bindingset
INSTALLS += target sources

symbian: include($$QT_SOURCE_TREE/examples/symbianpkgrules.pri)
