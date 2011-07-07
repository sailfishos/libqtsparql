#include <QtCore/qglobal.h>
#include <QtCore/qdebug.h>

#include <QtDeclarative/qdeclarative.h>
#include <QDeclarativeParserStatus>
#include <QtSparql/qsparqlquerymodel.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Sparql)

class Q_SPARQL_EXPORT SparqlQuery : public QObject, public QSparqlQuery
{
    Q_OBJECT
    Q_PROPERTY(QString query READ query WRITE setQuery)
    Q_CLASSINFO("DefaultProperty", "query")
};

QT_END_NAMESPACE

QT_END_HEADER
