import Qt 4.7
import QtSparql 1.0

Rectangle {
    id: rootComp
    property string setDriver: "QTRACKER_DIRECT"
    property string queryString: sparqlQueryString
    property int portNumber: setPortNumber
    property string host: setHost

    SparqlConnection {
        objectName: "connectionWithOptions"
        id: sparqlConnection
        driver: setDriver
        options: SparqlConnectionOptions { id:connectionOptions; hostName: host; port: portNumber }
    }

    function runSelectQuery() {
        return sparqlConnection.exec(queryString);
    }

    function returnConnectionOptions() {
        var hash = {};
        for (var prop in connectionOptions) {
            hash[prop] = connectionOptions[prop];
        }
        return hash;
    }

}

