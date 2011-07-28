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

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void sameConnection_selectQueries();
    void sameConnection_selectQueries_data();
    void sameConnection_updateQueries();
    void sameConnection_updateQueries_data();
    void sameConnection_multipleThreads_selectQueries();
    void sameConnection_multipleThreads_selectQueries_data();
    void sameConnection_multipleThreads_updateQueries();
    void sameConnection_multipleThreads_updateQueries_data();

    // multpleConnections tests will all be multi threaded
    void multipleConnections_selectQueries();
    void multipleConnections_selectQueries_data();
    //void multipleConnections_updateQueries();

    void singleResult_multipleThreads();
    void singleResult_multipleThreads_data();

private:
    QSharedPointer<QSignalSpy> dataReadySpy;
};

namespace {

class SignalObject : public QObject
{
    Q_OBJECT
public:
    SignalObject() : singleResult(false), position(0) {}
    SignalObject(bool singleResult) : singleResult(singleResult), position(0) {}

    bool singleResult;
    ~SignalObject()
    {
        // delete the signal mappers that were created
        foreach(QSignalMapper* map, signalMaps) {
           delete map;
        }
    }

    QSet<QSparqlResult*> pendingResults;
    int position;
    QList<QSignalMapper* > signalMaps;
    QList<QPair<int, int> > resultRanges;

    void append(QSparqlResult *r, QPair<int, int> range)
    {
        QSignalMapper *dataReadyMapper = new QSignalMapper();
        QSignalMapper *finishedMapper = new QSignalMapper();
        dataReadyMapper->setMapping(r, position);
        finishedMapper->setMapping(r, position);
        position++;

        connect(r, SIGNAL(dataReady(int)), dataReadyMapper, SLOT(map()));
        connect(dataReadyMapper, SIGNAL(mapped(int)), this, SLOT(onDataReady(int)));
        connect(r, SIGNAL(finished()), finishedMapper, SLOT(map()));
        connect(finishedMapper, SIGNAL(mapped(int)), this, SLOT(onFinished(int)));

        resultList.append(r);
        resultRanges.append(range);
        pendingResults.insert(r);

        // keep track of the signal mappers to delete later
        signalMaps.append(dataReadyMapper);
        signalMaps.append(finishedMapper);
    }
    QList<QSparqlResult*> resultList;

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

public Q_SLOTS:
    void onDataReady(int listPos)
    {
        QSparqlResult *result = resultList.at(listPos);
        while (result->next()) {
            // just do something pointless with the result
            result->value(1).toInt();
            result->value(0).toString();
        }
    }

    void onFinished(int listPos)
    {
        QPair<int, int> resultRange = resultRanges.at(listPos);
        QSparqlResult* result = resultList.at(listPos);
        int expectedResultSize = (resultRange.second - resultRange.first) + 1;
        QCOMPARE(expectedResultSize, result->size());

        //check the size of the result
        QCOMPARE(result->size(), expectedResultSize);

        if (!singleResult) {
            // the results should have been fully nexted in the data ready function
            QCOMPARE(result->pos(), (int)QSparql::AfterLastRow);
        }
        // go back through the results and validate that they are in range
        int resultCount = 0;
        while (result->previous()) {
            //we don't know the order, so just ensure the result is within range
            QVERIFY(result->value(1).toInt() <= resultRange.second && result->value(1).toInt() >= resultRange.first);
            resultCount++;
        }
        // now make sure the results counted match the size
        if (!singleResult)
            QCOMPARE(resultCount, expectedResultSize);

        pendingResults.remove(result);
    }

};

class ThreadObject : public QObject
{
    Q_OBJECT
public:
    QSparqlConnection *connection;
    SignalObject *signalObject;
    QList<QSparqlResult*> resultList;
    bool deleteConnection;
    bool deleteSignalObject;

    int numQueries;
    int testDataSize;

    ThreadObject()
    : connection(0), signalObject(0), deleteConnection(false), deleteSignalObject(false)
    {
    }

    ~ThreadObject()
    {
    }

    void cleanup()
    {
        if (deleteConnection) {
            delete connection;
        } else {
            // if we were passed a connection, delete the results
            // here to avoid leaking them
            foreach(QSparqlResult* result, resultList)
                delete result;
        }

        if (deleteSignalObject)
            delete signalObject;
    }
    void setParameters(int numQueries, int testDataSize)
    {
        this->numQueries = numQueries;
        this->testDataSize = testDataSize;
    }
    void setConnection(QSparqlConnection* connection)
    {
        this->connection = connection;
    }

    void setSignalObject(SignalObject* signalObject)
    {
        this->signalObject = signalObject;

    }

    void waitForFinished()
    {
        signalObject->waitForAllFinished(8000);
    }

public Q_SLOTS:
    void startQueries()
    {
        if (!connection) {
            this->connection = new QSparqlConnection("QTRACKER_DIRECT");
            deleteConnection = true;
        }
        if (!signalObject) {
            this->signalObject = new SignalObject();
            deleteSignalObject = true;
        }

        QTime time = QTime::currentTime();
        qsrand((uint)time.msec());
        // store the result ranges we are going to use
        QList<QPair<int, int> > resultRanges;
        // first result will read everything
        resultRanges.append(qMakePair(1, testDataSize));
        for (int i=1;i<numQueries;i++) {
            // high + 1) - low) + low
            int low = qrand() % ((testDataSize) - 1) + 1;
            int high = qrand() % ((testDataSize+1) - low) + low;
            resultRanges.append(qMakePair(low, high));
        }

        for (int i=0;i<numQueries;i++) {
            QPair<int, int> resultRange = resultRanges.at(i);
            QSparqlQuery select(QString("select ?u ?t {?u a nmm:MusicPiece;"
                                    "nmm:trackNumber ?t;"
                                    "nie:isLogicalPartOf <qsparql-tracker-direct-tests-concurrency-stress>"
                                    "FILTER ( ?t >=%1 && ?t <=%2 ) }").arg(resultRange.first).arg(resultRange.second));
            QSparqlResult *result = connection->exec(select);
            resultList.append(result);
            signalObject->append(result, resultRange);
        }

        waitForFinished();
        cleanup();
        this->thread()->quit();
    }

};

class UpdateObject : public QObject
{
    Q_OBJECT
public:
    QSparqlConnection *connection;
    QList<QSparqlResult*> resultList;
    QSet<QObject*> pendingResults;
    QSignalMapper signalMapper;
    bool deleteConnection;
    int numInserts;
    int numDeletes;
    int id;
    bool inThread;

    UpdateObject(int numInserts, int numDeletes, int id, bool inThread = false)
        : connection(0), deleteConnection(false), numInserts(numInserts), numDeletes(numDeletes), id(id),
        inThread(inThread)
    {
    }

    void cleanup()
    {
        if (numInserts-numDeletes != 0) {

            QString deleteString = "DELETE { ?u a nco:PersonContact }"
                                  " WHERE { ?u a nco:PersonContact; nie:isLogicalPartOf <qsparql-tracker-direct-concurrency-thread%1> . }";
            QSparqlQuery deleteQuery(deleteString.arg(id), QSparqlQuery::DeleteStatement);
            QSparqlResult* result = connection->syncExec(deleteQuery);
            delete result;
        }

        if (deleteConnection)
            delete connection;
    }

    void setConnection(QSparqlConnection *connection)
    {
        this->connection = connection;
    }

    void append(QSparqlResult* result)
    {
        resultList.append(result);
        pendingResults.insert(result);
        signalMapper.setMapping(result,result);

        connect(result, SIGNAL(finished()), &signalMapper, SLOT(map()));
        connect(&signalMapper, SIGNAL(mapped(QObject*)), this, SLOT(onFinished(QObject*)));
    }

    void waitForFinished()
    {
        while (!pendingResults.empty())
        {
            QTest::qWait(100);
        }
    }

public Q_SLOTS:
    void runUpdate()
    {
        // we're not testing for errors here
        QVERIFY(numDeletes <= numInserts);

        if (!connection) {
            connection = new QSparqlConnection("QTRACKER_DIRECT");
        }

        const QString insertTemplate = "insert { <addeduri00%1-%2> a nco:PersonContact; nie:isLogicalPartOf <qsparql-tracker-direct-concurrency-thread%2>;"
                                        "nco:nameGiven \"addedname00%1\"; nco:nameFamily \"addedFamily00%1\" . }";
        const QString selectTemplate = "select ?u ?ng ?nf { ?u a nco:PersonContact; nie:isLogicalPartOf <qsparql-tracker-direct-concurrency-thread%1>;"
                                        "nco:nameGiven ?ng; nco:nameFamily ?nf . }";
        const QString deleteTemplate = "delete { <addeduri00%1-%2> a nco:PersonContact }"
                                       " WHERE { <addeduri00%1-%2> a nco:PersonContact; nie:isLogicalPartOf <qsparql-tracker-direct-concurrency-thread%2> . }";

        for (int i=0;i<numInserts;i++) {
            QSparqlQuery insertQuery(insertTemplate.arg(i).arg(id), QSparqlQuery::InsertStatement);
            QSparqlResult *result = connection->exec(insertQuery);
            append(result);
        }

        waitForFinished();

        QHash<QString, QString> contactNameValues;
        // validate the results
        // we are not testing select queries so use exec/waitForFinished
        QSparqlResult *result = connection->exec(QSparqlQuery(selectTemplate.arg(id)));
        result->waitForFinished();
        QVERIFY(result->size() == numInserts);
        while (result->next()) {
            contactNameValues[result->value(0).toString()] = result->value(1).toString();
        }
        delete result;
        QVERIFY(contactNameValues.size() == numInserts);
        for(int i=0; i<numInserts; i++) {
            QCOMPARE(contactNameValues[QString("addeduri00%1-%2").arg(i).arg(id)], QString("addedname00%1").arg(i));
        }
        contactNameValues.clear();

        // now delete the results
        for (int i=0;i<numDeletes;i++) {
            QSparqlQuery deleteQuery(deleteTemplate.arg(i).arg(id), QSparqlQuery::DeleteStatement);
            QSparqlResult *result = connection->exec(deleteQuery);
            append(result);
        }
        waitForFinished();

        // verify the results now
        result = connection->exec(QSparqlQuery(selectTemplate.arg(id)));
        result->waitForFinished();
        QVERIFY(result->size() == numInserts-numDeletes);
        while (result->next()) {
            contactNameValues[result->value(0).toString()] = result->value(1).toString();
        }
        QVERIFY(result->size() == numInserts-numDeletes);
        delete result;
        // number of deletes might be less than the number of inserts, we delete from 0 -> numDeletes-1, so
        int startFrom = numInserts - numDeletes;
        if (startFrom != 0) {
            for (int i=startFrom;i<numInserts;i++) {
                QCOMPARE(contactNameValues[QString("addeduri00%1-%2").arg(i).arg(id)], QString("addedname00%1").arg(i));
            }
        }

        cleanup();
        if (inThread)
            this->thread()->quit();
    }

    void onFinished(QObject* result)
    {
        pendingResults.remove(result);
    }
};

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

    QSparqlConnectionOptions options;
    options.setDataReadyInterval(500);
    options.setMaxThreadCount(maxThreadCount);

    // seed the random number generator
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());
    // store the result ranges we are going to use
    QList<QPair<int, int> > resultRanges;
    // first result will read everything
    resultRanges.append(qMakePair(1, testDataAmount));
    for (int i=1;i<numQueries;i++) {
        // high + 1) - low) + low
        int low = qrand() % ((testDataAmount) - 1) + 1;
        int high = qrand() % ((testDataAmount+1) - low) + low;
        resultRanges.append(qMakePair(low, high));
    }

    QSparqlConnection conn("QTRACKER_DIRECT", options);
    SignalObject signalObject;

    for (int i=0;i<numQueries;i++) {
        QPair<int, int> resultRange = resultRanges.at(i);
        QSparqlQuery select(QString("select ?u ?t {?u a nmm:MusicPiece;"
                                    "nmm:trackNumber ?t;"
                                    "nie:isLogicalPartOf <qsparql-tracker-direct-tests-concurrency-stress>"
                                    "FILTER ( ?t >=%1 && ?t <=%2 ) }").arg(resultRange.first).arg(resultRange.second));
        QSparqlResult *result = conn.exec(select);
        signalObject.append(result, resultRange);
    }

    QVERIFY(signalObject.waitForAllFinished(8000));
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

    UpdateObject updateObject(numInserts, numDeletes, 1);
    updateObject.setConnection(&connection);
    updateObject.runUpdate();
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

void tst_QSparqlTrackerDirectConcurrency::sameConnection_multipleThreads_selectQueries()
{
    QFETCH(int, testDataAmount);
    QFETCH(int, numQueries);
    QFETCH(int, numThreads);

    QSparqlConnection connection("QTRACKER_DIRECT");

    QList<QThread*> createdThreads;
    QList<ThreadObject*> threadObjects;
    for (int i=0;i<numThreads;i++) {
        QThread *newThread = new QThread();
        createdThreads.append(newThread);

        ThreadObject *threadObject = new ThreadObject();
        threadObjects.append(threadObject);

        threadObject->setConnection(&connection);
        threadObject->setParameters(numQueries, testDataAmount);
        threadObject->moveToThread(newThread);

        // connec the threads started signal to the slot that does the work
        QObject::connect(newThread, SIGNAL(started()), threadObject, SLOT(startQueries()));
    }
    // start all the threads
    foreach(QThread* thread, createdThreads) {
        thread->start();
    }
    // wait for all the threads then delete
    // TODO: add timer so we don't wait forever
    foreach(QThread* thread, createdThreads) {
        while (!thread->isFinished())
            QTest::qWait(500);
        delete thread;
    }

    //cleanup
    foreach(ThreadObject *threadObject, threadObjects)
        delete threadObject;
}

void tst_QSparqlTrackerDirectConcurrency::sameConnection_multipleThreads_selectQueries_data()
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

void tst_QSparqlTrackerDirectConcurrency::sameConnection_multipleThreads_updateQueries()
{
    QFETCH(int, numThreads);
    QFETCH(int, numInserts);
    QFETCH(int, numDeletes);

    QSparqlConnection connection("QTRACKER_DIRECT");

    QList<QThread*> createdThreads;
    QList<UpdateObject*> updateObjects;
    for (int i=0;i<numThreads;i++) {
        QThread *newThread = new QThread();
        createdThreads.append(newThread);

        UpdateObject *updateObject = new UpdateObject(numInserts, numDeletes, i, true);
        updateObjects.append(updateObject);

        updateObject->setConnection(&connection);
        updateObject->moveToThread(newThread);

        // connec the threads started signal to the slot that does the work
        QObject::connect(newThread, SIGNAL(started()), updateObject, SLOT(runUpdate()));
    }
    // start all the threads
    foreach(QThread* thread, createdThreads) {
        thread->start();
    }
    // wait for all the threads then delete
    // TODO: add timer so we don't wait forever
    foreach(QThread* thread, createdThreads) {
        while (!thread->isFinished())
            QTest::qWait(500);
        delete thread;
    }

    //cleanup
    foreach(UpdateObject *updateObject, updateObjects)
        delete updateObject;
}

void tst_QSparqlTrackerDirectConcurrency::sameConnection_multipleThreads_updateQueries_data()
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

void tst_QSparqlTrackerDirectConcurrency::multipleConnections_selectQueries()
{
    QFETCH(int, testDataAmount);
    QFETCH(int, numQueries);
    QFETCH(int, numThreads);

    QSparqlConnection connection("QTRACKER_DIRECT");

    QList<QThread*> createdThreads;
    QList<ThreadObject*> threadObjects;
    for (int i=0;i<numThreads;i++) {
        QThread *newThread = new QThread();
        createdThreads.append(newThread);

        ThreadObject *threadObject = new ThreadObject();
        threadObjects.append(threadObject);
        threadObject->setParameters(numQueries, testDataAmount);
        threadObject->moveToThread(newThread);

        // connec the threads started signal to the slot that does the work
        QObject::connect(newThread, SIGNAL(started()), threadObject, SLOT(startQueries()));
    }
    // start all the threads
    foreach(QThread* thread, createdThreads) {
        thread->start();
    }
    // wait for all the threads then delete
    // TODO: add timer so we don't wait forever
    foreach(QThread* thread, createdThreads) {
        while (!thread->isFinished())
            QTest::qWait(500);
        delete thread;
    }
    //cleanup
    foreach(ThreadObject *threadObject, threadObjects)
        delete threadObject;
}

void tst_QSparqlTrackerDirectConcurrency::multipleConnections_selectQueries_data()
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

void tst_QSparqlTrackerDirectConcurrency::singleResult_multipleThreads()
{
    QFETCH(int, numberOfThreads);
    QFETCH(int, testDataAmount);

    QSparqlConnectionOptions options;
    options.setDataReadyInterval(1000);
    QSparqlConnection conn("QTRACKER_DIRECT", options);
    QSparqlQuery select(QString("select ?u ?t {?u a nmm:MusicPiece;"
                                    "nmm:trackNumber ?t;"
                                    "nie:isLogicalPartOf <qsparql-tracker-direct-tests-concurrency-stress> }"));
    // read everything
    QPair<int, int> resultRange = qMakePair(1, testDataAmount);

    QList<QThread*> threads;
    QList<SignalObject*> signalObjects;

    for (int i=0;i<numberOfThreads;i++)
    {
        QThread *newThread = new QThread();
        // we don't want to check the position of the results in
        // is finished for this test, since multiple threads will
        // access the result nexting/previous, so mark this as a
        // "single result" test
        // TODO: write a test that checks that behaviour
        SignalObject *signalObject = new SignalObject(true);
        threads.append(newThread);
        signalObjects.append(signalObject);

        signalObject->moveToThread(newThread);
        newThread->start();
    }

    QSparqlResult *result = conn.exec(select);

    Q_FOREACH(SignalObject *signalObject, signalObjects)
        signalObject->append(result, resultRange);

    QTest::qWait(5000);

    //now wait for the first signal object to finish
    //and delete them
    Q_FOREACH(SignalObject *signalObject, signalObjects) {
        signalObject->waitForAllFinished(3000);
        delete signalObject;
    }

    //quit the threads
    Q_FOREACH(QThread *thread, threads) {
        thread->quit();
        thread->wait();
        delete thread;
    }
}

void tst_QSparqlTrackerDirectConcurrency::singleResult_multipleThreads_data()
{
    createTrackerTestData();
    QTest::addColumn<int>("testDataAmount");
    QTest::addColumn<int>("numberOfThreads");

    QTest::newRow("Single Result, 1 Thread") <<
        TEST_DATA_AMOUNT << 1;
    QTest::newRow("Single Result, 2 Threads") <<
        TEST_DATA_AMOUNT << 2;
    QTest::newRow("Single Result, 4 Threads") <<
        TEST_DATA_AMOUNT << 4;
    QTest::newRow("Single Result, 8 Threads") <<
        TEST_DATA_AMOUNT << 8;
}

QTEST_MAIN( tst_QSparqlTrackerDirectConcurrency )
#include "tst_qsparql_tracker_direct_concurrency.moc"
