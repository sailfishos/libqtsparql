/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (ivan.frade@nokia.com)
**
** This file is part of the test suite of the QtSparql module (not yet part of the Qt Toolkit).
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

#include <QtTest/QtTest>
#include <QtSparql/QtSparql>

//const QString qtest(qTableName( "qtest", __FILE__ )); // FIXME: what's this

//TESTED_FILES=

class tst_QSparqlVirtuoso : public QObject
{
    Q_OBJECT

public:
    tst_QSparqlVirtuoso();
    virtual ~tst_QSparqlVirtuoso();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void query_contacts();
    void insert_and_delete_contact();
    void query_with_error();
};

tst_QSparqlVirtuoso::tst_QSparqlVirtuoso()
{
}

tst_QSparqlVirtuoso::~tst_QSparqlVirtuoso()
{
}

void tst_QSparqlVirtuoso::initTestCase()
{
}

void tst_QSparqlVirtuoso::cleanupTestCase()
{
}

void tst_QSparqlVirtuoso::init()
{
}

void tst_QSparqlVirtuoso::cleanup()
{
}

void tst_QSparqlVirtuoso::query_contacts()
{
    QSparqlConnectionOptions options;
    options.setDatabaseName("DRIVER=/usr/lib/odbc/virtodbc_r.so");
    QSparqlConnection conn("QVIRTUOSO", options);

    QSparqlQuery q("prefix nco: <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#> "
                   "prefix nie: <http://www.semanticdesktop.org/ontologies/2007/01/19/nie#> "
                   "select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-virtuoso-tests> ;"
                   "nco:nameGiven ?ng .}");
    QSparqlResult* r = conn.exec(q);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished(); // this test is syncronous only
    QCOMPARE(r->hasError(), false);
    QCOMPARE(r->size(), 3);
    QHash<QString, QString> contactNames;
    while (r->next()) {
        QCOMPARE(r->resultRow().count(), 2);
        contactNames[r->value(0).toString()] = r->value(1).toString();
    }
    QCOMPARE(contactNames.size(), 3);
    QCOMPARE(contactNames["uri001"], QString("name001"));
    QCOMPARE(contactNames["uri002"], QString("name002"));
    QCOMPARE(contactNames["uri003"], QString("name003"));
    delete r;
}

void tst_QSparqlVirtuoso::insert_and_delete_contact()
{
    // This test will leave unclean test data into virtuoso if it crashes.
    QSparqlConnectionOptions options;
    options.setDatabaseName("DRIVER=/usr/lib/odbc/virtodbc_r.so");
    QSparqlConnection conn("QVIRTUOSO", options);

    QSparqlQuery add("prefix nco: <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#> "
                     "prefix nie: <http://www.semanticdesktop.org/ontologies/2007/01/19/nie#> "
                     "insert into <http://www.openlinksw.com/schemas/virtrdf#> "
                     "{ <addeduri001> a nco:PersonContact; "
                     "nie:isLogicalPartOf <qsparql-virtuoso-tests> ;"
                     "nco:nameGiven \"addedname001\" . }",
                     QSparqlQuery::InsertStatement);

    QSparqlResult* r = conn.exec(add);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished(); // this test is syncronous only
    QCOMPARE(r->hasError(), false);
    delete r;

    // Verify that the insertion succeeded
    QSparqlQuery q("prefix nco: <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#> "
                   "prefix nie: <http://www.semanticdesktop.org/ontologies/2007/01/19/nie#> "
                   "select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-virtuoso-tests> ;"
                   "nco:nameGiven ?ng .}");
    QHash<QString, QString> contactNames;
    r = conn.exec(q);
    QVERIFY(r != 0);
    r->waitForFinished();
    QCOMPARE(r->size(), 4);
    while (r->next()) {
        contactNames[r->value(0).toString()] = r->value(1).toString();
    }
    QCOMPARE(contactNames.size(), 4);
    QCOMPARE(contactNames["addeduri001"], QString("addedname001"));
    delete r;

    // Delete the uri
    QSparqlQuery del("DELETE FROM GRAPH <http://www.openlinksw.com/schemas/virtrdf#> "
                     "{ <addeduri001> ?p ?o . } "
                     "FROM <http://www.openlinksw.com/schemas/virtrdf#> "
                     "WHERE { <addeduri001> ?p ?o . }",
                     QSparqlQuery::DeleteStatement);

    r = conn.exec(del);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished(); // this test is syncronous only
    QCOMPARE(r->hasError(), false);
    delete r;

    // Verify that it got deleted
    contactNames.clear();
    r = conn.exec(q);
    QVERIFY(r != 0);
    r->waitForFinished();
    QCOMPARE(r->size(), 3);
    while (r->next()) {
        contactNames[r->value(0).toString()] = r->value(1).toString();
    }
    QCOMPARE(contactNames.size(), 3);
    delete r;
}

void tst_QSparqlVirtuoso::query_with_error()
{
    QSparqlConnectionOptions options;
    options.setDatabaseName("DRIVER=/usr/lib/odbc/virtodbc_r.so");
    QSparqlConnection conn("QVIRTUOSO", options);

    QSparqlQuery q("this is not a valid query");
    QSparqlResult* r = conn.exec(q);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished(); // this test is syncronous only
    QCOMPARE(r->hasError(), true);
    QCOMPARE(r->lastError().type(), QSparqlError::StatementError);
    delete r;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    // TODO: There should probably be a better way of doing this:
    app.addLibraryPath("../../../plugins");
    tst_QSparqlVirtuoso tc; 
    return QTest::qExec(&tc, argc, argv);
}

// QTEST_MAIN( tst_QSparqlVirtuoso )
#include "tst_qsparql_virtuoso.moc"
