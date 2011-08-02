import Qt 4.7
import QtSparql 1.0

Rectangle {
    id: rootComp
    property string setDriver: "QTRACKER_DIRECT"
    property string queryString: sparqlQueryString
    property int modelStatus: 0

    signal modelCountChanged()
    signal modelStatusReady()

    ListView {
        id: contactsView
        width: parent.width
        height: parent.height

        model: SparqlResultsList {
            id: sparqlResultList
            onCountChanged: { modelCountChanged() }
            objectName: "queryModel"
            // load existing query model
            connection: sparqlConnection
            query: sparqlQueryString
            onStatusChanged: { modelStatusChanged() }

        }
       delegate: Item {  height: 50;  }
    }

    SparqlConnection {
        objectName: "sparqlConnection"
        id: sparqlConnection
        driver: setDriver
    }

    function modelStatusChanged()
    {
        modelStatus = sparqlResultList.status;
        if (modelStatus == SparqlResultsList.Ready) {
            modelStatusReady();
        }
    }

    function getCount()
    {
        var resultListCount = sparqlResultList.count;
        return resultListCount
    }

    function getStatus()
    {
        // return the property value here
        // that way we can also check to make sure
        // the notify for Status is working
        return modelStatus
    }

}

