/*!
    \page qmlSparqlConnection SparqlConnection
    \brief Provides a QML binding for QSparqlConnection. The connection can be used to issue SPARQL queries
    directly from javascript, or as a connection for the ListView model \ref qmlSparqlResultList

    \code
    import QtSparql 1.0
    \endcode

    \section qmlproperties Properties
    \li \ref qmlPropertyDriver "driver : string"
    \li \ref qmlPropertyStatus "status : enumeration"
    \li \ref qmlPropertyOptions "options : SparqlConnectionOptions"
    \li \ref qmlPropertyResult "result : variant"

    \section qmlmethods Methods
    \li \ref qmlMethodSelect "variant select"
    \li \ref qmlMethodUpdate "variant update"
    \li \ref qmlMethodConstruct "variant construct"
    \li \ref qmlMethodErrorString "string errorString"

    \section qmlPropertyDoc Property Documentation

    The only required property to set is \a driver.

    \anchor qmlPropertyDriver
    <table><tr><th>driver : string</th></tr></table>
    Set the driver to use for the connection, E.g. to use the tracker direct driver :
    \code
    SparqlConnection {
        id: connection
        driver: "QTRACKER_DIRECT"
    }
    \endcode
    \anchor qmlPropertyStatus
    <table><tr><th>status : enumeration \a read-only</th></tr></table>
    Specifies the connection status, which can be one of the following:
        \li Null - Connection has not been established
        \li Ready - Connection is ready
        \li Loading - Connection opening is established and waiting for completion
        \li Error - There was an error in the connection opening, or an error with a query

    The status can be monitored using the statusChanged() signal. E.g :
    \code
    SparqlConnection {
        id: connectionalternatively
        driver: "QTRACKER_DIRECT"
        onStatusChanged: checkStatus(status)
    }

    function checkStatus(status)
    {
        if (status == SparqlConnection.Error)
            console.log("Error = "+connection.errorString());
    }
    \endcode

    \anchor qmlPropertyOptions
    <table><tr><th>options : SparqlConnectionOptions</th></tr></table>

    If connection options need to be set, the SparqlConnectionOptions binding can be used, E.g :

    \code
    SparqlConnection {
        driver: "QSPARQL_ENDPOINT"
        options: SparqlConnectionOptions {
                    hostName: "localhost"
                    port: 1234
                 }
    \endcode

    For a full list of options, see QSparqlConnectionOptions.

    \anchor qmlPropertyResult
    <table><tr><th>result : variant \a read-only </th></tr></table>

    Stores the current result after executing a query with \ref qmlMethodSelect "select()". This is a convenience
    property to allow for a common function to be used for both synchronous and asynchronous queries. When a new result
    is ready, a resultReady() signal will be emitted. E.g :
    \code
    SparqlConnection {
        id: connection
        driver: "QTRACKER_DIRECT"
        onResultReady: resultFunction(result)
    }

    function resultFunction(result)
    {
        \\ do something with result
    }
    \endcode

    See the section \ref usingResults "Using Results" for more information.

    \section qmlMethodDoc Method Documentation

    \anchor qmlMethodSelect
    <table><tr><th>variant select (string query, bool async = false)</th></tr></table>
    Runs a QSparqlQuery::SelectStatement or QSparqlQuery::AskStatment, synchronous by default.

    If the query is a SelectStatement the function will return a list of results, or a boolean value
    for AskStatements.

    See the section \ref usingResults "Using Results" for result usage.

    If the async parameter is set to true, the query will be executed asynchronously, and the function
    will return 0. For asynchronous usage, see the section \ref asyncQueries "Asynchronous Queries".

    \anchor qmlMethodUpdate
    <table><tr><th>variant update (string query, bool async = false)</th></tr></table>
    Runs a QSparqlQuery::InsertStatement or QSparqlQuery::DeleteStatement. If there was an error with the
    update query, -1 will be returned, otherwise an empty list will be returned.

    \anchor qmlMethodConstruct
    <table><tr><th>variant construct (string query, bool async = false)</th></tr></table>
    Runs a QSparqlQuery::ConstructQuery returning the results, or -1 if there was an error.

    \anchor qmlMethodErrorString
    <table><tr><th>string errorString ()</th></tr></table>
    Returns a string description of the last error that occurred if status is SparqlConnection::Error

    \section qmlDetatiled Detailed Description
    \subsection basicUsage Basic Usage
    The SparqlConnection binding can be used to issue SPARQL queries, and as a connection for the ListView model SparqlResultList.
    To create a new connection :
    \code
    SparqlConnection {
        id: connection
        driver: "QTRACKER_DIRECT"
    }
    \endcode

    The connection can then be used in javascript functions using the id "connection".

    \subsection usingResults Using Results
    Property names for results will be extracted from the query string that is used. For example, results for the query :
    \code
    select ?u ?ng ?nf { ?u a nco:PersonContact; nco:nameGiven ?ng; nco:nameFamily ?nf }
    \endcode
    Will have property names "u", "ng" and "nf". It is important to note that when using a function in the query, the property
    name must be defined using the AS keyword. E.g :
    \code
    select ?u fn:string( (?nf, ?ng), ' ') AS ?joinedName { ... }
    \endcode
    Will return "u" and "joinedNamed" as the property names.

    When using \ref qmlMethodSelect "select()" results will be returned as a list of "result rows" :
    \code
    var result = connection.select(query)
    \endcode
    It is easy to iterate through the results using its length property :
    \code
    for (var i=0; i<result.length; i++) {
        var resultRow = result[i];
        var nameGiven = resultRow["ng"]
        // Alternatively the property name could be used
        var nameGivenProperty = resultRow.ng;
    }
    \endcode

    Alternatively, you can access result properties by looping through the property values, E.g :
    \code
    for (var i=0; i<result.length; i++) {
        var resultRow = result[i];
        for (var property in resultRow) {
            console.log("property name = "+property)
            console.log("property value = "+resultRow[property])
        }
    }
    \endcode

    \subsection asyncQueries Asynchronous Queries

    Queries may also be executed asynchronously, and processed once they have been completed.
    This is convenient if you intend to issue large queries with the connection, as any call to
    \ref qmlMethodSelect "select()" or \ref qmlMethodUpdate "update()" will not block. This is done by setting the
    async paramater to true for both \ref qmlMethodSelect "select()" and \ref qmlMethodUpdate "update()".

    To issue a select query asynchronoulsy :
    \code
    connection.select(query, true);
    \endcode

    The result can be processed after the connections resultReady() signal has been emitted, E.g :
    \code
    SparqlConnection {
        ...
        onResultReady: resultFunction(result)
    }

    function resultFunction(result)
    {
        // check for an error
        if (connection.status != SparqlConnection.Error) {
            //process the results
        }
    }
    \endcode

    It is also possible to connect to the resultReady() signal directly in javascript, E.g :

    \code
    function asyncSelect() {
        sparqlConnection.resultReady.connect(someFunction)
        sparqlConnection.select(query, true)
    }
    \endcode

    Please note: Synchronous calls to select() will also emit the resultReady signal, so if you intend to use
    both asynchronous and synchronous queries you can process both using the onResultReady: property.
*/

