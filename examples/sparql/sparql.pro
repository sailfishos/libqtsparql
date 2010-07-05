TEMPLATE      = subdirs

SUBDIRS       = simple \
#               dbpedia \ # enable when querymodel is enabled
#                querymodel \ # -..-
                bindingset \
                asynctracker

# install-- FIXME
sources.files = sparql.pro README
sources.path = $$[QT_INSTALL_EXAMPLES]/sparql
INSTALLS += sources
