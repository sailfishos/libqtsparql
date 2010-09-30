/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the documentation of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGui>
#include <QtSparql/QtSparql>

#include <iostream>

using namespace std;

QString tr(const char *text)
{
    return QApplication::translate(text, text);
}

void QSparqlConnection_snippets()
{
    {
//! [0]
    QSparqlConnectionOptions options;
    options.setHostName("acidalia");
    options.setDatabaseName("customdb");
    options.setUserName("mojito");
    options.setPassword("J0a1m8");
    QSparqlConnection connection("QVIRTUOSO", options);
    bool ok = connection.isValid();
//! [0]
    Q_UNUSED(ok);
    }

    {
//! [1]
    QSparqlConnection conn("QTRACKER");
//! [1]
    }
}

void QSparqlBinding_snippets()
{
    QSparqlConnection connection("QSPARQL_ENDPOINT");
#if 0
    {
//! [2]
    QSparqlBinding binding("age", QVariant::Int);
    binding.setValue(QPixmap());  // WRONG
//! [2]
    }
#endif

    {
//! [3]
    QSparqlBinding binding("age", QVariant::Int);
    binding.setValue(QString("123"));  // casts QString to int
//! [3]
    }

    {
//! [4]
    QSparqlQuery query;
    QSparqlResult * result = connection.exec(query);
//! [4] //! [5]
    QSparqlResultRow resultRow = result->current();
//! [5] //! [6]
    QSparqlBinding binding = resultRow.binding("country");
//! [6]
    }
}

void doSomething(const QString &)
{
}

void QSparqlQuery_snippets()
{
    QSparqlConnection connection("QSPARQL_ENDPOINT");
    {
    // typical loop
//! [7]
    QSparqlQuery query("SELECT ?country WHERE { ?country rdf:type dbpedia-owl:Country . }");
    QSparqlResult *result = connection.exec(query);
    while (result->next()) {
        QString country = result->value(0).toString();
        doSomething(country);
    }
//! [7]
    }

    {
    // binding index lookup
//! [8]
    QSparqlQuery query("SELECT ?country WHERE { ?country rdf:type dbpedia-owl:Country . }");
    QSparqlResult * result = connection.exec(query);
    int bindingNo = result->current().indexOf("country");
    while (result->next()) {
        QString country = result->value(bindingNo).toString();
        doSomething(country);
    }
//! [8]
    }

    QSparqlQuery query;

    {
    // examine with named binding
//! [14]
    QMapIterator<QString, QVariant> i(query.boundValues());
    while (i.hasNext()) {
        i.next();
        cout << i.key().toAscii().data() << ": "
             << i.value().toString().toAscii().data() << endl;
    }
//! [14]
    }

    {
    // examine with positional binding
//! [15]
    QList<QVariant> list = query.boundValues().values();
    for (int i = 0; i < list.size(); ++i)
        cout << i << ": " << list.at(i).toString().toAscii().data() << endl;
//! [15]
    }
}

void QSparqlQueryModel_snippets()
{
    QSparqlConnection connection("QSPARQL_ENDPOINT");
    {
//! [16]
    QSparqlQueryModel *model = new QSparqlQueryModel;
    QSparqlQuery query("SELECT ?name ?surname WHERE { "
                       "?person foaf:name ?name; "
                       "foaf:surname ?surname . }");
    model->setQuery(query, connection);
    model->setHeaderData(0, Qt::Horizontal, tr("Name"));
    model->setHeaderData(1, Qt::Horizontal, tr("Surname"));

//! [17]
    QTableView *view = new QTableView;
//! [17] //! [18]
    view->setModel(model);
//! [18] //! [19]
    view->show();
//! [16] //! [19] //! [20]
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
//! [20]
    }

//! [21]
    QSparqlQueryModel model;
    QSparqlQuery query("select distinct ?person ?salary where { "
                       "?person dbpedia-owl:salary ?salary}");
    model.setQuery(query, connection);
    int salary = model.resultRow(4).value("salary").toInt();
//! [21]
    Q_UNUSED(salary);

    {
//! [22]
    int salary = model.data(model.index(4, 2)).toInt();
//! [22]
    Q_UNUSED(salary);
    }

    for (int row = 0; row < model.rowCount(); ++row) {
        for (int col = 0; col < model.columnCount(); ++col) {
            qDebug() << model.data(model.index(row, col));
        }
    }
}

class MyModel : public QSparqlQueryModel
{
public:
    QVariant data(const QModelIndex &item, int role) const;

    int m_specialColumnNo;
};

//! [23]
QVariant MyModel::data(const QModelIndex &item, int role) const
{
    if (item.column() == m_specialColumnNo) {
        // handle column separately
    }
    return QSparqlQueryModel::data(item, role);
}
//! [23]

void sql_intro_snippets()
{
    QSparqlResult *result;
    QSparqlConnectionOptions options;
    QSparqlConnection connection("QVIRTUOSO", options);
    {
//! [26]
    QSparqlConnectionOptions options;
    options.setHostName("acidalia");
    options.setDatabaseName("customdb");
    options.setUserName("mojito");
    options.setPassword("J0a1m8");
    QSparqlConnection conn("QVIRTUOSO", options);
    bool ok = conn.isValid();
//! [26]
    Q_UNUSED(ok);
    }

    {
//! [27]
    QSparqlConnection firstDB("QTRACKER");
    QSparqlConnection secondDB("QTRACKER");
//! [27]
    }

    {
    // SELECT1
//! [31]
    QSparqlQuery query("select distinct ?person ?salary where { "
                       "?person dbpedia-owl:salary ?salary . "
                       " FILTER (?salary > 5000)");
    result = connection.exec(query);
//! [31]

//! [32]
    while (result->next()) {
        QString name = result->value(0).toString();
        int salary = result->value(1).toInt();
        qDebug() << name << salary;
    }
//! [32]
    }

    {
    // FEATURE
//! [33]
    QSparqlQuery query("select distinct ?person ?salary where { "
                       "?person dbpedia-owl:salary ?salary . "
                       " FILTER (?salary > 5000)");
    int numRows;
    result = connection.exec(query);

    if (connection.hasFeature(QSparqlConnection::QuerySize)) {
        numRows = result->size();
    } else {
        // this can be very slow
        result->last();
        numRows = result->pos() + 1;
    }
//! [33]
    }

    {
    // INSERT1
//! [34]
    QSparqlQuery query("insert into <http://example.org/contacts#> "
                       "{ <http://example.org/foaf/Thad_Beaumont> a nco:PersonContact; "
                       "nco:nameGiven \"Thad Beaumont\" . }",
                       QSparqlQuery::InsertStatement);
    result = connection.exec(query);
//! [34]
    }


    {
    // UPDATE1
//! [37]
    QSparqlQuery query("WITH <http://example/addresses> "
                       "DELETE { ?person foaf:firstName 'Bill' } "
                       "INSERT { ?person foaf:firstName 'William' } "
                       "WHERE "
                       "{ ?person a foaf:Person . "
                       "  ?person foaf:firstName 'Bill' } ", 
                       QSparqlQuery::InsertStatement);
    result = connection.exec(query);
//! [37]
    }

    {
    // DELETE1
//! [38]
    QSparqlQuery del("DELETE FROM GRAPH <http://example.org/contacts#> "
                     "{ <uri001> ?p ?o . } "
                     "FROM <http://example.org/contacts#> "
                     "WHERE { <uri001> ?p ?o . }",
                     QSparqlQuery::DeleteStatement);
    result = connection.exec(query);
//! [38]
    }

    {
    // SQLQUERYMODEL1
//! [40]
    QSparqlQueryModel model;
    QSparqlQuery query("select distinct ?person ?salary where { "
                       "?person dbpedia-owl:salary ?salary . }");
    model.setQuery(query, connection);

    for (int i = 0; i < model.rowCount(); ++i) {
        int id = model.resultRow(i).value("salary").toInt();
        QString name = model.resultRow(i).value("person").toString();
        qDebug() << id << name;
    }
//! [40]
    }

}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QSparqlConnection_snippets();
    QSparqlBinding_snippets();
    QSparqlQuery_snippets();
    QSparqlQueryModel_snippets();
}
