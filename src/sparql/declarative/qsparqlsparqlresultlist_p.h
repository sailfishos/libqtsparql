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
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_CLASSINFO("DefaultProperty", "query")
    Q_INTERFACES(QDeclarativeParserStatus)

public:
    SparqlConnection *connection;
    QString queryString;
    QString lastErrorMessage;

    SparqlResultList();
    void classBegin();
    void componentComplete();

    Q_INVOKABLE QString errorString() const;
    Q_INVOKABLE QVariant get(int rowNumber);
    Q_INVOKABLE void reload();
    void setConnection(SparqlConnection* connection);
    SparqlConnection* getConnection();
    void writeQuery(QString query);
    QString readQuery() const;

    enum Status { Null, Ready, Loading, Error };
    Status modelStatus;
    Status status();
public Q_SLOTS:
    void onFinished();
    void onStarted();
    void onConnectionComplete();
Q_SIGNALS:
    void countChanged();
    void statusChanged(SparqlResultList::Status);
};

QT_END_NAMESPACE

QT_END_HEADER
