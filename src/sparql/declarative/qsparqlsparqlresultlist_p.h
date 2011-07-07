#include <QtCore/qglobal.h>
#include <QtCore/qdebug.h>

#include <QtDeclarative/qdeclarative.h>
#include <QDeclarativeParserStatus>
#include <QtSparql/qsparqlquerymodel.h>


QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Sparql)

class SparqlConnection;

class Q_SPARQL_EXPORT SparqlResultList : public QSparqlQueryModel, public QDeclarativeParserStatus
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QSparqlQueryModel)
    Q_ENUMS(Status)
    Q_PROPERTY(QString query READ readQuery WRITE writeQuery)
    Q_PROPERTY(SparqlConnection* connection READ getConnection WRITE setConnection)
//    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
//    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
//    Q_CLASSINFO("DefaultProperty", "query")
    Q_INTERFACES(QDeclarativeParserStatus)

public:  
    SparqlConnection *connection;
    QString queryString;

    SparqlResultList() { connection=0; }
    void classBegin() {};
    void componentComplete();

    QString readQuery() const { return queryString; };

    void setConnection(SparqlConnection* connection)
    {
        if (connection)
        {
            this->connection = connection;
        }
    }

    SparqlConnection* getConnection()
    {
        return connection;
    }

    void writeQuery(QString query) { queryString = query; }
};

QT_END_NAMESPACE

QT_END_HEADER
