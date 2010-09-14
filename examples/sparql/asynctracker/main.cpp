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

#include <QDebug>
#include <QtSparql>
#include <QCoreApplication>

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
        qDebug() << "\t" << i << rr.binding(i).toString();
}

class MyObject : public QObject
{
Q_OBJECT
public slots:
    void onFinished() {
        QSparqlResult* r = qobject_cast<QSparqlResult *>(sender());
        while (r->next()) {
            printPosition(r);
            printResultRow(r->current());
        }
        // Then the query is positioned "after the last row"
        printPosition(r);
        printResultRow(r->current());
        QCoreApplication::exit(0);
    }
    void onResultsReadyAt(int start, int end) {
        qDebug() << "Results ready" << start << end;
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QSparqlConnection conn("QTRACKER");
    QSparqlQuery query("select ?u ?p { ?u a nco:Contact ; nco:hasPhoneNumber ?pn . "
                       "?pn a nco:PhoneNumber ; nco:phoneNumber ?p }");
    QSparqlResult* result = conn.exec(query);
    MyObject obj;
    QObject::connect(result, SIGNAL(finished()), &obj, SLOT(onFinished()));
    QObject::connect(result, SIGNAL(resultsReadyAt(int, int)),
                     &obj, SLOT(onResultsReadyAt(int, int)));

    app.exec();
    qDebug() << "Exiting";
    delete result;
    return 0;
}

#include "moc_main.cpp"
