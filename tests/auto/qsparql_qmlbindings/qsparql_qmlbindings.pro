include(../sparqltest.pri)
CONFIG += qt warn_on console depend_includepath
# Without 'gui' QTEST_MAIN() would instantiate QCoreApplication instead of
# QApplication which is needed for the declarative stuff to work.
QT += testlib gui qml

SOURCES  += tst_qsparql_qmlbindings.cpp

check.depends = $$TARGET
check.commands = ./tst_qsparql_qmlbindings

memcheck.depends = $$TARGET
memcheck.commands = $$VALGRIND $$VALGRIND_OPT ./tst_qsparql_qmlbindings

QMAKE_EXTRA_TARGETS += check memcheck

#QT = sparql # enable this later

install_qml.files = qsparqlconnection-qt5.qml qsparqlresultlist-qt5.qml qsparqllegacy-qt5.qml
install_qml.path = $$PREFIX/$$QTSPARQL_INSTALL_LIB/$$PACKAGENAME-tests/
INSTALLS += install_qml
