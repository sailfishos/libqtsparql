import Qt 4.7
import QtSparql 1.0

Rectangle {
    id: rootComp
    property string setDriver: "QTRACKER_DIRECT"
    property string queryString: sparqlQueryString
    property int portNumber: setPortNumber
    property string host: setHost
    property int connectionStatus: 0

    SparqlConnection {
        objectName: "connectionWithOptions"
        id: sparqlConnection
        driver: setDriver
        options: SparqlConnectionOptions { id:connectionOptions; hostName: host; port: portNumber }
        onStatusChanged: connectionStatus = sparqlConnection.status
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

    function getStatus()
    {
        // return the property value here
        // that way we can also check to make sure
        // the notify for Status is working
        return connectionStatus
    }
}

