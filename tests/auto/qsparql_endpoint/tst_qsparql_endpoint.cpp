/****************************************************************************
**
** Copyright (C) 2010-2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (ivan.frade@nokia.com)
**
** This file is part of the test suite of the QtSparql module (not yet part of the Qt Toolkit).
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
** If you have questions regarding the use of this file, please contact
** Nokia at ivan.frade@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtSparql/QtSparql>

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
    void construct_current_members();
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
    // For running the test without installing the plugins. Should work in
    // normal and vpath builds.
    QCoreApplication::addLibraryPath("../../../plugins");
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
    QSparqlConnection conn("QSPARQL_ENDPOINT", options);

    QSparqlQuery q("SELECT DISTINCT ?Object ?PlaceOfBirth "
                   "WHERE { "
                   "<http://dbpedia.org/resource/The_Beatles> <http://dbpedia.org/property/currentMembers> ?Object . "
                   "?Object <http://dbpedia.org/ontology/birthPlace> ?PlaceOfBirth . }");
    QSparqlResult* r = conn.exec(q);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished(); // this test is synchronous only
    QCOMPARE(r->hasError(), false);
    // dbpedia gives 5 results for the query, rather than 4
    // so comment this out for now
    //QCOMPARE(r->size(), 4);
    QHash<QString, QString> placesOfBirth;
    while (r->next()) {
        QCOMPARE(r->current().count(), 2);
        placesOfBirth[r->current().binding(0).toString()] = r->current().binding(1).toString();
    }
    // ringo isn't getting his place of birth returned
    // so comment this out for now
    // QCOMPARE(placesOfBirth.size(), 4);

    // George Harrison is having several locations for places of birth returned, so comment this out for now
    // QCOMPARE(placesOfBirth["<http://dbpedia.org/resource/George_Harrison>"], QString("<http://dbpedia.org/resource/Wavertree>"));
    QCOMPARE(placesOfBirth["<http://dbpedia.org/resource/John_Lennon>"], QString("<http://dbpedia.org/resource/Liverpool>"));
    // Commented out due to fault with dbpedia
    //QCOMPARE(placesOfBirth["<http://dbpedia.org/resource/Ringo_Starr>"], QString("<http://dbpedia.org/resource/Dingle%252C_Liverpool>"));
    QCOMPARE(placesOfBirth["<http://dbpedia.org/resource/Paul_McCartney>"], QString("<http://dbpedia.org/resource/Liverpool>"));
    delete r;
}

void tst_QSparqlEndpoint::construct_current_members()
{
    QSparqlConnectionOptions options;
    options.setHostName("dbpedia.org");
    QSparqlConnection conn("QSPARQL_ENDPOINT", options);

    QSparqlQuery q("CONSTRUCT { <http://dbpedia.org/resource/The_Beatles> <http://dbpedia.org/property/currentMembers> ?Object } "
                   "WHERE { <http://dbpedia.org/resource/The_Beatles> <http://dbpedia.org/property/currentMembers> ?Object . }",
                   QSparqlQuery::ConstructStatement );
    QSparqlResult* r = conn.exec(q);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished(); // this test is synchronous only
    QCOMPARE(r->hasError(), false);
    QCOMPARE(r->isGraph(), true);
    QCOMPARE(r->size(), 4);
    QStringList currentMembers;
    currentMembers << QLatin1String("<http://dbpedia.org/resource/George_Harrison>");
    currentMembers << QLatin1String("<http://dbpedia.org/resource/John_Lennon>");
    currentMembers << QLatin1String("<http://dbpedia.org/resource/Ringo_Starr>");
    currentMembers << QLatin1String("<http://dbpedia.org/resource/Paul_McCartney>");
    
    while (r->next()) {
        QCOMPARE(r->current().count(), 3);
        QCOMPARE(r->current().binding(0).name(), QString("s"));
        QCOMPARE(r->current().binding(0).toString(), QString("<http://dbpedia.org/resource/The_Beatles>"));
        
        QCOMPARE(r->current().binding(1).name(), QString("p"));
        QCOMPARE(r->current().binding(1).toString(), QString("<http://dbpedia.org/property/currentMembers>"));
        
        QCOMPARE(r->current().binding(2).name(), QString("o"));
        // Commented out due to fault with dbpedia
        //QCOMPARE((bool) currentMembers.contains(r->current().binding(2).toString()), true);
    }

    delete r;
}

void tst_QSparqlEndpoint::ask_current_member()
{
    QSparqlConnectionOptions options;
    options.setHostName("dbpedia.org");
    QSparqlConnection conn("QSPARQL_ENDPOINT", options);

    QSparqlQuery add1("ASK { <http://dbpedia.org/resource/The_Beatles> <http://dbpedia.org/property/currentMembers> <http://dbpedia.org/resource/George_Harrison> . }",
                     QSparqlQuery::AskStatement);

    QSparqlResult* r = conn.exec(add1);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished(); // this test is synchronous only
    QCOMPARE(r->hasError(), false);
    QCOMPARE(r->isBool(), true);
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
    QSparqlConnection conn("QSPARQL_ENDPOINT", options);

    QSparqlQuery q("this is not a valid query");
    QSparqlResult* r = conn.exec(q);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished(); // this test is synchronous only
    QCOMPARE(r->hasError(), true);
    QCOMPARE(r->lastError().type(), QSparqlError::StatementError);
    delete r;
}

QTEST_MAIN( tst_QSparqlEndpoint )
#include "tst_qsparql_endpoint.moc"
