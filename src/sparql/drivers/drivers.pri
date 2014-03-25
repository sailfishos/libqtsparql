# if sparql-drivers contains the driver name, we build the driver into QtSparql library.

contains(sparql-drivers, all ) {
    sparql-driver +=   tracker tracker_direct virtuoso endpoint
}

contains(sparql-drivers, tracker) {
    HEADERS +=      drivers/tracker/qsparql_tracker_p.h
    SOURCES +=      drivers/tracker/qsparql_tracker.cpp
    DEFINES += QT_SPARQL_TRACKER

    QT += dbus
}

contains(sparql-drivers, tracker_direct) {
    HEADERS += drivers/tracker_direct/qsparql_tracker_direct_p.h \
               drivers/tracker_direct/qsparql_tracker_direct_driver_p.h \
               drivers/tracker_direct/qsparql_tracker_direct_result_p.h \
               drivers/tracker_direct/qsparql_tracker_direct_select_result_p.h \
               drivers/tracker_direct/qsparql_tracker_direct_sync_result_p.h \
               drivers/tracker_direct/qsparql_tracker_direct_update_result_p.h \
               drivers/tracker_direct/atomic_int_operations_p.h
    SOURCES += drivers/tracker_direct/qsparql_tracker_direct_driver_p.cpp \
               drivers/tracker_direct/qsparql_tracker_direct_result_p.cpp \
               drivers/tracker_direct/qsparql_tracker_direct_select_result_p.cpp \
               drivers/tracker_direct/qsparql_tracker_direct_sync_result_p.cpp \
               drivers/tracker_direct/qsparql_tracker_direct_update_result_p.cpp
    CONFIG += no_keywords link_pkgconfig
    PKGCONFIG += tracker-sparql-1.0
    DEFINES += QT_SPARQL_TRACKER_DIRECT
}

contains(sparql-drivers, virtuoso) {
     HEADERS += drivers/virtuoso/qsparql_virtuoso.h
     SOURCES += drivers/virtuoso/qsparql_virtuoso.cpp

     mac:!contains( LIBS, .*odbc.* ):LIBS        *= -liodbc
     unix:!contains( LIBS, .*odbc.* ):LIBS       *= -lodbc

     win32 {
         !win32-borland:LIBS     *= -lodbc32
         win32-borland:LIBS      *= $(BCB)/lib/PSDK/odbc32.lib
     }
     DEFINES += QT_SPARQL_VIRTUOSO
}


contains(sparql-drivers, endpoint) {
    HEADERS +=      drivers/endpoint/qsparql_endpoint_p.h
    SOURCES +=      drivers/endpoint/qsparql_endpoint.cpp
    DEFINES += QT_SPARQL_ENDPOINT
}
