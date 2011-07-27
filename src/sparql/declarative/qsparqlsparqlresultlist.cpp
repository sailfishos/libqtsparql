#include "qsparqlsparqlresultlist_p.h"
#include "qsparqlsparqlconnection_p.h"
#include <QtSparql>

SparqlResultList::SparqlResultList() : connection(0)
{
    modelStatus = Null;
    connect(this, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SIGNAL(countChanged()));
    connect(this, SIGNAL(finished()), this, SLOT(onFinished()));
    lastErrorMessage = QLatin1String("");
}

void SparqlResultList::classBegin()
{
}

void SparqlResultList::componentComplete()
{
    modelStatus = Loading;
    Q_EMIT statusChanged(modelStatus);
    // we will create the connection once the component has finished reading, that way
    // we know if any connection options have been set
    if (connection && connection->isValid()) {
        setQuery(QSparqlQuery(queryString), *connection);
    } else {
        lastErrorMessage = QLatin1String("Error opening connection");
        modelStatus = Error;
        Q_EMIT statusChanged(modelStatus);
    }
}

void SparqlResultList::writeQuery(QString query)
{
    queryString = query;
}

QString SparqlResultList::readQuery() const
{
    return queryString;
}

void SparqlResultList::setConnection(SparqlConnection* connection)
{
    if (connection)
    {
        this->connection = connection;
    }
}

SparqlConnection* SparqlResultList::getConnection()
{
    return connection;
}

SparqlResultList::Status SparqlResultList::status()
{
    return modelStatus;
}

QString SparqlResultList::errorString() const
{
    return lastErrorMessage;
}

void SparqlResultList::onFinished()
{
    if (lastError().type() == QSparqlError::NoError) {
        modelStatus = Ready;
    } else {
        lastErrorMessage = lastError().message();
        modelStatus = Error;
    }
    Q_EMIT statusChanged(modelStatus);
}
