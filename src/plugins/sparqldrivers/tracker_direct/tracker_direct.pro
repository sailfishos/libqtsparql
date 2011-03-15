TARGET   = qsparqltrackerdirect

HEADERS         = ../../../sparql/drivers/tracker_direct/qsparql_tracker_direct_p.h
SOURCES         = main.cpp \
                  ../../../sparql/drivers/tracker_direct/qsparql_tracker_direct.cpp

unix: {
    CONFIG += link_pkgconfig
    PKGCONFIG += tracker-sparql-0.10
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
