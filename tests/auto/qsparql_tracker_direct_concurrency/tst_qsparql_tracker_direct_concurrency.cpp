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
#include "../tracker_direct_common.h"

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

    void multipleConnections_multipleThreads_selectQueries();
    void multipleConnections_multipleThreads_selectQueries_data();
    void multipleConnections_multipleThreads_updateQueries();
    void multipleConnections_multipleThreads_updateQueries_data();
};

namespace {

void seedRandomRangeGenerator()
{
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());
}

QPair<int,int> randomRangeIn(int max)
{
    const int low = qrand() % (max-1) + 1;
    const int high = qrand() % ((max+1) - low) + low;
    return qMakePair(low, high);
}

void waitForAllFinished(const QList<QThread*>& threads, int timeoutMs)
{
    Q_FOREACH(QThread* thread, threads) {
        QVERIFY( thread->wait(timeoutMs) );
    }
}

class ResultChecker : public QObject
{
    Q_OBJECT
public:
    QSignalMapper dataReadyMapper;
    QSignalMapper finishedMapper;
    QList<QSparqlResult*> allResults;
    QHash<QSparqlResult*, QPair<int,int> > pendingResults;

    ResultChecker()
        // Need to set parent on signalMappers to ensure they are moved to test thread with this object
        : dataReadyMapper(this), finishedMapper(this)
    {
        connect(&dataReadyMapper, SIGNAL(mapped(QObject*)), this, SLOT(onDataReady(QObject*)));
        connect(&finishedMapper, SIGNAL(mapped(QObject*)), this, SLOT(onFinished(QObject*)));
    }

    ~ResultChecker()
    {
        pendingResults.clear();
        qDeleteAll(allResults);
    }

    void append(QSparqlResult *r, const QPair<int, int>& range)
    {
        QVERIFY( r );
        QVERIFY( !r->hasError() );
        dataReadyMapper.setMapping(r, r);
        connect(r, SIGNAL(dataReady(int)), &dataReadyMapper, SLOT(map()));
        finishedMapper.setMapping(r, r);
        connect(r, SIGNAL(finished()), &finishedMapper, SLOT(map()));
        allResults.append(r);
        pendingResults.insert(r, range);
    }

    bool waitForAllFinished(int silenceTimeoutMs)
    {
        QTime timeoutTimer;
        timeoutTimer.start();
        bool timeout = false;
        while (!pendingResults.empty() && !timeout) {
            const int pendingResultsCountBefore = pendingResults.count();
            QTest::qWait(silenceTimeoutMs / 10);
            if (pendingResults.count() < pendingResultsCountBefore) {
                timeoutTimer.restart();
            }
            else if (timeoutTimer.elapsed() > silenceTimeoutMs) {
                timeout = true;
            }
        }
        return !timeout;
    }

Q_SIGNALS:
    void allFinished();

public Q_SLOTS:
    void onDataReady(QObject* mappedResult)
    {
        QSparqlResult *result = qobject_cast<QSparqlResult*>(mappedResult);
        while (result->next()) {
            // just do something pointless with the result
            result->value(1).toInt();
            result->value(0).toString();
        }
    }

    void onFinished(QObject* mappedResult)
    {
        QSparqlResult *result = qobject_cast<QSparqlResult*>(mappedResult);
        QVERIFY( result );
        QVERIFY( !result->hasError() );
        QVERIFY( pendingResults.contains(result) );

        const QPair<int, int> resultRange = pendingResults.value(result);
        const int expectedResultSize = (resultRange.second - resultRange.first) + 1;
        QCOMPARE(result->size(), expectedResultSize);

        // the results should have been fully nexted in the data ready function
        QCOMPARE(result->pos(), int(QSparql::AfterLastRow));
        // go back through the results and validate that they are in range
        int resultCount = 0;
        while (result->previous()) {
            //we don't know the order, so just ensure the result is within range
            QVERIFY(result->value(1).toInt() >= resultRange.first);
            QVERIFY(result->value(1).toInt() <= resultRange.second);
            resultCount++;
        }
        // now make sure the results counted match the size
        QCOMPARE(resultCount, expectedResultSize);

        pendingResults.remove(result);
        if (pendingResults.empty())
            Q_EMIT allFinished();
    }
};

void startConcurrentQueries(QSparqlConnection& conn, ResultChecker& resultChecker, int numQueries, int testDataAmount);

class ThreadQueryRunner : public QObject
{
    Q_OBJECT

    QSparqlConnection *connection;
    QSparqlConnection *ownConnection;
    ResultChecker *resultChecker;
    ResultChecker *ownResultChecker;

    int numQueries;
    int testDataSize;

public:
    ThreadQueryRunner()
        : connection(0), ownConnection(0), resultChecker(0), ownResultChecker(0)
    {
    }

    ~ThreadQueryRunner()
    {
        cleanup();
    }

    void cleanup()
    {
        delete ownResultChecker;
        ownResultChecker = 0;
        delete ownConnection;
        ownConnection = 0;
    }

    void setParameters(int numQueries, int testDataSize)
    {
        this->numQueries = numQueries;
        this->testDataSize = testDataSize;
    }

    void setConnection(QSparqlConnection* connection)
    {
        this->connection = connection;
        delete ownConnection;
        ownConnection = 0;
    }

    void setResultChecker(ResultChecker* resultChecker)
    {
        this->resultChecker = resultChecker;
        delete ownResultChecker;
        ownResultChecker = 0;
    }

public Q_SLOTS:
    void startQueries()
    {
        if (!connection) {
            const int dataReadyInterval = qMax(testDataSize/100, 10);
            QSparqlConnectionOptions options;
            options.setDataReadyInterval(dataReadyInterval);
            ownConnection = new QSparqlConnection("QTRACKER_DIRECT", options);
            connection = ownConnection;
        }
        if (!resultChecker) {
            ownResultChecker = new ResultChecker;
            resultChecker = ownResultChecker;
        }

        connect(resultChecker, SIGNAL(allFinished()), this, SLOT(quit()));
        startConcurrentQueries(*connection, *resultChecker, numQueries, testDataSize);
    }

private Q_SLOTS:
    void quit()
    {
        cleanup();
        this->thread()->quit();
    }

};

class UpdateTester : public QObject
{
    Q_OBJECT
    QSparqlConnection *connection;
    QSparqlConnection *ownConnection;
    QList<QSparqlResult*> resultList;
    QSet<QSparqlResult*> pendingResults;
    QSignalMapper updateFinishedMapper;
    QSignalMapper deleteFinishedMapper;
    int numInserts;
    int numDeletes;
    int id;
    bool waiting;

public:
    UpdateTester(int id)
        : connection(0), ownConnection(0)
          // Need to set parents on signalMappers to ensure ther are moved to test thread with this object
        , updateFinishedMapper(this)
        , deleteFinishedMapper(this)
        , id(id), waiting(false)
    {
        connect(&updateFinishedMapper, SIGNAL(mapped(QObject*)), this, SLOT(onUpdateFinished(QObject*)));
        connect(&deleteFinishedMapper, SIGNAL(mapped(QObject*)), this, SLOT(onDeleteFinished(QObject*)));
    }

    void setParameters(int numInserts, int numDeletes)
    {
        QVERIFY(numDeletes <= numInserts);
        this->numInserts = numInserts;
        this->numDeletes = numDeletes;
    }

    void setConnection(QSparqlConnection *connection)
    {
        this->connection = connection;
        delete ownConnection;
        ownConnection = 0;
    }

    void waitForFinished()
    {
        waiting = true;
        while (waiting)
        {
            QTest::qWait(100);
        }
    }

    void startInThread(QThread* thread)
    {
        this->moveToThread(thread);
        connect(thread, SIGNAL(started()), this, SLOT(runInThread()));
    }

Q_SIGNALS:
    void updatesComplete();
    void validateUpdateComplete();
    void deletionsComplete();
    void validateDeletionComplete();
    void finished();

public Q_SLOTS:
    void start()
    {
        connect(this, SIGNAL(updatesComplete()), this, SLOT(startValidateUpdate()));
        connect(this, SIGNAL(validateUpdateComplete()), this, SLOT(startDeletions()));
        connect(this, SIGNAL(deletionsComplete()), this, SLOT(startValidateDeletion()));
        connect(this, SIGNAL(validateDeletionComplete()), this, SLOT(finish()));
        initConnection();
        startUpdates();
    }

    void run()
    {
        start();
        waitForFinished();
        cleanup();
    }

    void runInThread()
    {
        run();
        this->thread()->quit();
    }

    void cleanup()
    {
        if (connection && numInserts-numDeletes > 0) {
            QString deleteTemplate = "DELETE { ?u a nco:PersonContact }"
                                  " WHERE { ?u a nco:PersonContact; nie:isLogicalPartOf <qsparql-tracker-direct-concurrency-thread%1> . }";
            QSparqlQuery deleteQuery(deleteTemplate.arg(id), QSparqlQuery::DeleteStatement);
            QSparqlResult* result = connection->syncExec(deleteQuery);
            delete result;
        }

        qDeleteAll(resultList);
        resultList.clear();

        if (ownConnection) {
            delete ownConnection;
            ownConnection = connection = 0;
        }
    }

private Q_SLOTS:
    void initConnection()
    {
        if (!connection) {
            ownConnection = new QSparqlConnection("QTRACKER_DIRECT");
            connection = ownConnection;
        }
    }

    void startUpdates()
    {
        const QString insertTemplate = "insert { <addeduri00%1-%2> a nco:PersonContact; nie:isLogicalPartOf <qsparql-tracker-direct-concurrency-thread%2>;"
                                        "nco:nameGiven \"addedname00%1\"; nco:nameFamily \"addedFamily00%1\" . }";
        for (int i=0;i<numInserts;i++) {
            QSparqlQuery insertQuery(insertTemplate.arg(i).arg(id), QSparqlQuery::InsertStatement);
            QSparqlResult *result = connection->exec(insertQuery);
            QVERIFY( result );
            QVERIFY( !result->hasError() );
            appendPendingResult(result, &updateFinishedMapper);
        }
    }

    void startValidateUpdate()
    {
        QSparqlResult* result = connection->exec(QSparqlQuery(selectTemplate().arg(id)));
        connect(result, SIGNAL(finished()), this, SLOT(validateUpdateResult()));
        QVERIFY( result );
        QVERIFY( !result->hasError() );
        resultList.append(result);
    }

    void validateUpdateResult()
    {
        doValidateUpdateResult();
        Q_EMIT validateUpdateComplete();
    }

    void startDeletions()
    {
        const QString deleteTemplate = "delete { <addeduri00%1-%2> a nco:PersonContact }"
                                       " WHERE { <addeduri00%1-%2> a nco:PersonContact; nie:isLogicalPartOf <qsparql-tracker-direct-concurrency-thread%2> . }";
        for (int i=0;i<numDeletes;i++) {
            QSparqlQuery deleteQuery(deleteTemplate.arg(i).arg(id), QSparqlQuery::DeleteStatement);
            QSparqlResult* result = connection->exec(deleteQuery);
            QVERIFY( result );
            QVERIFY( !result->hasError() );
            appendPendingResult(result, &deleteFinishedMapper);
        }
    }

    void startValidateDeletion()
    {
        QSparqlResult* result = connection->exec(QSparqlQuery(selectTemplate().arg(id)));
        QVERIFY( result );
        QVERIFY( !result->hasError() );
        connect(result, SIGNAL(finished()), this, SLOT(validateDeleteResult()));
        resultList.append(result);
    }

    void validateDeleteResult()
    {
        doValidateDeleteResult();
        Q_EMIT validateDeletionComplete();
    }

    void onUpdateFinished(QObject* mappedResult)
    {
        checkIsPendingResult(mappedResult);
        if (removePendingResultWasLast(mappedResult))
            Q_EMIT updatesComplete();
    }

    void onDeleteFinished(QObject* mappedResult)
    {
        checkIsPendingResult(mappedResult);
        if (removePendingResultWasLast(mappedResult))
            Q_EMIT deletionsComplete();
    }

    void finish()
    {
        waiting = false;  // see waitForFinished()
        Q_EMIT finished();
    }

private:
    static QString selectTemplate()
    {
        return QString("select ?u ?ng ?nf { ?u a nco:PersonContact; nie:isLogicalPartOf <qsparql-tracker-direct-concurrency-thread%1>;"
                            "nco:nameGiven ?ng; nco:nameFamily ?nf . }");
    }

    void appendPendingResult(QSparqlResult* result, QSignalMapper* signalMapper)
    {
        QVERIFY( !pendingResults.contains(result) );
        resultList.append(result);
        pendingResults.insert(result);
        signalMapper->setMapping(result,result);
        connect(result, SIGNAL(finished()), signalMapper, SLOT(map()));
    }

    void checkIsPendingResult(QObject* mappedResult) const
    {
        QSparqlResult* result = qobject_cast<QSparqlResult*>(mappedResult);
        QVERIFY( result );
        QVERIFY( pendingResults.contains(result) );
    }

    bool removePendingResultWasLast(QObject* mappedResult)
    {
        QSparqlResult* result = qobject_cast<QSparqlResult*>(mappedResult);
        pendingResults.remove(result);
        return pendingResults.isEmpty();
    }

    void doValidateUpdateResult()
    {
        QSparqlResult* result = resultList.back();
        QVERIFY( !result->hasError() );
        QHash<QString, QString> contactNameValues;
        QCOMPARE(result->size(), numInserts);
        while (result->next()) {
            contactNameValues[result->value(0).toString()] = result->value(1).toString();
        }
        QCOMPARE(contactNameValues.size(), numInserts);
        for(int i=0; i<numInserts; i++) {
            QCOMPARE(contactNameValues[QString("addeduri00%1-%2").arg(i).arg(id)], QString("addedname00%1").arg(i));
        }
    }

    void doValidateDeleteResult()
    {
        QSparqlResult* result = resultList.back();
        QVERIFY( !result->hasError() );
        QCOMPARE(result->size(), numInserts-numDeletes);
        QHash<QString, QString> contactNameValues;
        while (result->next()) {
            contactNameValues[result->value(0).toString()] = result->value(1).toString();
        }
        QCOMPARE(contactNameValues.size(), numInserts-numDeletes);
        // number of deletes might be less than the number of inserts, we delete from 0 -> numDeletes-1, so
        int startFrom = numInserts - numDeletes;
        if (startFrom != 0) {
            for (int i=startFrom;i<numInserts;i++) {
                QCOMPARE(contactNameValues[QString("addeduri00%1-%2").arg(i).arg(id)], QString("addedname00%1").arg(i));
            }
        }
    }
};

void startConcurrentQueries(QSparqlConnection& conn, ResultChecker& resultChecker, int numQueries, int testDataAmount)
{
    for (int i=0;i<numQueries;i++) {
        QPair<int, int> resultRange;
        QString filter;
        if (i % 10 == 0) {
            // Override every 10th result to read all
            resultRange = qMakePair(1, testDataAmount);
            filter = ". }";
        }
        else {
            resultRange  = randomRangeIn(testDataAmount);
            filter = QString("FILTER ( ?t >=%1 && ?t <=%2 ) }").arg(resultRange.first).arg(resultRange.second);
        }
        QSparqlQuery select(QString("select ?u ?t {?u a nmm:MusicPiece; "
                                    "nmm:trackNumber ?t; "
                                    "nie:isLogicalPartOf <qsparql-tracker-direct-tests-concurrency-stress> "
                                    + filter));
        QSparqlResult *result = conn.exec(select);
        resultChecker.append(result, resultRange);
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
    seedRandomRangeGenerator();
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
        testData = createTestData(TEST_DATA_AMOUNT, "<qsparql-tracker-direct-tests>", testTag);
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
    const int dataReadyInterval = qMax(testDataAmount/100, 10);

    QSparqlConnectionOptions options;
    options.setDataReadyInterval(dataReadyInterval);
    options.setMaxThreadCount(maxThreadCount);
    QSparqlConnection conn("QTRACKER_DIRECT", options);
    ResultChecker resultChecker;

    startConcurrentQueries(conn, resultChecker, numQueries, testDataAmount);
    QVERIFY(resultChecker.waitForAllFinished(8000));
}

void tst_QSparqlTrackerDirectConcurrency::sameConnection_selectQueries_data()
{
    createTrackerTestData();
    QTest::addColumn<int>("testDataAmount");
    QTest::addColumn<int>("numQueries");
    QTest::addColumn<int>("maxThreadCount");

    QTest::newRow("10 queries, 4 Threads") <<
        TEST_DATA_AMOUNT << 10 << 4;
    QTest::newRow("100 queries, 4 Threads") <<
        TEST_DATA_AMOUNT << 100 << 4;
    QTest::newRow("10 queries, 1 Thread") <<
        TEST_DATA_AMOUNT << 10 << 1;
    QTest::newRow("100 queries, 1 Thread") <<
        TEST_DATA_AMOUNT << 100 << 1;
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
    QTest::newRow("1000 inserts, 1000 deletes") <<
        1000 << 1000;
}

void tst_QSparqlTrackerDirectConcurrency::multipleConnections_selectQueries()
{
    QFETCH(int, testDataAmount);
    QFETCH(int, numQueries);
    QFETCH(int, numConnections);
    const int dataReadyInterval = qMax(testDataAmount/100, 10);

    QList<QSparqlConnection*> connections;
    QList<ResultChecker*> resultCheckers;

    // Create the connections and start the queries
    for (int i = 0; i < numConnections; ++i) {
        QSparqlConnectionOptions options;
        options.setDataReadyInterval(dataReadyInterval);
        QSparqlConnection* conn = new QSparqlConnection("QTRACKER_DIRECT", options);
        connections << conn;
        ResultChecker* checker = new ResultChecker;
        resultCheckers << checker;
        startConcurrentQueries(*conn, *checker, numQueries, testDataAmount);
    }

    // Wait for all the queries to finish
    Q_FOREACH(ResultChecker* checker, resultCheckers) {
        QVERIFY(checker->waitForAllFinished(15000));
    }

    qDeleteAll(resultCheckers);
    qDeleteAll(connections);
}

void tst_QSparqlTrackerDirectConcurrency::multipleConnections_selectQueries_data()
{
    createTrackerTestData();
    QTest::addColumn<int>("testDataAmount");
    QTest::addColumn<int>("numQueries");
    QTest::addColumn<int>("numConnections");

    QTest::newRow("10 queries, 2 connections") <<
        TEST_DATA_AMOUNT << 10 << 2;
    QTest::newRow("100 queries, 2 connections") <<
        TEST_DATA_AMOUNT << 100 << 2;
    QTest::newRow("10 queries, 10 connections") <<
        TEST_DATA_AMOUNT << 10 << 10;
    QTest::newRow("100 queries, 10 connections") <<
        TEST_DATA_AMOUNT << 100 << 10;
}

void tst_QSparqlTrackerDirectConcurrency::multipleConnections_multipleThreads_selectQueries()
{
    QFETCH(int, testDataAmount);
    QFETCH(int, numQueries);
    QFETCH(int, numThreads);

    QList<QThread*> createdThreads;
    QList<ThreadQueryRunner*> threadQueryRunners;
    for (int i=0;i<numThreads;i++) {
        QThread *newThread = new QThread;
        createdThreads.append(newThread);

        ThreadQueryRunner *threadQueryRunner = new ThreadQueryRunner;
        threadQueryRunners.append(threadQueryRunner);
        threadQueryRunner->setParameters(numQueries, testDataAmount);
        threadQueryRunner->moveToThread(newThread);

        // Connect the threads started signal to the slot that does the work
        QObject::connect(newThread, SIGNAL(started()), threadQueryRunner, SLOT(startQueries()));
    }
    // start all the threads
    Q_FOREACH(QThread* thread, createdThreads) {
        thread->start();
    }

    waitForAllFinished(createdThreads, 15000*numThreads);
    qDeleteAll(threadQueryRunners);
    qDeleteAll(createdThreads);
}

void tst_QSparqlTrackerDirectConcurrency::multipleConnections_multipleThreads_selectQueries_data()
{
    createTrackerTestData();
    QTest::addColumn<int>("testDataAmount");
    QTest::addColumn<int>("numQueries");
    QTest::addColumn<int>("numThreads");

    QTest::newRow("10 queries, 2 Threads") <<
        TEST_DATA_AMOUNT << 10 << 2;
    QTest::newRow("100 queries, 2 Threads") <<
        TEST_DATA_AMOUNT << 100 << 2;
    QTest::newRow("10 queries, 10 Threads") <<
        TEST_DATA_AMOUNT << 10 << 10;
    QTest::newRow("100 queries, 10 Threads") <<
        TEST_DATA_AMOUNT << 100 << 10;
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

    waitForAllFinished(createdThreads, 15000*numThreads);
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
    QTest::newRow("2 Threads, 1000 inserts, 1000 deletes") <<
        2 << 1000 << 1000;
    QTest::newRow("4 Threads, 100 inserts, 100 deletes") <<
        4 << 100 << 100;
    QTest::newRow("4 Threads, 1000 inserts, 1000 deletes") <<
        4 << 1000 << 1000;
}

QTEST_MAIN( tst_QSparqlTrackerDirectConcurrency )
#include "tst_qsparql_tracker_direct_concurrency.moc"
