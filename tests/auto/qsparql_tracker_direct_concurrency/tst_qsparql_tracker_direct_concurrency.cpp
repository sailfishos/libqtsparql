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

#include "../testhelpers.h"
#include "querytester.h"
#include "updatetester.h"
#include "resultchecker.h"
#include "../utils/testdata.h"

#include <QtTest/QtTest>
#include <QtSparql/QtSparql>

#define TEST_DATA_AMOUNT 3000

class tst_QSparqlTrackerDirectConcurrency : public QObject
{
    Q_OBJECT
public:
    tst_QSparqlTrackerDirectConcurrency();
    virtual ~tst_QSparqlTrackerDirectConcurrency();

private:
    TestData *testData;
    void createTrackerTestData();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void sameConnection_selectQueries();
    void sameConnection_selectQueries_data();
    void sameConnection_updateQueries();
    void sameConnection_updateQueries_data();

    void multipleConnections_selectQueries();
    void multipleConnections_selectQueries_data();
    void multipleConnections_updateQueries();
    void multipleConnections_updateQueries_data();

    void multipleConnections_multipleThreads_selectQueries();
    void multipleConnections_multipleThreads_selectQueries_data();
    void multipleConnections_multipleThreads_updateQueries();
    void multipleConnections_multipleThreads_updateQueries_data();
};

namespace {

void waitForAllFinished(const QList<QThread*>& threads, int timeoutMs)
{
    Q_FOREACH(QThread* thread, threads) {
        QVERIFY( thread->wait(timeoutMs) );
    }
}

} //end namespace

tst_QSparqlTrackerDirectConcurrency::tst_QSparqlTrackerDirectConcurrency()
{
}

tst_QSparqlTrackerDirectConcurrency::~tst_QSparqlTrackerDirectConcurrency()
{
}

void tst_QSparqlTrackerDirectConcurrency::initTestCase()
{
    // For running the test without installing the plugins. Should work in
    // normal and vpath builds.
    QCoreApplication::addLibraryPath("../../../plugins");
    testData = 0;

    // delete any previous pieces of test data that may have been left
    // over if the test was killed prematurely
    QSparqlConnection conn("QTRACKER_DIRECT");
    QString deleteString = "delete { ?u a rdfs:Resource } WHERE { ?u nie:isLogicalPartOf <qsparql-tracker-direct-tests-concurrency-stress> }";
    QSparqlResult *result = conn.syncExec(QSparqlQuery(deleteString, QSparqlQuery::DeleteStatement));
    QVERIFY(!result->hasError());
    delete result;
}

void tst_QSparqlTrackerDirectConcurrency::cleanupTestCase()
{
    if (testData) {
        delete testData;
        testData = 0;
    }
}

void tst_QSparqlTrackerDirectConcurrency::createTrackerTestData()
{
    if (!testData) {
        const QString testTag("<qsparql-tracker-direct-tests-concurrency-stress>");
        testData = TestData::createTrackerTestData(TEST_DATA_AMOUNT, "<qsparql-tracker-direct-tests>", testTag);
        QTest::qWait(2000);
        QVERIFY( testData->isOK() );
    }
}

void tst_QSparqlTrackerDirectConcurrency::init()
{
}

void tst_QSparqlTrackerDirectConcurrency::cleanup()
{
}

void tst_QSparqlTrackerDirectConcurrency::sameConnection_selectQueries()
{
    QFETCH(int, testDataAmount);
    QFETCH(int, numQueries);
    QFETCH(int, maxThreadCount);
    QFETCH(bool, forwardOnly);
    const int dataReadyInterval = qMax(testDataAmount/100, 10);

    QSparqlConnectionOptions options;
    options.setDataReadyInterval(dataReadyInterval);
    if (maxThreadCount > 0)
        options.setMaxThreadCount(maxThreadCount);
    QSparqlConnection conn("QTRACKER_DIRECT", options);

    QueryTester queryTester;
    queryTester.setConnection(&conn);
    queryTester.setParameters(numQueries, testDataAmount, forwardOnly);

    queryTester.startQueries();
    QVERIFY(queryTester.waitForAllFinished(8000));
}

void tst_QSparqlTrackerDirectConcurrency::sameConnection_selectQueries_data()
{
    createTrackerTestData();
    QTest::addColumn<int>("testDataAmount");
    QTest::addColumn<int>("numQueries");
    QTest::addColumn<int>("maxThreadCount");
    QTest::addColumn<bool>("forwardOnly");

    QTest::newRow("10 queries, 1 Thread") <<
        TEST_DATA_AMOUNT << 10 << 1 << false;
    QTest::newRow("10 queries, 1 Thread, forward only") <<
        TEST_DATA_AMOUNT << 10 << 1 << true;
    QTest::newRow("100 queries, 1 Thread") <<
        TEST_DATA_AMOUNT << 100 << 1 << false;
    QTest::newRow("100 queries, 1 Thread, forward only") <<
        TEST_DATA_AMOUNT << 100 << 1 << true;
    QTest::newRow("10 queries, default number of threads") <<
        TEST_DATA_AMOUNT << 10 << 0 << false;
    QTest::newRow("10 queries, default number of threads, forward only") <<
        TEST_DATA_AMOUNT << 10 << 0 << true;
    QTest::newRow("100 queries, default number of threads") <<
        TEST_DATA_AMOUNT << 100 << 0 << false;
    QTest::newRow("100 queries, default number of threads, forward only") <<
        TEST_DATA_AMOUNT << 100 << 0 << true;
    QTest::newRow("10 queries, 4 Threads") <<
        TEST_DATA_AMOUNT << 10 << 4 << false;
    QTest::newRow("10 queries, 4 Threads, forward only") <<
        TEST_DATA_AMOUNT << 10 << 4 << true;
    QTest::newRow("100 queries, 4 Threads") <<
        TEST_DATA_AMOUNT << 100 << 4 << false;
    QTest::newRow("100 queries, 4 Threads, forward only") <<
        TEST_DATA_AMOUNT << 100 << 4 << true;
}

void tst_QSparqlTrackerDirectConcurrency::sameConnection_updateQueries()
{
    QFETCH(int, numInserts);
    QFETCH(int, numDeletes);
    QSparqlConnection connection("QTRACKER_DIRECT");

    UpdateTester updateTester(1);
    updateTester.setParameters(numInserts, numDeletes);
    updateTester.setConnection(&connection);
    updateTester.run();
}

void tst_QSparqlTrackerDirectConcurrency::sameConnection_updateQueries_data()
{
    QTest::addColumn<int>("numInserts");
    QTest::addColumn<int>("numDeletes");

    QTest::newRow("10 inserts, 10 deletes") <<
        10 << 10;
    QTest::newRow("100 inserts, 100 deletes") <<
        100 << 100;
    QTest::newRow("250 inserts, 250 deletes") <<
        250 << 250;
}

void tst_QSparqlTrackerDirectConcurrency::multipleConnections_selectQueries()
{
    QFETCH(int, testDataAmount);
    QFETCH(int, numQueries);
    QFETCH(int, numConnections);
    QFETCH(bool, forwardOnly);
    QList<QueryTester*> queryTesters;

    // Create the connections and start the queries
    for (int i = 0; i < numConnections; ++i) {
        QueryTester* queryTester = new QueryTester;
        queryTester->setParameters(numQueries, testDataAmount, forwardOnly);
        queryTesters << queryTester;
        queryTester->startQueries();
    }

    // Wait for all the queries to finish
    Q_FOREACH(QueryTester* queryTester, queryTesters) {
        QVERIFY(queryTester->waitForAllFinished(60000));
    }

    qDeleteAll(queryTesters);
}

void tst_QSparqlTrackerDirectConcurrency::multipleConnections_selectQueries_data()
{
    createTrackerTestData();
    QTest::addColumn<int>("testDataAmount");
    QTest::addColumn<int>("numQueries");
    QTest::addColumn<int>("numConnections");
    QTest::addColumn<bool>("forwardOnly");

    QTest::newRow("10 queries, 2 connections") <<
        TEST_DATA_AMOUNT << 10 << 2 << false;
    QTest::newRow("10 queries, 2 connections, forward only") <<
        TEST_DATA_AMOUNT << 10 << 2 << true;
    QTest::newRow("100 queries, 2 connections") <<
        TEST_DATA_AMOUNT << 100 << 2 << false;
    QTest::newRow("100 queries, 2 connections, forward only") <<
        TEST_DATA_AMOUNT << 100 << 2 << true;
    QTest::newRow("10 queries, 4 connections") <<
        TEST_DATA_AMOUNT << 10 << 4 << false;
    QTest::newRow("10 queries, 4 connections, forward only") <<
        TEST_DATA_AMOUNT << 10 << 4 << true;
    QTest::newRow("100 queries, 4 connections") <<
        TEST_DATA_AMOUNT << 100 << 4 << false;
    QTest::newRow("100 queries, 4 connections, forward only") <<
        TEST_DATA_AMOUNT << 100 << 4 << true;
}

void tst_QSparqlTrackerDirectConcurrency::multipleConnections_updateQueries()
{
    QFETCH(int, numInserts);
    QFETCH(int, numDeletes);
    QFETCH(int, numConnections);

    QList<QSparqlConnection*> connections;
    QList<UpdateTester*> updateTesters;

    for (int i = 0; i < numConnections; ++i) {
        QSparqlConnection* conn = new QSparqlConnection("QTRACKER_DIRECT");
        connections << conn;

        UpdateTester* updateTester = new UpdateTester(i);
        updateTester->setConnection(conn);
        updateTester->setParameters(numInserts, numDeletes);
        updateTesters << updateTester;
        updateTester->start();
    }
    Q_FOREACH(UpdateTester* updateTester, updateTesters) {
        updateTester->waitForFinished();
        updateTester->cleanup();
    }

    qDeleteAll(updateTesters);
    qDeleteAll(connections);
}

void tst_QSparqlTrackerDirectConcurrency::multipleConnections_updateQueries_data()
{
    QTest::addColumn<int>("numInserts");
    QTest::addColumn<int>("numDeletes");
    QTest::addColumn<int>("numConnections");

    QTest::newRow("10 inserts, 10 deletes, 2 connections") <<
        10 << 10 << 2;
    QTest::newRow("100 inserts, 100 deletes, 2 connections") <<
        100 << 100 << 2;
    QTest::newRow("250 inserts, 250 deletes, 2 connections") <<
        250 << 250 << 2;
    QTest::newRow("10 inserts, 10 deletes, 4 connections") <<
        10 << 10 << 4;
    QTest::newRow("100 inserts, 100 deletes, 4 connections") <<
        100 << 100 << 4;
    QTest::newRow("250 inserts, 250 deletes, 4 connections") <<
        250 << 250 << 4;

}

void tst_QSparqlTrackerDirectConcurrency::multipleConnections_multipleThreads_selectQueries()
{
    QFETCH(int, testDataAmount);
    QFETCH(int, numQueries);
    QFETCH(int, numThreads);
    QFETCH(bool, forwardOnly);

    QList<QThread*> createdThreads;
    QList<QueryTester*> queryTesters;
    for (int i=0;i<numThreads;i++) {
        QThread *newThread = new QThread;
        createdThreads.append(newThread);

        QueryTester *queryTester = new QueryTester;
        queryTester->setParameters(numQueries, testDataAmount, forwardOnly);
        queryTesters.append(queryTester);
        queryTester->startInThread(newThread);
    }
    // start all the threads
    Q_FOREACH(QThread* thread, createdThreads) {
        thread->start();
    }

    waitForAllFinished(createdThreads, 60000*numThreads);
    qDeleteAll(queryTesters);
    qDeleteAll(createdThreads);
}

void tst_QSparqlTrackerDirectConcurrency::multipleConnections_multipleThreads_selectQueries_data()
{
    createTrackerTestData();
    QTest::addColumn<int>("testDataAmount");
    QTest::addColumn<int>("numQueries");
    QTest::addColumn<int>("numThreads");
    QTest::addColumn<bool>("forwardOnly");

    QTest::newRow("10 queries, 2 Threads") <<
        TEST_DATA_AMOUNT << 10 << 2 << false;
    QTest::newRow("10 queries, 2 Threads, forward only") <<
        TEST_DATA_AMOUNT << 10 << 2 << true;
    QTest::newRow("100 queries, 2 Threads") <<
        TEST_DATA_AMOUNT << 100 << 2 << false;
    QTest::newRow("100 queries, 2 Threads, forward only") <<
        TEST_DATA_AMOUNT << 100 << 2 << true;
    QTest::newRow("10 queries, 4 Threads") <<
        TEST_DATA_AMOUNT << 10 << 4 << false;
    QTest::newRow("10 queries, 4 Threads, forward only") <<
        TEST_DATA_AMOUNT << 10 << 4 << true;
    QTest::newRow("100 queries, 4 Threads") <<
        TEST_DATA_AMOUNT << 100 << 4 << false;
    QTest::newRow("100 queries, 4 Threads, forward only") <<
        TEST_DATA_AMOUNT << 100 << 4 << true;
}

void tst_QSparqlTrackerDirectConcurrency::multipleConnections_multipleThreads_updateQueries()
{
    QFETCH(int, numThreads);
    QFETCH(int, numInserts);
    QFETCH(int, numDeletes);

    QList<QThread*> createdThreads;
    QList<UpdateTester*> updateTesters;
    for (int i=0;i<numThreads;i++) {
        QThread* newThread = new QThread;
        createdThreads.append(newThread);

        UpdateTester* updateTester = new UpdateTester(i);
        updateTester->setParameters(numInserts, numDeletes);
        updateTesters.append(updateTester);
        updateTester->startInThread(newThread);
    }
    // start all the threads
    Q_FOREACH(QThread* thread, createdThreads) {
        thread->start();
    }

    waitForAllFinished(createdThreads, 60000*numThreads);
    qDeleteAll(updateTesters);
    qDeleteAll(createdThreads);
}

void tst_QSparqlTrackerDirectConcurrency::multipleConnections_multipleThreads_updateQueries_data()
{
    QTest::addColumn<int>("numThreads");
    QTest::addColumn<int>("numInserts");
    QTest::addColumn<int>("numDeletes");

    QTest::newRow("1 Thread, 10 inserts, 10 deletes") <<
        1 << 10 << 10;
    QTest::newRow("2 Threads, 100 inserts, 100 deletes") <<
        2 << 100 << 100;
    QTest::newRow("2 Threads, 250 inserts, 250 deletes") <<
        2 << 250 << 250;
    QTest::newRow("4 Threads, 100 inserts, 100 deletes") <<
        4 << 100 << 100;
    QTest::newRow("4 Threads, 250 inserts, 250 deletes") <<
        4 << 250 << 250;
}

QTEST_MAIN( tst_QSparqlTrackerDirectConcurrency )
#include "tst_qsparql_tracker_direct_concurrency.moc"
