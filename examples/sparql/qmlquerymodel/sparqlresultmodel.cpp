#include "sparqlresultmodel.h"

#include <QtCore/QDebug>
#include <QtDeclarative/QDeclarativeExtensionPlugin>
#include <QtDeclarative/qdeclarative.h>

SparqlResultModel::SparqlResultModel(QObject *parent) :
    QAbstractListModel(parent),
    m_connection(0), m_result(0)
{
}

int
SparqlResultModel::rowCount(const QModelIndex &parent) const
{
    //qDebug() << "SparqlResultModel::rowCount() size: " << m_result->size();
    return m_result->size();
}

QVariant
SparqlResultModel::data(const QModelIndex &index, int role) const
{
    // qDebug() << "SparqlResultModel::data" << index << " role " << role << " isValid " << index.isValid();

    if (!index.isValid())
        return QVariant();

    m_result->seek(index.row());
    return m_result->resultRow().value(role - (Qt::UserRole + 1));
}

void
SparqlResultModel::reload()
{
    if (m_hostName.isEmpty() || m_query.isEmpty())
        return;

    /* Create and run the sparql query */
    if (m_result != 0)
        delete m_result;

    QSparqlConnectionOptions options;
    options.setHostName(m_hostName);
    m_connection = new QSparqlConnection("QENDPOINT", options);

    m_result = m_connection->exec(QSparqlQuery(m_query));
    connect(m_result, SIGNAL(finished()), this, SLOT(queryFinished()));

}

void
SparqlResultModel::queryFinished()
{
    qDebug() << m_result->size() << "results found.";

    QHash<int, QByteArray> roleNames;
    roleNames = QAbstractItemModel::roleNames();

    if (m_result->first()) {
        QSparqlResultRow resultRow = m_result->resultRow();

        for (int i = 0; i < resultRow.count(); i++) {
            roleNames.insert((Qt::UserRole + 1) + i, resultRow.binding(i).name().toLatin1());
            qDebug() << "Result name: " << resultRow.variableName(i).toLatin1() << " role " << Qt::UserRole + 1 + i;
        }

        setRoleNames(roleNames);
    }

    reset();
}

QString
SparqlResultModel::hostName() const
{
    return m_hostName;
}

void
SparqlResultModel::setHostName(const QString &hostName)
{
    qDebug() << "Setting host name" << hostName;
    m_hostName = hostName;
    reload();
}

QString
SparqlResultModel::query() const
{
    return m_query;
}

void
SparqlResultModel::setQuery(const QString &query)
{
    qDebug() << "Setting query" << query;
    m_query = query;
    reload();
}

class SparqlPlugin : public QDeclarativeExtensionPlugin
{
public:
    void registerTypes(const char *uri)
    {
        qmlRegisterType<SparqlResultModel>(uri, 0, 1, "SparqlModel");
    }
};

Q_EXPORT_PLUGIN2(sparqlplugin, SparqlPlugin);