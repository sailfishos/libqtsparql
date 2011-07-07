#include "qsparqlsparqlconnection_p.h"
#include "qsparqlsparqlconnectionoptions_p.h"
#include <QSparqlResult>
#include <QSparqlResultRow>

void SparqlConnection::componentComplete()
{
    // we will create the connection once the component has finished reading, that way
    // we know if any connection options have been set
    if (options) {
        qmlConstructor(driverName, *options);
    }
    else
        qmlConstructor(driverName);
}

QVariantList SparqlConnection::exec(QString queryString)
{
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
