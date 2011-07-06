#include <QtCore/qglobal.h>
#include <QtCore/qdebug.h>

#include <QtDeclarative/qdeclarative.h>
#include <QDeclarativeParserStatus>
#include <QtSparql/qsparqlquerymodel.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Sparql)

class Q_SPARQL_EXPORT SparqlConnection : public QSparqlConnection, public QDeclarativeParserStatus
{
    Q_OBJECT
    Q_PROPERTY(QString driver WRITE setDriver READ getdriverName)
    Q_CLASSINFO("DefaultProperty", "driver")
    Q_INTERFACES(QDeclarativeParserStatus)

public:
    QString driverName;
    QSparqlConnectionOptions *options;
    SparqlConnection() { options = 0;}
    ~SparqlConnection() {}

    QString getdriverName() { return driverName; }

    void classBegin() {};
    void componentComplete()
    {
        // we will create the connection once the component has finished reading, that way
        // we know if any connection options have been set
    }
    // QSparqlConnectionOptions options
    // QSparqlConnectionOptions* getConnectionOptions() { return options; }

    void setDriver(QString driverName) { this->driverName = driverName; }

    //void setConnectionOptions(QSparqlConnectionOptions* options) { this->options = options; }
};

QT_END_NAMESPACE

QT_END_HEADER
