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

class tst_QSparqlEndpoint : public QObject
{
    Q_OBJECT

public:
    tst_QSparqlEndpoint();
    virtual ~tst_QSparqlEndpoint();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void query_places_of_birth();
    void ask_current_member();

    void query_with_error();
};

tst_QSparqlEndpoint::tst_QSparqlEndpoint()
{
}

tst_QSparqlEndpoint::~tst_QSparqlEndpoint()
{
}

void tst_QSparqlEndpoint::initTestCase()
{
}

void tst_QSparqlEndpoint::cleanupTestCase()
{
}

void tst_QSparqlEndpoint::init()
{
}

void tst_QSparqlEndpoint::cleanup()
{
}

void tst_QSparqlEndpoint::query_places_of_birth()
{
    QSparqlConnectionOptions options;
    options.setHostName("dbpedia.org");
    QSparqlConnection conn("QENDPOINT", options);

    QSparqlQuery q("SELECT DISTINCT ?Object ?PlaceOfBirth "
                   "WHERE { "
                   "<http://dbpedia.org/resource/The_Beatles> <http://dbpedia.org/property/currentMembers> ?Object . "
                   "?Object <http://dbpedia.org/ontology/birthPlace> ?PlaceOfBirth . }");
    QSparqlResult* r = conn.exec(q);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished(); // this test is synchronous only
    QCOMPARE(r->hasError(), false);
    QCOMPARE(r->size(), 4);
    QHash<QString, QString> placesOfBirth;
    while (r->next()) {
        QCOMPARE(r->resultRow().count(), 2);
        placesOfBirth[r->resultRow().binding(0).toString()] = r->resultRow().binding(1).toString();
    }
    QCOMPARE(placesOfBirth.size(), 4);
    QCOMPARE(placesOfBirth["<http://dbpedia.org/resource/George_Harrison>"], QString("<http://dbpedia.org/resource/Wavertree>"));
    QCOMPARE(placesOfBirth["<http://dbpedia.org/resource/John_Lennon>"], QString("<http://dbpedia.org/resource/Liverpool>"));
    QCOMPARE(placesOfBirth["<http://dbpedia.org/resource/Ringo_Starr>"], QString("<http://dbpedia.org/resource/Dingle%252C_Liverpool>"));
    QCOMPARE(placesOfBirth["<http://dbpedia.org/resource/Paul_McCartney>"], QString("<http://dbpedia.org/resource/Liverpool>"));
    delete r;
}

void tst_QSparqlEndpoint::ask_current_member()
{
    QSparqlConnectionOptions options;
    options.setHostName("dbpedia.org");
    QSparqlConnection conn("QENDPOINT", options);

    QSparqlQuery add1("ASK { <http://dbpedia.org/resource/The_Beatles> <http://dbpedia.org/property/currentMembers> <http://dbpedia.org/resource/Ringo_Starr> . }",
                     QSparqlQuery::AskStatement);

    QSparqlResult* r = conn.exec(add1);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished(); // this test is synchronous only
    QCOMPARE(r->hasError(), false);
    QCOMPARE(r->boolValue(), true);
    delete r;
    
    QSparqlQuery add2("ASK { <http://dbpedia.org/resource/The_Beatles> <http://dbpedia.org/property/currentMembers> <http://dbpedia.org/resource/Pete_Best> . }",
                     QSparqlQuery::AskStatement);

    r = conn.exec(add2);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished(); // this test is synchronous only
    QCOMPARE(r->hasError(), false);
    QCOMPARE(r->boolValue(), false);    
    delete r;
}

void tst_QSparqlEndpoint::query_with_error()
{
    QSparqlConnectionOptions options;
    options.setHostName("dbpedia.org");
    QSparqlConnection conn("QENDPOINT", options);

    QSparqlQuery q("this is not a valid query");
    QSparqlResult* r = conn.exec(q);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished(); // this test is synchronous only
    QCOMPARE(r->hasError(), true);
    QCOMPARE(r->lastError().type(), QSparqlError::BackendError);
    delete r;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    // TODO: There should probably be a better way of doing this:
    app.addLibraryPath("../../../plugins");
    tst_QSparqlEndpoint tc; 
    return QTest::qExec(&tc, argc, argv);
}

// QTEST_MAIN( tst_QSparqlEndpoint )
#include "tst_qsparql_endpoint.moc"
