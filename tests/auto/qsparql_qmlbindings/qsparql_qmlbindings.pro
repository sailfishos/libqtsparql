include(../sparqltest.pri)
CONFIG += qt warn_on console depend_includepath
QT += testlib declarative

SOURCES  += tst_qsparql_qmlbindings.cpp

check.depends = $$TARGET
check.commands = ./tst_qsparql_qmlbindings

memcheck.depends = $$TARGET
memcheck.commands = $$VALGRIND $$VALGRIND_OPT ./tst_qsparql_qmlbindings

QMAKE_EXTRA_TARGETS += check memcheck

#QT = sparql # enable this later

install_qml.files = qsparqlconnection.qml qsparqlresultlist.qml qsparqllegacy.qml
install_qml.path = $$PREFIX/lib/$$PACKAGENAME-tests/
INSTALLS += install_qml
