SOURCES = tracker-sparql.cpp
CONFIG -= qt

unix: {
    CONFIG += link_pkgconfig
    PKGCONFIG += tracker-sparql-1.0
}
