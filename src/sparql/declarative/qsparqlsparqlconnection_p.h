#include <QtCore/qglobal.h>
#include <QtCore/qdebug.h>

#include <QtDeclarative/qdeclarative.h>
#include <QDeclarativeParserStatus>
#include <QtSparql/qsparqlquerymodel.h>


QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Sparql)

class SparqlQuery;
class SparqlConnectionOptions;

class Q_SPARQL_EXPORT SparqlConnection : public QSparqlConnection, public QDeclarativeParserStatus
{
    Q_OBJECT
    Q_ENUMS(Status)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString driver WRITE setDriver READ getdriverName)
    Q_PROPERTY(SparqlConnectionOptions * options WRITE setOptions READ getOptions)
    Q_CLASSINFO("DefaultProperty", "driver")
    Q_INTERFACES(QDeclarativeParserStatus)

public:
    QString driverName;
    QString lastErrorMessage;
    SparqlConnectionOptions *options;
    SparqlConnection();
    ~SparqlConnection() {}

    QString getdriverName() { return driverName; }

    void classBegin();
    void componentComplete();

    Q_INVOKABLE QVariant exec(QString query);
    Q_INVOKABLE QString errorString() const;

    void setOptions(SparqlConnectionOptions* options)
    {
        if (options)
        {
            this->options = options;
        }
    }

    SparqlConnectionOptions* getOptions()
    {
        return options;
    }

    void setDriver(QString driverName) { this->driverName = driverName; }

    enum Status { Null, Ready, Loading, Error };
    Status connectionStatus;
    Status status();

Q_SIGNALS:
    void statusChanged(SparqlConnection::Status);
};

QT_END_NAMESPACE

QT_END_HEADER
