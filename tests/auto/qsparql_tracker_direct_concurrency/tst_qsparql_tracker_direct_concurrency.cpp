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

class tst_QSparqlTrackerDirectConcurrency : public QObject
{
    Q_OBJECT
public:
    tst_QSparqlTrackerDirectConcurrency();
    virtual ~tst_QSparqlTrackerDirectConcurrency();

private:

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void sameConnection_selectQueries();
    void sameConnection_selectQueries_data();
    //void sameConnection_updateQueries();
    void sameConnection_multipleThreads_selectQueries();
    void sameConnection_multipleThreads_selectQueries_data();
    //void sameConnection_multipleThreads_updateQueries();

    // multpleConnections tests will all be multi threaded
    //void multipleConnections_selectQueries();
    //void multipleConnections_updateQueries();

private:
    QSharedPointer<QSignalSpy> dataReadySpy;
};

namespace {

class SignalObject : public QObject
{
    Q_OBJECT
public:
    SignalObject() : position(0) {}
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
        // the results should have been fully nexted in the data ready function
        QCOMPARE(result->pos(), (int)QSparql::AfterLastRow);
        // go back through the results and validate that they are in range
        int resultCount = 0;
        while (result->previous()) {
            //we don't know the order, so just ensure the result is within range
            QVERIFY(result->value(1).toInt() <= resultRange.second && result->value(1).toInt() >= resultRange.first);
            resultCount++;
        }
        // now make sure the results counted match the size
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
}

void tst_QSparqlTrackerDirectConcurrency::cleanupTestCase()
{
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
    const QString testTag("<qsparql-tracker-direct-tests-concurrency-stress>");
    QScopedPointer<TestData> testData(createTestData(testDataAmount, "<qsparql-tracker-direct-tests>", testTag));
    QTest::qWait(2000);
    QVERIFY( testData->isOK() );

    // seed the random number generator
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());
    // store the result ranges we are going to use
    QList<QPair<int, int> > resultRanges;
    // first result will read everything
    resultRanges.append(qMakePair(1, 3000));
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
    QTest::addColumn<int>("testDataAmount");
    QTest::addColumn<int>("numQueries");
    QTest::addColumn<int>("maxThreadCount");

    QTest::newRow("3000 items, 10 queries, 4 Threads") <<
        3000 << 10 << 4;
    QTest::newRow("3000 items, 100 queries, 4 Threads") <<
        3000 << 100 << 4;
    QTest::newRow("3000 items, 10 queries, 1 Thread") <<
        3000 << 10 << 1;
    QTest::newRow("3000 items, 100 queries, 1 Thread") <<
        3000 << 100 << 1;
}

void tst_QSparqlTrackerDirectConcurrency::sameConnection_multipleThreads_selectQueries()
{
    QFETCH(int, testDataAmount);
    QFETCH(int, numQueries);
    QFETCH(int, numThreads);

    QSparqlConnection connection("QTRACKER_DIRECT");
    const QString testTag("<qsparql-tracker-direct-tests-concurrency-stress>");
    QScopedPointer<TestData> testData(createTestData(testDataAmount, "<qsparql-tracker-direct-tests>", testTag));
    QTest::qWait(2000);
    QVERIFY( testData->isOK() );

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
    QTest::addColumn<int>("testDataAmount");
    QTest::addColumn<int>("numQueries");
    QTest::addColumn<int>("numThreads");

    QTest::newRow("3000 items, 10 queries, 2 Threads") <<
        3000 << 10 << 2;
    QTest::newRow("3000 items, 100 queries, 2 Threads") <<
        3000 << 100 << 2;
    QTest::newRow("3000 items, 10 queries, 10 Threads") <<
        3000 << 10 << 10;
    QTest::newRow("3000 items, 100 queries, 10 Threads") <<
        3000 << 100 << 10;
}

QTEST_MAIN( tst_QSparqlTrackerDirectConcurrency )
#include "tst_qsparql_tracker_direct_concurrency.moc"
