#include "sparqlquerytext.h"

#include <QtSparql/QSparqlResultRow>
#include <QtSparql/QSparqlError>

SparqlQueryText::SparqlQueryText(QSparqlConnection& conn, QWidget *parent)
    : QTextEdit(parent), connection(conn)
{
    setText(QLatin1String("SELECT ?Predicate ?Object \nWHERE { <http://dbpedia.org/resource/The_Beatles> ?Predicate ?Object . }"));
    model = new QSparqlQueryModel();
    model->setHeaderData(0, Qt::Horizontal, tr("Predicate"));
    model->setHeaderData(1, Qt::Horizontal, tr("Object"));

    tableView = new QTableView();
    tableView->setModel(model);
}

void SparqlQueryText::runQuery()
{
    qDebug() << "In runQuery";
    query = new QSparqlQuery(toPlainText());
    result = connection.exec(*query);
    connect(result, SIGNAL(finished()), this, SLOT(showResults()));
    model->setQuery(*query, connection);
    tableView->show();
}

void printPosition(const QSparqlResult* q)
{
    int i = q->pos();
    switch (i) {
    case QSparql::BeforeFirstRow:
        qDebug() << "Position: Before first";
        break;
    case QSparql::AfterLastRow:
        qDebug() << "Position: After last";
        break;
    default:
        qDebug() << "Position:" << i;
        break;
    }
}

void printResultRow(const QSparqlResultRow& rr)
{
    qDebug() << "Result row:";
    if (rr.isEmpty()) {
        qDebug() << "Empty";
        return;
    }
    qDebug() << "Column count:" << rr.count();
    for (int i = 0; i < rr.count(); ++i)
        qDebug() << "\t" << i << rr.value(i).toString();
}

void SparqlQueryText::showResults()
{
    qDebug() << "In showResults";
    if (result->lastError().isValid()) {
        qDebug() << "Query failed: " << result->lastError();
        return;
    }
    
    qDebug() << "---- Iterating forward ----";
    // First the query is positioned "before the first row"
    printPosition(result);
    printResultRow(result->resultRow());
    while (result->next()) {
        printPosition(result);
        printResultRow(result->resultRow());
    }
    // Then the query is positioned "after the last row"
    printPosition(result);
    printResultRow(result->resultRow());
}


