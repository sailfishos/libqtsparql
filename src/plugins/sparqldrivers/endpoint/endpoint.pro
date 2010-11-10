TARGET	 = qsparqlendpoint

HEADERS		= ../../../sparql/drivers/endpoint/qsparql_endpoint_p.h
SOURCES		= main.cpp \
		  ../../../sparql/drivers/endpoint/qsparql_endpoint.cpp

unix: {
    LIBS *= $$QT_LFLAGS_ENDPOINT
    QMAKE_CXXFLAGS *= $$QT_CFLAGS_ENDPOINT
}

include(../qsparqldriverbase.pri)

QT += xml
QT += network

coverage {
	LIBS += -lgcov
	QMAKE_CXXFLAGS += -ftest-coverage -fprofile-arcs -fno-elide-constructors
	QMAKE_EXTRA_TARGETS += coverage
	coverage.commands  = lcov -d . --capture --output-file all.cov -b . &&
	coverage.commands += lcov -e all.cov '*/endpoint/*.cpp' -e all.cov '*/endpoint/*.h' -o src.cov &&
	coverage.commands += genhtml -o coverage src.cov || echo "no coverage measures for endpoint driver"
}
