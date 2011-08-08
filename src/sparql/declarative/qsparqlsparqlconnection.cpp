#include "qsparqlsparqlconnection_p.h"
#include "qsparqlsparqlconnectionoptions_p.h"
#include <QSparqlResult>
#include <QSparqlResultRow>
#include <QSparqlError>

SparqlConnection::SparqlConnection()
{
    connectionStatus = Null;
    options = 0;
    lastResult = 0;
    asyncResult = 0;
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
    Q_EMIT onCompleted();
}

QString SparqlConnection::errorString() const
{
    if (connectionStatus != Error)
        return QString();
    return lastErrorMessage;
}

QVariant SparqlConnection::select(QString queryString, bool async)
{
    QSparqlQuery query(queryString);
    return runQuery(query, async);
}

QVariant SparqlConnection::update(QString queryString, bool async)
{
    // inserts and deletes are both update queries, and run in the same
    // way, so it doesn't matter if this is an insert or delete statement
    QSparqlQuery query(queryString, QSparqlQuery::InsertStatement);
    return runQuery(query, async);
}

QVariant SparqlConnection::construct(QString queryString, bool async)
{
    QSparqlQuery query(queryString, QSparqlQuery::ConstructStatement);
    return runQuery(query, async);
}

QVariant SparqlConnection::runQuery(QSparqlQuery query, bool async)
{
    if (!isValid()) {
        return -1;
    }
    connectionStatus = Ready;
    Q_EMIT statusChanged(connectionStatus);

    if (async) {
        asyncResult = exec(query);
        connect(asyncResult, SIGNAL(finished()), this, SLOT(onResultFinished()));
        lastResult = 0;
    } else {
        QSparqlResult *result = syncExec(query);
        return resultToVariant(result);
    }
    return 0;
}

QVariant SparqlConnection::getResult()
{
    return lastResult;
}

void SparqlConnection::onResultFinished()
{
    resultToVariant(asyncResult);
}

QVariant SparqlConnection::resultToVariant(QSparqlResult *result)
{
    QVariantList resultList;
    // check for a result error
    if (result->hasError()) {
        connectionStatus = Error;
        lastErrorMessage = result->lastError().message();
        Q_EMIT statusChanged(connectionStatus);
        delete result;
        return -1;
    }

    if (result->isBool()) {
        resultList.append(result->boolValue());
    } else {
        while(result->next()) {
            QSparqlResultRow row = result->current();
            QVariantMap resultHash;
            for (int i=0; i<row.count(); i++) {
                resultHash.insert(row.binding(i).name(), row.value(i));
            }
            resultList.append(resultHash);
        }
    }
    result->deleteLater();
    lastResult.clear();
    lastResult = resultList;
    Q_EMIT resultReady(lastResult);
    return lastResult;
}

SparqlConnection::Status SparqlConnection::status()
{
    return connectionStatus;
}
