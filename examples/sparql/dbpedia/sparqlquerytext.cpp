/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (ivan.frade@nokia.com)
**
** This file is part of the examples of the QtSparql module (not yet part of the Qt Toolkit).
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at ivan.frade@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/
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


