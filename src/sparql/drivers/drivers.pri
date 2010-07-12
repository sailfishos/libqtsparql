# if sparql-drivers contains the driver name, we build the driver into QtSparql library.

contains(sparql-drivers, all ) {
    sparql-driver +=   tracker virtuoso endpoint
}

contains(sparql-drivers, tracker) {
    HEADERS +=      drivers/tracker/qsparql_tracker.h \
    	    	    drivers/tracker/qsparql_tracker_p.h
    SOURCES +=      drivers/tracker/qsparql_tracker.cpp

    QT += dbus
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
}


contains(sparql-drivers, endpoint) {
    HEADERS +=      drivers/endpoint/qsparql_endpoint.h
    SOURCES +=      drivers/endpoint/qsparql_endpoint.cpp
}
