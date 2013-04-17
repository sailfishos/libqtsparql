SOURCES += declarative/qsparqlsparqlconnection.cpp declarative/qsparqlsparqllistmodel.cpp
equals(QT_MAJOR_VERSION, 4) {
    HEADERS += declarative/qsparqlsparqlconnection_p.h declarative/qsparqlsparqlconnectionoptions_p.h declarative/qsparqlsparqllistmodel_p.h
    QT += declarative
}
equals(QT_MAJOR_VERSION, 5) {
    HEADERS += declarative/qsparqlsparqlconnection-qt5_p.h declarative/qsparqlsparqlconnectionoptions-qt5_p.h declarative/qsparqlsparqllistmodel-qt5_p.h
    QT += qml
}
