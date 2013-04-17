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
#include <QtSparql>
#include "EndpointService.h"

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
    void select_query();
    void ask_query();
    void query_with_error();
    void select_query_server_not_responding();
    void connection_to_nonexisting_server();
    void destroy_connection();
    void broken_result();
    void update_query();
private:
    EndpointService *endpointService;
};

tst_QSparqlEndpoint::tst_QSparqlEndpoint()
{
}

tst_QSparqlEndpoint::~tst_QSparqlEndpoint()
{
}

void tst_QSparqlEndpoint::initTestCase()
{

    endpointService = new EndpointService(8080);
    endpointService->start();
    while (!endpointService->isRunning())
        QTest::qWait(100);

    // For running the test without installing the plugins. Should work in
    // normal and vpath builds.
    QCoreApplication::addLibraryPath("../../../plugins");
}

void tst_QSparqlEndpoint::cleanupTestCase()
{
    endpointService->stopService(2000);
    delete endpointService;
}

void tst_QSparqlEndpoint::init()
{
}

void tst_QSparqlEndpoint::cleanup()
{
}

void tst_QSparqlEndpoint::select_query()
{
    QSparqlConnectionOptions options;
    options.setPort(8080);
    options.setHostName("127.0.0.1");
    QSparqlConnection conn("QSPARQL_ENDPOINT", options);

    QSparqlQuery q("SELECT ?book ?who "
                   "WHERE { "
                   "?book a <http://www.example/Book> . "
                   "?who <http://www.example/Author> ?book . }");
    QSparqlResult* r = conn.exec(q);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished(); // this test is synchronous only
    QCOMPARE(r->hasError(), false);
    QCOMPARE(r->size(), 2);
    QHash<QString, QString> author;
    while (r->next()) {
        QCOMPARE(r->current().count(), 2);
        author[r->current().binding(0).toString()] = r->current().binding(1).toString();
    }
    QCOMPARE(author["<http://www.example/book/book5>"], QString("_:r29392923r2922"));
    QCOMPARE(author["<http://www.example/book/book6>"], QString("_:r8484882r49593"));

    delete r;
}

void tst_QSparqlEndpoint::ask_query()
{
    QSparqlConnectionOptions options;
    options.setPort(8080);
    options.setHostName("127.0.0.1");
    QSparqlConnection conn("QSPARQL_ENDPOINT", options);

    QSparqlQuery q("ASK WHERE { ?book <http://www.example/Author> \"J.K. Rowling\"} ", QSparqlQuery::AskStatement);
    QSparqlResult* r = conn.exec(q);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished();
    QCOMPARE(r->hasError(), false);
    QCOMPARE(r->isBool(), true);
    QCOMPARE(r->boolValue(), false);

    delete r;
}


void tst_QSparqlEndpoint::query_with_error()
{
    QSparqlConnectionOptions options;
    options.setPort(8080);
    options.setHostName("127.0.0.1");
    QSparqlConnection conn("QSPARQL_ENDPOINT", options);

    QSparqlQuery q("bad query");
    QSparqlResult* r = conn.exec(q);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished();
    QCOMPARE(r->hasError(), true);
    // TODO: Bug! Endpoint driver sets error to ConnectionError instead of StatementError
    //QCOMPARE(r->lastError().type(), QSparqlError::StatementError);

    delete r;
}

void tst_QSparqlEndpoint::select_query_server_not_responding()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QSKIP("Neither endpoint driver nor QNetworkAccessManager has timeout functionality");
#else
    QSKIP("Neither endpoint driver nor QNetworkAccessManager has timeout functionality", SkipAll);
#endif
    // When connection is established but server doesn't respond, client (endpoint driver) hangs
    QSparqlConnectionOptions options;
    options.setPort(8080);
    options.setHostName("127.0.0.1");
    QSparqlConnection conn("QSPARQL_ENDPOINT", options);

    QSparqlQuery q("SELECT ?book ?who "
                   "WHERE { "
                   "?book a <http://www.example/Book> . "
                   "?who <http://www.example/Author> ?book . }");

    endpointService->pause();
    QSparqlResult* r = conn.exec(q);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished(); // this test is synchronous only
    endpointService->resume();
    QCOMPARE(r->hasError(), true);
    delete r;
}

void tst_QSparqlEndpoint::connection_to_nonexisting_server()
{
    QSparqlConnectionOptions options;
    options.setPort(8085);
    options.setHostName("127.0.0.8");
    QSparqlConnection conn("QSPARQL_ENDPOINT", options);

    QSparqlQuery q("SELECT ?book ?who "
                   "WHERE { "
                   "?book a <http://www.example/Book> . "
                   "?who <http://www.example/Author> ?book . }");

    QSparqlResult* r = conn.exec(q);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished(); // this test is synchronous only
    QCOMPARE(r->hasError(), true);
    QCOMPARE(r->lastError().type(), QSparqlError::ConnectionError);
    delete r;
}

void tst_QSparqlEndpoint::destroy_connection()
{
    QSparqlConnectionOptions options;
    options.setPort(8080);
    options.setHostName("127.0.0.1");
    QSparqlConnection *conn = new QSparqlConnection("QSPARQL_ENDPOINT", options);

    QSparqlQuery q("SELECT ?book ?who "
                   "WHERE { "
                   "?book a <http://www.example/Book> . "
                   "?who <http://www.example/Author> ?book . }");
    QSparqlResult* r = conn->exec(q);
    const bool immediatelyFinished = r->isFinished();
    r->setParent(this);
    delete conn; conn = 0;
    QVERIFY(r != 0);
    if (immediatelyFinished) {
        QCOMPARE(r->hasError(), false);
        QCOMPARE(r->size(), 2);
        QHash<QString, QString> author;
        while (r->next()) {
            QCOMPARE(r->current().count(), 2);
            author[r->current().binding(0).toString()] = r->current().binding(1).toString();
        }
        QCOMPARE(author["<http://www.example/book/book5>"], QString("_:r29392923r2922"));
        QCOMPARE(author["<http://www.example/book/book6>"], QString("_:r8484882r49593"));
    } else {
        QCOMPARE(r->hasError(), true);
        QCOMPARE(r->lastError().type(), QSparqlError::ConnectionError);
    }
    delete r;
}

void tst_QSparqlEndpoint::broken_result()
{
    QSparqlConnectionOptions options;
    options.setPort(8080);
    options.setHostName("127.0.0.1");
    QSparqlConnection conn("QSPARQL_ENDPOINT", options);

    QSparqlQuery q("broken result");
    QSparqlResult* r = conn.exec(q);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished(); // this test is synchronous only
    QCOMPARE(r->hasError(), true);
    QCOMPARE(r->lastError().type(), QSparqlError::ConnectionError);
    delete r;
}

void tst_QSparqlEndpoint::update_query()
{
    QSparqlConnectionOptions options;
    options.setPort(8080);
    options.setHostName("127.0.0.1");
    QSparqlConnection conn("QSPARQL_ENDPOINT", options);

    QSparqlQuery q("INSERT {<http://www.example/book/book15> a <http://www.example/Book>}",
                   QSparqlQuery::InsertStatement);
    QSparqlResult* r = conn.exec(q);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished(); // this test is synchronous only
    QCOMPARE(r->hasError(), false);
    delete r;
}

QTEST_MAIN( tst_QSparqlEndpoint )
#include "tst_qsparql_endpoint.moc"
