TEMPLATE      = subdirs

SUBDIRS       = simple \
                dbpedia \
                querymodel \
                iteration \
                asynctracker \
                contacts

# install-- FIXME
sources.files = sparql.pro README
sources.path = $$[QT_INSTALL_EXAMPLES]/sparql
INSTALLS += sources
