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
    coverage.commands = mkdir -p ../../coverage &&
    coverage.commands += lcov -d . --capture --output-file ../../coverage/all-tracker.cov -b . &&
    coverage.commands += cd ../../coverage &&
    coverage.commands += lcov -e all-tracker.cov '*/tracker/*.cpp' -e all-tracker.cov '*/drivers/tracker/*.h' -o src-tracker.cov &&
    coverage.commands += genhtml -o . src-tracker.cov || echo "no coverage measures for tracker driver"
}
