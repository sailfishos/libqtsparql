#ifndef SPARQLRESULTMODEL_H
#define SPARQLRESULTMODEL_H

#include <QAbstractListModel>

#include <QtSparql/QSparqlQuery>
#include <QtSparql/QSparqlResult>
#include <QtSparql/QSparqlBinding>
#include <QtSparql/QSparqlConnectionOptions>
#include <QtSparql/QSparqlConnection>

class TrackerQuery;

class SparqlResultModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString hostName READ hostName WRITE setHostName)
    Q_PROPERTY(QString query READ query WRITE setQuery)
    Q_CLASSINFO("DefaultProperty", "hostName")
public:
    SparqlResultModel(QObject *parent = 0);

    void reload();

    QVariant data(const QModelIndex &index, int role) const;
    int rowCount(const QModelIndex &parent) const;

    QString hostName() const;
    void setHostName(const QString &hostName);

    QString query() const;
    void setQuery(const QString &query);

private slots:
    void queryFinished();

private:
    QSparqlConnection *m_connection;
    QSparqlResult *m_result;
    QString m_query;
    QString m_hostName;
};

#endif // SPARQLRESULTMODEL_H