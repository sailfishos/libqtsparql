import Qt 4.7
import QtSparql 0.1

Rectangle {
    id: rootComp
    property string setDriver: "QTRACKER_DIRECT"
    property string queryString: sparqlQueryString
    property SparqlQuery query : SparqlQuery { query: queryString }
    SparqlConnection {
        objectName: "connectionWithOptions"
        id: sparqlConnection
        driver: setDriver
        options: SparqlConnectionOptions { hostName: "localhost" }
    }

    function runQuery() {
        return sparqlConnection.exec(query);
    }

}

