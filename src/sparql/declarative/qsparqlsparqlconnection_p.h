#include <QtCore/qglobal.h>
#include <QtCore/qdebug.h>

#include <QtDeclarative/qdeclarative.h>
#include <QDeclarativeParserStatus>
#include <QtSparql/qsparqlquerymodel.h>


QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Sparql)

class SparqlConnectionOptions;

class Q_SPARQL_EXPORT SparqlConnection : public QSparqlConnection, public QDeclarativeParserStatus
{
    Q_OBJECT
    Q_PROPERTY(QString driver WRITE setDriver READ getdriverName)
    Q_PROPERTY(SparqlConnectionOptions * options WRITE setOptions READ getOptions)
    Q_CLASSINFO("DefaultProperty", "driver")
    Q_INTERFACES(QDeclarativeParserStatus)

public:
    QString driverName;
    SparqlConnectionOptions *options;
    SparqlConnection() { options = 0; }
    ~SparqlConnection() {}

    QString getdriverName() { return driverName; }

    void classBegin() {};
    void componentComplete();

    void setOptions(SparqlConnectionOptions* options)
    {
        this->options = options;
    }

    SparqlConnectionOptions* getOptions()
    {
        return options;
    }

    void setDriver(QString driverName) { this->driverName = driverName; }
};

QT_END_NAMESPACE

QT_END_HEADER
