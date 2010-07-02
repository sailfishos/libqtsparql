#include "sparqlquerytext.h"

#include <QtSparql/QSparqlBindingSet>
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

void printBindingSet(const QSparqlBindingSet& bs)
{
    qDebug() << "Binding set:";
    if (bs.isEmpty()) {
        qDebug() << "Empty";
        return;
    }
    qDebug() << "Column count:" << bs.count();
    for (int i = 0; i < bs.count(); ++i)
        qDebug() << "\t" << i << bs.value(i).toString();
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
    printBindingSet(result->bindingSet());
    while (result->next()) {
        printPosition(result);
        printBindingSet(result->bindingSet());
    }
    // Then the query is positioned "after the last row"
    printPosition(result);
    printBindingSet(result->bindingSet());
}


