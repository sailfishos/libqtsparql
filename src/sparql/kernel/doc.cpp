/*!
    \mainpage The QtSparql library

    \brief QtSparql is a client-side library for accessing RDF stores.

    The query language for RDF stores is <a
    href="http://www.w3.org/TR/rdf-sparql-query/">SPARQL</a>.

    QtSparql takes in SPARQL queries, forwards them to the selected backend, and
    gives back the results of the query.  It can return the results
    asynchronously if the backend supports asynchronous operations.

    QtSparql can connect to different backends.  Currently the following backends
    exist:

    - tracker backend for accessing <a href="http://projects.gnome.org/tracker/">Tracker</a>
    - endpoint backend of accessing online RDF stores, e.g., <a href="http://dbpedia.org">DBpedia</a>
    - virtuoso backend for accessing <a href="http://docs.openlinksw.com/virtuoso/">Virtuoso</a>

    \section gettingstarted Getting started

    \attention The QtSparql library is not yet stable; we make no
    promises about API / ABI compatibility!

    The following code snippets demonstrate how to retrieve data from
    a RDF database using QtSparql.

    - Create a QSparqlConnection object specifiying the backend you want to use.
      If necessary, specify the parameters by using QSparqlConnectionOptions and
      passing it to QSparqlConnection.

    E.g., to use tracker:
    \dontinclude simple/main.cpp
    \skipline QSparqlConnection

    E.g., to use DBpedia:
    \dontinclude dbpedia/main.cpp
    \skip QSparqlConnectionOptions
    \until QSPARQL_ENDPOINT

    - Construct a QSparqlQuery with the SPARQL query string.  Specify the query
      type, if needed.

    E.g.,
    \dontinclude simple/main.cpp
    \skipline QSparqlQuery

    or

    \dontinclude iteration/main.cpp
    \skip QSparqlQuery insert
    \until InsertStatement

    - Use QSparqlConnection::exec() to execute the query. It returns a
      pointer to QSparqlResult.

    E.g.,
    \dontinclude simple/main.cpp
    \skipline QSparqlResult

    - You can use QSparqlResult::waitForFinished() to wait until the query has
      finished, or connect to the QSparqlResult::finished() signal.

    - The QSparqlResult can be iterated over by using the following functions:
      QSparqlResult::first(), QSparqlResult::last(), QSparqlResult::next(),
      QSparqlResult::previous(), QSparqlResult::seek(). The caller is
      responsible for deleting the QSparqlResult.

    E.g.,
    \dontinclude simple/main.cpp
    \skip result->next
    \until toString

    - Data can be retrieved by using QSparqlResult::value().

    The following classes are the most relevant for getting started with QSparql:
    - QSparqlConnection
    - QSparqlQuery
    - QSparqlResult
    - QSparqlQueryModel

    \section querymodels Query models

    TODO: QSparlQueryModel

    \section backendspecific Accessing backend-specific functionalities

    \warning This is under discussion; changes are likely

    An example: using the signalling functionality implemented only by the
    tracker driver ( http://live.gnome.org/Tracker/Documentation/SignalsOnChanges ).

    The tracker driver is the only driver which offers notifications when data
    in the store changes. To connect to the change signal, you need to do the
    following:

    - link against the tracker driver
      ($QT_PLUGIN_PATH/sparqldrivers/libqsparqltracker.so), in addition to
      libqtsparql
    - include a driver specific header file, qsparql_tracker_signals.h
    - instantiate the QTrackerChangeNotifier object and connect to its signals

*/
