TARGET	 = qsparqltracker

HEADERS		= ../../../sparql/drivers/tracker/qsparql_tracker_p.h

SOURCES		= main.cpp \
		  ../../../sparql/drivers/tracker/qsparql_tracker.cpp

unix: {
    LIBS *= $$QT_LFLAGS_TRACKER
    QMAKE_CXXFLAGS *= $$QT_CFLAGS_TRACKER
}

include(../qsparqldriverbase.pri)
QT += dbus

coverage {
	LIBS += -lgcov
	QMAKE_CXXFLAGS += -ftest-coverage -fprofile-arcs -fno-elide-constructors
	QMAKE_EXTRA_TARGETS += coverage
	coverage.commands  = lcov -d . --capture --output-file all.cov -b . &&
	coverage.commands += lcov -e all.cov '*/tracker/*.cpp' -e all.cov '*/drivers/tracker/*.h' -o src.cov &&
	coverage.commands += genhtml -o coverage src.cov || echo "no coverage measures for tracker driver"
        
}
