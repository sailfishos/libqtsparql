doxygen_doc.target = doc/html/index.html
doxygen_doc.commands = cd doc && VERSION=\"$$DOC_VERSION\" LIBRARYNAME=\"$$LIBRARYNAME\" doxygen doxygen.cfg
doc.depends = doxygen_doc

install_doc.files = doc/html/*.html doc/html/*.css doc/html/*.gif doc/html/*.png
install_doc.path = $$QTSPARQL_INSTALL_DOCS/$$PACKAGENAME-doc/html
install_doc.CONFIG = no_check_exist
INSTALLS += install_doc

clean_doc.commands = rm -rf doc/html doc/man
distclean.depends += clean_doc

QMAKE_EXTRA_TARGETS += doc doxygen_doc clean_doc
