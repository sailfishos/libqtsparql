doxygen_doc.target = doc/html/index.html
doxygen_doc.commands = cd doc && VERSION=\"$$VERSION\" LIBRARYNAME=\"$$LIBRARYNAME\" doxygen doxygen.cfg
doc.depends = doxygen_doc

install_doc.files = doc/html
install_doc.path = $$PREFIX/share/doc/$$PACKAGENAME-doc/
install_doc.CONFIG = no_check_exist
INSTALLS += install_doc

clean_doc.commands = rm -rf doc/html doc/man
distclean.depends += clean_doc

QMAKE_EXTRA_TARGETS += doc doxygen_doc clean_doc
