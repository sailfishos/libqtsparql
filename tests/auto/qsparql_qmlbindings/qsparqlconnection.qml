import Qt 4.7
import QtSparql 0.1

Rectangle {
    id: rootComp
    property string setDriver: "QTRACKER_DIRECT"
    property string queryString: sparqlQueryString

    SparqlConnection {
        objectName: "connectionWithOptions"
        id: sparqlConnection
        driver: setDriver
        options: SparqlConnectionOptions { hostName: "localhost" }
    }

    function runSelectQuery() {
        return sparqlConnection.exec(queryString);
    }

}

