/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the test suite of the Qt Toolkit.
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
** Nokia at qt-info@nokia.com.
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

// This test code is based on the qdbusthreading tests in qt

#include "../testhelpers.h"

#include <QtTest>
#include <QtCore/QVarLengthArray>
#include <QtCore/QThread>
#include <QtCore/QObject>
#include <QtCore/QSemaphore>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <QtCore/QMap>

#include <QtSparql/QtSparql>

#define TEST_PORT 1111

class Thread : public QThread
{
    Q_OBJECT
    static int counter;
public:
    Thread(bool automatic = true);
    void run();

    using QThread::exec;

public Q_SLOTS:
    void queryFinished();
    void resultsReturned(int count);
};
int Thread::counter;

class tst_QSparqlThreading : public QObject
{
    Q_OBJECT
    static tst_QSparqlThreading *_self;
    QAtomicInt threadJoinCount;
    QSemaphore threadJoin;
public:
    QSemaphore sem1, sem2;
    volatile bool success;
    QEventLoop *loop;

    QSparqlConnection *conn1;
    QSparqlResult *r1;

    QSparqlConnection *conn2;
    QSparqlResult *r2;

    tst_QSparqlThreading();
    static inline tst_QSparqlThreading *self() { return _self; }

    void joinThreads();
    bool waitForSignal(QObject *obj, const char *signal, int delay = 1);

public Q_SLOTS:
    void cleanup();
    void threadStarted() { threadJoinCount.ref(); }
    void threadFinished() { threadJoin.release(); }

    void concurrentEndpointQueries_thread();
    void concurrentVirtuosoQueries_thread();
    void concurrentTrackerQueries_thread();
    void concurrentTrackerDirectQueries_thread();
    void concurrentTrackerDirectInserts_thread();
    void subThreadTrackerDirectQuery_thread();

    void queryFinished();
    void resultsReturned(int count);

private Q_SLOTS:
    void initTestCase();
    void concurrentEndpointQueries();
    void concurrentVirtuosoQueries();
    void concurrentTrackerQueries();
    void concurrentTrackerDirectQueries();
    void concurrentTrackerDirectInserts();
    void subThreadTrackerDirectQuery();
};
tst_QSparqlThreading *tst_QSparqlThreading::_self;


Thread::Thread(bool automatic)
{
    setObjectName(QString::fromLatin1("Aux thread %1").arg(++counter));
    connect(this, SIGNAL(started()), tst_QSparqlThreading::self(), SLOT(threadStarted()));
    connect(this, SIGNAL(finished()), tst_QSparqlThreading::self(), SLOT(threadFinished()),
            Qt::DirectConnection);
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()), Qt::DirectConnection);
    if (automatic)
        start();
}

void Thread::run()
{
    QVarLengthArray<char, 56> name;
    name.append(QTest::currentTestFunction(), qstrlen(QTest::currentTestFunction()));
    name.append("_thread", sizeof "_thread");
    QMetaObject::invokeMethod(tst_QSparqlThreading::self(), name.constData(), Qt::DirectConnection);
}

void Thread::queryFinished()
{
    //qDebug() << "Thread::queryFinished()";
}

void Thread::resultsReturned(int /*totalCount*/)
{
    //qDebug() << "Thread::resultsReturned(" << totalCount << ")";
}

void tst_QSparqlThreading::queryFinished()
{
    //qDebug() << "tst_QSparqlThreading::queryFinished()";
}

void tst_QSparqlThreading::resultsReturned(int /*totalCount*/)
{
    //qDebug() << "tst_QSparqlThreading::resultsReturned(" << totalCount << ")";
}

tst_QSparqlThreading::tst_QSparqlThreading()
    : loop(0), conn1(0), r1(0), conn2(0), r2(0)
{
    _self = this;
    QCoreApplication::instance()->thread()->setObjectName("Main thread");
}

void tst_QSparqlThreading::joinThreads()
{
    threadJoin.acquire(threadJoinCount);
    threadJoinCount = 0;
}

bool tst_QSparqlThreading::waitForSignal(QObject *obj, const char *signal, int delay)
{
    QObject::connect(obj, signal, &QTestEventLoop::instance(), SLOT(exitLoop()));
    QPointer<QObject> safe = obj;

    QTestEventLoop::instance().enterLoop(delay);
    if (!safe.isNull())
        QObject::disconnect(safe, signal, &QTestEventLoop::instance(), SLOT(exitLoop()));
    return QTestEventLoop::instance().timeout();
}

void tst_QSparqlThreading::cleanup()
{
    joinThreads();

    if (sem1.available())
        sem1.acquire(sem1.available());
    if (sem2.available())
        sem2.acquire(sem2.available());

    delete loop;
    loop = 0;

    QTest::qWait(500);
    delete conn1;
    conn1 = 0;
    delete conn2;
    conn2 = 0;
}

void tst_QSparqlThreading::initTestCase()
{
    // For running the test without installing the plugins. Should work in
    // normal and vpath builds.
    QCoreApplication::addLibraryPath("../../../plugins");
}

void tst_QSparqlThreading::concurrentEndpointQueries_thread()
{
    sem1.acquire();
    QSparqlConnectionOptions options;
    options.setHostName("localhost");
    options.setPort(8890);
    conn2 = new QSparqlConnection("QSPARQL_ENDPOINT", options);

    QSparqlQuery q("SELECT ?s ?p ?o WHERE { ?s ?p ?o . }");
    r2 = conn2->exec(q);
    connect(r2, SIGNAL(finished()), QThread::currentThread(), SLOT(queryFinished()));
    connect(r2, SIGNAL(dataReady(int)), QThread::currentThread(), SLOT(resultsReturned(int)));
    sem2.release();
    r2->waitForFinished();
}

void tst_QSparqlThreading::concurrentEndpointQueries()
{
    QPointer<Thread> th = new Thread;

    sem1.release();
    QSparqlConnectionOptions options;
    options.setHostName("localhost");
    options.setPort(8890);
    conn1 = new QSparqlConnection("QSPARQL_ENDPOINT", options);

    QSparqlQuery q("SELECT ?s ?p ?o WHERE { ?s ?p ?o . }");
    r1 = conn1->exec(q);
    connect(r1, SIGNAL(finished()), SLOT(queryFinished()));
    connect(r1, SIGNAL(dataReady(int)), SLOT(resultsReturned(int)));
    sem2.acquire();

    r1->waitForFinished();

    if (!th.isNull()) {
        waitForSignal(th, SIGNAL(finished()));
    }

    CHECK_ERROR(r1);
    CHECK_ERROR(r2);
    QCOMPARE(r1->size(), r2->size());
}

void tst_QSparqlThreading::concurrentVirtuosoQueries_thread()
{
    sem1.acquire();
    QSparqlConnectionOptions options;
    options.setDatabaseName("DRIVER=/usr/lib/odbc/virtodbc_r.so");
    options.setPort(TEST_PORT);
    conn2 = new QSparqlConnection("QVIRTUOSO", options);

    QSparqlQuery q("SELECT ?s ?p ?o WHERE { ?s ?p ?o . }");
    r2 = conn2->exec(q);
    connect(r2, SIGNAL(finished()), QThread::currentThread(), SLOT(queryFinished()));
    connect(r2, SIGNAL(dataReady(int)), QThread::currentThread(), SLOT(resultsReturned(int)));
    sem2.release();
    r2->waitForFinished();
}

void tst_QSparqlThreading::concurrentVirtuosoQueries()
{
    QPointer<Thread> th = new Thread;

    sem1.release();
    QSparqlConnectionOptions options;
    options.setDatabaseName("DRIVER=/usr/lib/odbc/virtodbc_r.so");
    options.setPort(TEST_PORT);
    conn1 = new QSparqlConnection("QVIRTUOSO", options);

    QSparqlQuery q("SELECT ?s ?p ?o WHERE { ?s ?p ?o . }");
    r1 = conn1->exec(q);
    connect(r1, SIGNAL(finished()), SLOT(queryFinished()));
    connect(r1, SIGNAL(dataReady(int)), SLOT(resultsReturned(int)));
    sem2.acquire();

    r1->waitForFinished();

    if (!th.isNull()) {
        waitForSignal(th, SIGNAL(finished()));
    }

    CHECK_ERROR(r1);
    CHECK_ERROR(r2);
    QCOMPARE(r1->size(), r2->size());
}


void tst_QSparqlThreading::concurrentTrackerQueries_thread()
{
    sem1.acquire();
    conn2 = new QSparqlConnection("QTRACKER");

    QSparqlQuery q("select ?u {?u a rdfs:Resource .}");
    r2 = conn2->exec(q);
    connect(r2, SIGNAL(finished()), QThread::currentThread(), SLOT(queryFinished()));
    connect(r2, SIGNAL(dataReady(int)), QThread::currentThread(), SLOT(resultsReturned(int)));
    sem2.release();
    r2->waitForFinished();
}

void tst_QSparqlThreading::concurrentTrackerQueries()
{
    QPointer<Thread> th = new Thread;

    sem1.release();
    conn1 = new QSparqlConnection("QTRACKER");

    QSparqlQuery q("select ?u {?u a rdfs:Resource .}");
    r1 = conn1->exec(q);
    connect(r1, SIGNAL(finished()), SLOT(queryFinished()));
    connect(r1, SIGNAL(dataReady(int)), SLOT(resultsReturned(int)));
    sem2.acquire();

    r1->waitForFinished();

    if (!th.isNull()) {
        waitForSignal(th, SIGNAL(finished()));
    }

    CHECK_ERROR(r1);
    CHECK_ERROR(r2);
    QCOMPARE(r1->size(), r2->size());
}

void tst_QSparqlThreading::concurrentTrackerDirectQueries_thread()
{
    sem1.acquire();
    conn2 = new QSparqlConnection("QTRACKER_DIRECT");

    QSparqlQuery q("select ?u {?u a rdfs:Resource .}");
    r2 = conn2->exec(q);
    connect(r2, SIGNAL(finished()), QThread::currentThread(), SLOT(queryFinished()));
    connect(r2, SIGNAL(dataReady(int)), QThread::currentThread(), SLOT(resultsReturned(int)));
    sem2.release();
    r2->waitForFinished();
}

void tst_QSparqlThreading::concurrentTrackerDirectQueries()
{
    QPointer<Thread> th = new Thread;

    sem1.release();
    conn1 = new QSparqlConnection("QTRACKER_DIRECT");

    QSparqlQuery q("select ?u {?u a rdfs:Resource .}");
    r1 = conn1->exec(q);
    connect(r1, SIGNAL(finished()), SLOT(queryFinished()));
    connect(r1, SIGNAL(dataReady(int)), SLOT(resultsReturned(int)));
    sem2.acquire();

    r1->waitForFinished();

    if (!th.isNull()) {
        waitForSignal(th, SIGNAL(finished()));
    }

    CHECK_ERROR(r1);
    CHECK_ERROR(r2);
    QCOMPARE(r1->size(), r2->size());
}

void tst_QSparqlThreading::concurrentTrackerDirectInserts_thread()
{
    sem1.acquire();
    conn2 = new QSparqlConnection("QTRACKER_DIRECT");

    QSparqlQuery q("insert { ?:addeduri a nco:PersonContact; "
                     "nie:isLogicalPartOf <qsparql-threading-tests> ;"
                     "nco:nameGiven \"addedname007\" .}",
                     QSparqlQuery::InsertStatement);
    q.bindValue(conn2->createUrn("addeduri"));

    r2 = conn2->exec(q);
    connect(r2, SIGNAL(finished()), QThread::currentThread(), SLOT(queryFinished()));
    connect(r2, SIGNAL(dataReady(int)), QThread::currentThread(), SLOT(resultsReturned(int)));
    sem2.release();
    r2->waitForFinished();

    CHECK_ERROR(r2);

    delete r2;

    // Verify that the insertion succeeded
    QSparqlQuery q2("select ?addeduri {?addeduri a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-threading-tests> ;"
                   "nco:nameGiven \"addedname007\" .}");
    r2 = conn2->exec(q2);
    QVERIFY(r2 != 0);
    r2->waitForFinished();
    QCOMPARE(r2->size(), 1);
    if (r2->next()) {
        // We can only compare the first 9 chars because the rest is a new uuid string
        QCOMPARE(r2->value(0).toString().mid(0, 9), QString("urn:uuid:"));
    }

    // Delete the uri
    QSparqlQuery del("delete { ?:addeduri a rdfs:Resource. }",
                     QSparqlQuery::DeleteStatement);

    del.bindValue(r2->binding(0));
    delete r2;
    r2 = conn2->exec(del);
    qDebug() << "r2 delete query:" << r2->query();
    QVERIFY(r2 != 0);
    r2->waitForFinished(); // this test is synchronous only
    CHECK_ERROR(r2);
    delete r2;

    // Verify that it got deleted
    r2 = conn2->exec(q2);
    QVERIFY(r2 != 0);
    r2->waitForFinished();
    QCOMPARE(r2->size(), 0);
    delete r2;

}

void tst_QSparqlThreading::concurrentTrackerDirectInserts()
{
    QPointer<Thread> th = new Thread;

    sem1.release();
    conn1 = new QSparqlConnection("QTRACKER_DIRECT");

    QSparqlQuery q1("insert { ?:addeduri a nco:PersonContact; "
                     "nie:isLogicalPartOf <qsparql-threading-tests> ;"
                     "nco:nameGiven \"addedname008\" .}",
                     QSparqlQuery::InsertStatement);
    q1.bindValue(conn1->createUrn("addeduri"));

    r1 = conn1->exec(q1);
    connect(r1, SIGNAL(finished()), SLOT(queryFinished()));
    connect(r1, SIGNAL(dataReady(int)), SLOT(resultsReturned(int)));
    sem2.acquire();

    r1->waitForFinished();

    CHECK_ERROR(r1);

    delete r1;

    // Verify that the insertion succeeded
    QSparqlQuery q2("select ?addeduri {?addeduri a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-threading-tests> ;"
                   "nco:nameGiven \"addedname008\" .}");
    r1 = conn1->exec(q2);
    QVERIFY(r1 != 0);
    r1->waitForFinished();
    QCOMPARE(r1->size(), 1);
    if (r1->next()) {
        // We can only compare the first 9 chars because the rest is a new uuid string
        QCOMPARE(r1->value(0).toString().mid(0, 9), QString("urn:uuid:"));
    }

    // Delete the uri
    QSparqlQuery del("delete { ?:addeduri a rdfs:Resource. }",
                     QSparqlQuery::DeleteStatement);

    del.bindValue(r1->binding(0));
    delete r1;
    r1 = conn1->exec(del);
    qDebug() << "r1 delete query:" << r1->query();
    QVERIFY(r1 != 0);
    r1->waitForFinished(); // this test is synchronous only
    CHECK_ERROR(r1);
    delete r1;

    // Verify that it got deleted
    r1 = conn1->exec(q2);
    QVERIFY(r1 != 0);
    r1->waitForFinished();
    QCOMPARE(r1->size(), 0);
    delete r1;

    if (!th.isNull()) {
        waitForSignal(th, SIGNAL(finished()));
    }
}

void tst_QSparqlThreading::subThreadTrackerDirectQuery_thread()
{
    sem1.acquire();
    conn2 = new QSparqlConnection("QTRACKER_DIRECT");

    QSparqlQuery q("select ?u {?u a rdfs:Resource .}");
    r2 = conn2->exec(q);
    connect(r2, SIGNAL(finished()), QThread::currentThread(), SLOT(queryFinished()));
    connect(r2, SIGNAL(dataReady(int)), QThread::currentThread(), SLOT(resultsReturned(int)));
    sem2.release();
    r2->waitForFinished();
}

void tst_QSparqlThreading::subThreadTrackerDirectQuery()
{
    QPointer<Thread> th = new Thread;

    sem1.release();
    sem2.acquire();

    if (!th.isNull()) {
        waitForSignal(th, SIGNAL(finished()));
    }

    CHECK_ERROR(r2);
}

QTEST_MAIN(tst_QSparqlThreading)
#include "tst_qsparql_threading.moc"
