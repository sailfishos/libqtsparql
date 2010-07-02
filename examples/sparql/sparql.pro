TEMPLATE      = subdirs

SUBDIRS       = simple \
                dbpedia \
                querymodel \
                bindingset \
                asynctracker

# install
sources.files = sparql.pro README
sources.path = $$[QT_INSTALL_EXAMPLES]/sparql
INSTALLS += sources

symbian: include($$QT_SOURCE_TREE/examples/symbianpkgrules.pri)
