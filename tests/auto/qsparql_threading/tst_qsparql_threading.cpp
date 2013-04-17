/****************************************************************************
**
** Copyright (C) 2010-2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the test suite of the Qt Toolkit.
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

#include <QtSparql>

#include "../messagerecorder.h"

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

    void queryFinished();
    void resultsReturned(int count);

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void concurrentEndpointQueries();
    void concurrentVirtuosoQueries();
    void concurrentTrackerQueries();
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    threadJoin.acquire(threadJoinCount.load());
    threadJoinCount.store(0);
#else
    threadJoin.acquire(threadJoinCount);
    threadJoinCount = 0;
#endif
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
    QSparqlConnection conn("QTRACKER");
    QSparqlQuery q("DELETE { ?u a rdfs:Resource . } "
                   "WHERE {?u nie:isLogicalPartOf <qsparql-threading-tests> .}"
                   "DELETE { <qsparql-threading-tests> a rdfs:Resource . }",
                   QSparqlQuery::DeleteStatement);
    QSparqlResult* r = conn.exec(q);
    CHECK_QSPARQL_RESULT(r);
    r->waitForFinished();
    CHECK_QSPARQL_RESULT(r);
    delete r;
}

void tst_QSparqlThreading::cleanupTestCase()
{
    QSparqlConnection conn("QTRACKER");
    QSparqlQuery q("DELETE { ?u a rdfs:Resource . }"
                   "WHERE  { ?u nie:isLogicalPartOf <qsparql-threading-tests> . }"
                   "DELETE { <qsparql-threading-tests> a rdfs:Resource . }",
                    QSparqlQuery::DeleteStatement);
    QSparqlResult* r = conn.exec(q);
    CHECK_QSPARQL_RESULT(r);
    r->waitForFinished();
    CHECK_QSPARQL_RESULT(r);
    delete r;
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

    CHECK_QSPARQL_RESULT(r1);
    CHECK_QSPARQL_RESULT(r2);
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

    CHECK_QSPARQL_RESULT(r1);
    CHECK_QSPARQL_RESULT(r2);
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

    CHECK_QSPARQL_RESULT(r1);
    CHECK_QSPARQL_RESULT(r2);
    QCOMPARE(r1->size(), r2->size());
}

QTEST_MAIN(tst_QSparqlThreading)
#include "tst_qsparql_threading.moc"
