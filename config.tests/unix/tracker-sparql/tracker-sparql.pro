SOURCES = tracker-sparql.cpp
CONFIG -= qt

unix: {
    CONFIG += link_pkgconfig
    PKGCONFIG += tracker-sparql-2.0
}
