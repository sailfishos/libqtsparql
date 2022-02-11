TEMPLATE      = subdirs

SUBDIRS       = simple \
                # qmlbindings \
                iteration \
                asynctracker \

# QWidget based examples disabled for now:
#   dbpedia \
#   qmlquerymodel \
#   querymodel \
#   contacts


# install -- FIXME: install + package examples later
#sources.files = sparql.pro README
#sources.path = $$[QT_INSTALL_EXAMPLES]/sparql
#INSTALLS += sources
