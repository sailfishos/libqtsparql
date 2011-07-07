#include "qsparqlsparqlconnection_p.h"
#include "qsparqlsparqlconnectionoptions_p.h"
#include "qsparqlsparqlquery_p.h"
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

QVariantList SparqlConnection::exec(SparqlQuery *query)
{
    QSparqlResult *result = syncExec(*query);
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
