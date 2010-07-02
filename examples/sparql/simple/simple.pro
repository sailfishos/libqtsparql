SOURCES       = main.cpp
QT           += sparql
QT           -= gui

# install
target.path = $$[QT_INSTALL_EXAMPLES]/sparql/simple
sources.files = $$SOURCES *.h $$RESOURCES $$FORMS simple.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/sparql/simple
INSTALLS += target sources

symbian: include($$QT_SOURCE_TREE/examples/symbianpkgrules.pri)
