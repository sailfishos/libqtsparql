SOURCES      += main.cpp
HEADERS      += main.cpp
QT           += sparql
QT           -= gui

# install
target.path = $$[QT_INSTALL_EXAMPLES]/sparql/asynctracker
sources.files = $$SOURCES *.h $$RESOURCES $$FORMS asynctracker.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/sparql/asynctracker
INSTALLS += target sources

symbian: include($$QT_SOURCE_TREE/examples/symbianpkgrules.pri)
