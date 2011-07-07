#include "qsparqlsparqlresultlist_p.h"
#include "qsparqlsparqlconnection_p.h"
#include <QtSparql>

void SparqlResultList::componentComplete()
{
    // we will create the connection once the component has finished reading, that way
    // we know if any connection options have been set
    if (connection) {
        setQuery(QSparqlQuery(queryString), *connection);
    }
   
}
