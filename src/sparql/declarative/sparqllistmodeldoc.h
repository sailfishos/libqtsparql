/*!
    \page qmlSparqlResultList SparqlResultList
    \brief Provides a QML binding for QSparqlQueryModel.

    \code
    import QtSparql 1.0
    \endcode

    \section qmlproperties Properties
    \li \ref qmlPropertyQuery "query : string"
    \li \ref qmlPropertyConnection "connection : SparqlConnection"
    \li \ref qmlPropertyStatus "status : enumeration"
    \li \ref qmlPropertyCount "count : int"

    \section qmlmethods Methods
    \li \ref qmlMethodGet "variant get"
    \li \ref qmlMethodReload "void reload"
    \li \ref qmlMethodErrorString "string errorString"

    \section qmlPropertyDoc Property Documentation

    Required properties are query and connection.

    \anchor qmlPropertyQuery
    <table><tr><th>query : string</th></tr></table>
    Set the QSparqlQuery::SelectStatment to be used to populate the model

    \anchor qmlPropertyConnection
    <table><tr><th>connection : SparqlConnection</th></tr></table>
    Set the \ref qmlSparqlConnection to be used for the model.

    \anchor qmlPropertyStatus
    <table><tr><th>status : enumeration \a read-only</th></tr></table>
    Specifies the model status, which can be one of the following:
        \li Null - Model has not been set.
        \li Ready - Model is ready, and populated with results.
        \li Loading - Model is executing the query.
        \li Error - There was an error with the query or connection.

    The status can be monitored using the statusChanged() signal.

    \anchor qmlPropertyCount
    <table><tr><th>count : int \a read-only</th></tr></table>
    Returns the number of rows in the model. The countChanged() signal can be used
    to notify of any insertions or deletions.

    \section qmlMethodDoc Method Documentation

    \anchor qmlMethodGet
    <table><tr><th>variant get (int i)</th></tr></table>
    Returns the result from row i. See \ref qmlDetatiled "Detailed Description" for usage.

    \anchor qmlMethodReload
    <table><tr><th>void reload ()</th></tr></table>
    Executes the \ref qmlPropertyQuery "query" again. Useful if you wish to periodically
    check for new results.

    \anchor qmlMethodErrorString
    <table><tr><th>string errorString ()</th></tr></table>
    Returns a string description of the last error that occurred if status is SparqlResultList::Error.

    \section qmlDetatiled Detailed Description
    The SparqlResultList binding allows for SPARQL queries to be used as a ListView model.
    The binding requires a \ref qmlSparqlConnection and a \ref qmlPropertyQuery "query" to be set, E.g :

    \code
    ListView {
        id: contactView
        model: SparqlResultList {
                    id:resultList
                    connection: SparqlConnection { driver:"QTRACKER_DIRECT" }
                    query: "select ?u ?firstName ?secondName"+
                           "{ ?u a nco:PersonContact;"+
                           "nco:nameGiven ?firstName;"+
                           "nco:nameFamily ?secondName .}"
                }
        delegate: Item { Text { text: "firstname = "+firstName+", secondname = "+secondName } }
    }
    \endcode

    The delegates property names are extracted from the query, see the section "Using Results" in
    \ref qmlSparqlConnection for more information on this.

    If you intend to use the \ref qmlSparqlConnection for other queries, it may be more convenient to declare
    it as a property, E.g :

    \code
    property SparqlConnection sparqlConnection : SparqlConnection {
                                                    driver: "QTRACKER_DIRECT"
                                                 }

    ListView {
        model: SparqlResultList {
                connection: sparqlConnection
                ...
               }
    }
    \endcode

    The \ref qmlPropertyCount "count" property can be used to return the number of rows in the model,
    and to be notified of insertions/deletions by connecting to the countChanged() signal, E.g :

    \code
    SparqlResultList {
        onCountChanged : someFunction()
        ...
    }
    \endcode

    To return result information for a specific row in the model, the \ref qmlMethodGet "get" function can
    be used, E.g :

    \code
    SparqlResultList {
        id:resultList
        ...
    }

    function getRow(row)
    {
        var result = getRow(row)
        for (var property in result) {
            console.log("Property name = "+property)
            console.log("Property value = "+result[property])
        }
    }
    \endcode
*/
