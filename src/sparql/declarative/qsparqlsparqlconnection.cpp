#include "qsparqlsparqlconnection_p.h"
#include "qsparqlsparqlconnectionoptions_p.h"
#include <QSparqlResult>
#include <QSparqlResultRow>
#include <QSparqlError>

SparqlConnection::SparqlConnection()
{
    connectionStatus = Null;
    options = 0;
    lastErrorMessage = QLatin1String("");
}

void SparqlConnection::classBegin()
{
    connectionStatus = Loading;
    Q_EMIT statusChanged(connectionStatus);
}

void SparqlConnection::componentComplete()
{
    // we will create the connection once the component has finished reading, that way
    // we know if any connection options have been set
    if (options) {
        qmlConstructor(driverName, *options);
    } else {
        qmlConstructor(driverName);
    }

    // check connection opening when ok
    if (isValid()) {
        connectionStatus = Ready;
    } else {
        lastErrorMessage = QLatin1String("Failed to open connection");
        connectionStatus = Error;
    }
    Q_EMIT statusChanged(connectionStatus);
}

QString SparqlConnection::errorString() const
{
    if (connectionStatus != Error)
        return QString();
    return lastErrorMessage;
}

QVariant SparqlConnection::exec(QString queryString)
{
    // don't bother doing anything if the connection isn't valid
    if (!isValid()) {
        return -1;
    }

    // the last query may have resulted in an error, set set the status to
    // ready
    connectionStatus = Ready;
    Q_EMIT statusChanged(connectionStatus);

    QSparqlQuery query;
    query.setQuery(queryString);
    if ( queryString.contains(QLatin1String("insert"), Qt::CaseInsensitive) )
        query.setType(QSparqlQuery::InsertStatement);
    if ( queryString.contains(QLatin1String("delete"), Qt::CaseInsensitive) )
        query.setType(QSparqlQuery::DeleteStatement);
    if ( queryString.contains(QLatin1String("construct"), Qt::CaseInsensitive) )
        query.setType(QSparqlQuery::ConstructStatement);

    QSparqlResult *result = syncExec(query);
    QVariantList resultList;

    // check for a result error
    if (result->hasError()) {
        connectionStatus = Error;
        lastErrorMessage = result->lastError().message();
        Q_EMIT statusChanged(connectionStatus);
        delete result;
        return -1;
    }

    while(result->next()) {
        QSparqlResultRow row = result->current();
        QVariantMap resultHash;
        for (int i=0; i<row.count(); i++) {
            resultHash.insert(row.binding(i).name(), row.value(i));
        }
        resultList.append(resultHash);
    }
    delete result;
    return resultList;
}

SparqlConnection::Status SparqlConnection::status()
{
    return connectionStatus;
}
