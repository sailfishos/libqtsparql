TARGET   = qsparqltrackerdirect
CONFIG += no_keywords

HEADERS         = ../../../sparql/drivers/tracker_direct/qsparql_tracker_direct_p.h \
                  ../../../sparql/drivers/tracker_direct/qsparql_tracker_direct_driver_p.h \
                  ../../../sparql/drivers/tracker_direct/qsparql_tracker_direct_result_p.h \
                  ../../../sparql/drivers/tracker_direct/qsparql_tracker_direct_select_result_p.h \
                  ../../../sparql/drivers/tracker_direct/qsparql_tracker_direct_sync_result_p.h \
                  ../../../sparql/drivers/tracker_direct/qsparql_tracker_direct_update_result_p.h
SOURCES         = main.cpp \
                  ../../../sparql/drivers/tracker_direct/qsparql_tracker_direct_driver_p.cpp \
                  ../../../sparql/drivers/tracker_direct/qsparql_tracker_direct_result_p.cpp \
                  ../../../sparql/drivers/tracker_direct/qsparql_tracker_direct_select_result_p.cpp \
                  ../../../sparql/drivers/tracker_direct/qsparql_tracker_direct_sync_result_p.cpp \
                  ../../../sparql/drivers/tracker_direct/qsparql_tracker_direct_update_result_p.cpp

unix: {
    CONFIG += link_pkgconfig
    PKGCONFIG += tracker-sparql-1.0
}

include(../qsparqldriverbase.pri)

coverage {
    LIBS += -lgcov
    QMAKE_CXXFLAGS += -ftest-coverage -fprofile-arcs -fno-elide-constructors
    QMAKE_EXTRA_TARGETS += coverage
    coverage.commands = mkdir -p ../../coverage &&
    coverage.commands += lcov -d . --capture --output-file ../../coverage/all-direct.cov -b . &&
    coverage.commands += cd ../../coverage &&
    coverage.commands += lcov -e all-direct.cov '*/tracker_direct/*.cpp' -e all-direct.cov '*/drivers/tracker_direct/*.h' -o src-direct.cov &&
    coverage.commands += genhtml -o . src-direct.cov || echo "no coverage measures for tracker direct driver"

}
