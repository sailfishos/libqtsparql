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

#include <QtTest>
#include <QtCore/QVarLengthArray>
#include <QtCore/QThread>
#include <QtCore/QObject>
#include <QtCore/QSemaphore>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <QtCore/QMap>

#include <QtSparql/QtSparql>

class Thread : public QThread
{
    Q_OBJECT
    static int counter;
public:
    Thread(bool automatic = true);
    void run();

    using QThread::exec;
};
int Thread::counter;

class tst_QSparqlEndpointThreading : public QObject
{
    Q_OBJECT
    static tst_QSparqlEndpointThreading *_self;
    QAtomicInt threadJoinCount;
    QSemaphore threadJoin;
public:
    QSemaphore sem1, sem2;
    volatile bool success;
    QEventLoop *loop;
    const char *functionSpy;
    QThread *threadSpy;
    int signalSpy;

    tst_QSparqlEndpointThreading();
    static inline tst_QSparqlEndpointThreading *self() { return _self; }

    void joinThreads();
    bool waitForSignal(QObject *obj, const char *signal, int delay = 1);

public Q_SLOTS:
    void cleanup();
    void signalSpySlot() { ++signalSpy; }
    void threadStarted() { threadJoinCount.ref(); }
    void threadFinished() { threadJoin.release(); }

    void concurrentCreation_thread();

private Q_SLOTS:
    void initTestCase();
    void concurrentCreation();
};
tst_QSparqlEndpointThreading *tst_QSparqlEndpointThreading::_self;



class Object : public QObject
{
    Q_OBJECT
public:
    Object()
    {
    }

    ~Object()
    {
        QMetaObject::invokeMethod(QThread::currentThread(), "quit", Qt::QueuedConnection);
    }

public Q_SLOTS:
    void method()
    {
        tst_QSparqlEndpointThreading::self()->functionSpy = Q_FUNC_INFO;
        tst_QSparqlEndpointThreading::self()->threadSpy = QThread::currentThread();
        emit signal();
        deleteLater();
    }

Q_SIGNALS:
    void signal();
};

Thread::Thread(bool automatic)
{
    setObjectName(QString::fromLatin1("Aux thread %1").arg(++counter));
    connect(this, SIGNAL(started()), tst_QSparqlEndpointThreading::self(), SLOT(threadStarted()));
    connect(this, SIGNAL(finished()), tst_QSparqlEndpointThreading::self(), SLOT(threadFinished()),
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
    QMetaObject::invokeMethod(tst_QSparqlEndpointThreading::self(), name.constData(), Qt::DirectConnection);
}

tst_QSparqlEndpointThreading::tst_QSparqlEndpointThreading()
    : loop(0), functionSpy(0), threadSpy(0)
{
    _self = this;
    QCoreApplication::instance()->thread()->setObjectName("Main thread");
}

void tst_QSparqlEndpointThreading::joinThreads()
{
    threadJoin.acquire(threadJoinCount);
    threadJoinCount = 0;
}

bool tst_QSparqlEndpointThreading::waitForSignal(QObject *obj, const char *signal, int delay)
{
    QObject::connect(obj, signal, &QTestEventLoop::instance(), SLOT(exitLoop()));
    QPointer<QObject> safe = obj;

    QTestEventLoop::instance().enterLoop(delay);
    if (!safe.isNull())
        QObject::disconnect(safe, signal, &QTestEventLoop::instance(), SLOT(exitLoop()));
    return QTestEventLoop::instance().timeout();
}

void tst_QSparqlEndpointThreading::cleanup()
{
    joinThreads();

    if (sem1.available())
        sem1.acquire(sem1.available());
    if (sem2.available())
        sem2.acquire(sem2.available());

    delete loop;
    loop = 0;

    QTest::qWait(500);
}

void tst_QSparqlEndpointThreading::initTestCase()
{
    // For running the test without installing the plugins. Should work in
    // normal and vpath builds.
    QCoreApplication::addLibraryPath("../../../plugins");
}

void tst_QSparqlEndpointThreading::concurrentCreation_thread()
{
    sem1.acquire();
    QSparqlConnectionOptions options;
    options.setHostName("localhost");
    options.setPort(8890);
    QSparqlConnection conn("QSPARQL_ENDPOINT", options);

    QSparqlQuery q("SELECT ?s ?p ?o WHERE { ?s ?p ?o . }");
    QSparqlResult* r = conn.exec(q);
    sem2.release();
}

void tst_QSparqlEndpointThreading::concurrentCreation()
{
    Thread *th = new Thread;

    {
        sem1.release();
        QSparqlConnectionOptions options;
        options.setHostName("localhost");
        options.setPort(8890);
        QSparqlConnection conn("QSPARQL_ENDPOINT", options);

        QSparqlQuery q("SELECT ?s ?p ?o WHERE { ?s ?p ?o . }");
        QSparqlResult* r = conn.exec(q);
        sem2.acquire();
    }

    waitForSignal(th, SIGNAL(finished()));
}

QTEST_MAIN(tst_QSparqlEndpointThreading)
#include "tst_qsparql_endpoint_threading.moc"
