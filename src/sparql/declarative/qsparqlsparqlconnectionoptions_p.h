#include <QtCore/qglobal.h>
#include <QtCore/qdebug.h>

#include <QtDeclarative/qdeclarative.h>
#include <QDeclarativeParserStatus>
#include <QtSparql/qsparqlquerymodel.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Sparql)

class Q_SPARQL_EXPORT SparqlConnectionOptions : public QObject, public QSparqlConnectionOptions
{
    Q_OBJECT
    Q_PROPERTY(QString databaseName READ databaseName WRITE setDatabaseName)
    Q_PROPERTY(QString userName READ userName WRITE setUserName)
    Q_PROPERTY(QString password READ password WRITE setPassword)
    Q_PROPERTY(QString hostName READ hostName WRITE setHostName)
    Q_PROPERTY(QString path READ path WRITE setPath)
    Q_PROPERTY(int port READ port WRITE setPort)


};

QT_END_NAMESPACE

QT_END_HEADER
