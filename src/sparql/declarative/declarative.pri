SOURCES += declarative/qsparqlsparqlconnection.cpp declarative/qsparqlsparqllistmodel.cpp
HEADERS += declarative/qsparqlsparqlconnection_p.h declarative/qsparqlsparqlconnectionoptions_p.h declarative/qsparqlsparqllistmodel_p.h
equals(QT_MAJOR_VERSION, 4) {
    QT += declarative
}
equals(QT_MAJOR_VERSION, 5) {
    QT += qml
    DEFINES *= QT_VERSION_5
}
