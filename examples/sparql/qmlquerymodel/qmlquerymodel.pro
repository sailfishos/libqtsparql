include(../sparql-examples.pri)

SOURCES += main.cpp

#QT += sparql #enable this later
QT += gui declarative dbus

copy2build.target = $$QT_BUILD_TREE/examples/sparql/qmlquerymodel/main.qml
copy2build.commands = $$QMAKE_COPY $$PWD/main.qml $$QT_BUILD_TREE/examples/sparql/qmlquerymodel
QMAKE_EXTRA_TARGETS += copy2build

copy2buildhook.depends = copy2build
copy2buildhook.target = Makefile
QMAKE_EXTRA_TARGETS += copy2buildhook

# install # FIXME: install & package examples later
#target.path = $$EXAMPLES_DIR/sparql/qmlquerymodel
#sources.files = $$SOURCES *.h $$RESOURCES $$FORMS qmlquerymodel.pro
#sources.path = $$EXAMPLES_DIR/sparql/qmlquerymodel
#INSTALLS += target sources
