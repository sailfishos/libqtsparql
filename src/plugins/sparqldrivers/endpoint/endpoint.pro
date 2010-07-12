TARGET	 = qsparqlendpoint

HEADERS		= ../../../sparql/drivers/endpoint/qsparql_endpoint.h
SOURCES		= main.cpp \
		  ../../../sparql/drivers/endpoint/qsparql_endpoint.cpp

QT += xml
QT += network

unix: {
    LIBS *= $$QT_LFLAGS_ENDPOINT
    QMAKE_CXXFLAGS *= $$QT_CFLAGS_ENDPOINT
}

include(../qsparqldriverbase.pri)

coverage {
	LIBS += -lgcov
	QMAKE_CXXFLAGS += -ftest-coverage -fprofile-arcs -fno-elide-constructors
	QMAKE_EXTRA_TARGETS += coverage
	coverage.commands  = lcov -d . --capture --output-file all.cov -b . &&
	coverage.commands += lcov -e all.cov '*/*/src/plugins/sparqldrivers/endpoint/*.cpp' -e all.cov '*/*/src/plugins/sparqldrivers/endpoint/*.h' -o src.cov &&
	coverage.commands += genhtml -o coverage src.cov || echo "no coverage measures for endpoint driver"
}
