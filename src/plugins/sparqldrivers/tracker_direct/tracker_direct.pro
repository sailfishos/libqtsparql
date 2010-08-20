TARGET	 = qsparqltrackerdirect

HEADERS		= ../../../sparql/drivers/tracker_direct/qsparql_tracker_direct.h \
                  ../../../sparql/drivers/tracker_direct/qsparql_tracker_direct_p.h
SOURCES		= main.cpp \
		  ../../../sparql/drivers/tracker_direct/qsparql_tracker_direct.cpp

unix: {
    LIBS *= $$QT_LFLAGS_TRACKER_DIRECT
    QMAKE_CXXFLAGS *= $$QT_CFLAGS_TRACKER_DIRECT
}

include(../qsparqldriverbase.pri)

coverage {
	LIBS += -lgcov
	QMAKE_CXXFLAGS += -ftest-coverage -fprofile-arcs -fno-elide-constructors
	QMAKE_EXTRA_TARGETS += coverage
	coverage.commands  = lcov -d . --capture --output-file all.cov -b . &&
	coverage.commands += lcov -e all.cov '*/tracker_direct/*.cpp' -e all.cov '*/drivers/tracker_direct/*.h' -o src.cov &&
	coverage.commands += genhtml -o coverage src.cov || echo "no coverage measures for tracker direct driver"
        
}
