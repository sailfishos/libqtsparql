import Qt 4.7
import QtSparql 1.0

Rectangle {
    id: rootComp
    property string setDriver: "QTRACKER_DIRECT"
    property string queryString: sparqlQueryString
    property int portNumber: setPortNumber
    property string host: setHost
    property int connectionStatus: 0
    property int resultReadyCalled: 0
    property variant resultFromSlot: 0

    signal resultReadySignal();

    SparqlConnection {
        objectName: "connectionWithOptions"
        id: sparqlConnection
        driver: setDriver
        options: SparqlConnectionOptions { id:connectionOptions; hostName: host; port: portNumber }
        onStatusChanged: connectionStatus = sparqlConnection.status
        onResultReady: resultReadySignal()
    }

    function runSelectQuery()
    {
       return sparqlConnection.select(queryString);
    }

    function runConstructQuery()
    {
        // will result in error, since construct queries are not
        // supported by tracker
        return sparqlConnection.construct(queryString);
    }

    function runSelectQueryAsync()
    {
        sparqlConnection.resultReady.connect(asyncResultReady);
        sparqlConnection.select(queryString, true);
    }

    function asyncResultReady(result)
    {
        resultFromSlot = result;
        resultReadySignal();
    }

    // returns from the result property
    function returnResults()
    {
        return sparqlConnection.result;
    }

    function returnResultsFromSlot()
    {
        return resultFromSlot;
    }

    function returnConnectionOptions() {
        var hash = {};
        for (var prop in connectionOptions) {
            hash[prop] = connectionOptions[prop];
        }
        return hash;
    }

    function insertContact() {
        var insertQuery = "insert { <qmlInsert> a nco:PersonContact; "+
                          "nie:isLogicalPartOf <qsparql-qml-tests>; "+
                          "nco:nameGiven 'QML INSERT' }";
        sparqlConnection.update(insertQuery);
    }

    function insertContactAsync() {
        var insertQuery = "insert { <qmlInsert> a nco:PersonContact; "+
                          "nie:isLogicalPartOf <qsparql-qml-tests>; "+
                          "nco:nameGiven 'QML INSERT' }";
        sparqlConnection.update(insertQuery, true);
    }

    function deleteContact() {
        var deleteQuery = "delete { <qmlInsert> a nco:PersonContact } "+
                          "where { <qmlInsert> a nco:PersonContact } ";

        sparqlConnection.update(deleteQuery);
    }

    function deleteContactAsync() {
        var deleteQuery = "delete { <qmlInsert> a nco:PersonContact } "+
                          "where { <qmlInsert> a nco:PersonContact } ";

        sparqlConnection.update(deleteQuery, true);
    }

    function getStatus()
    {
        // return the property value here
        // that way we can also check to make sure
        // the notify for Status is working
        return connectionStatus
    }

    function getLastError()
    {
        return sparqlConnection.errorString();
    }
}

