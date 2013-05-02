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
#include "../utils/testdata.h"

#include <QtTest/QtTest>
#include <QtSparql>

class tst_QSparqlTrackerDirect : public TrackerDirectCommon
{
    Q_OBJECT

public:
    tst_QSparqlTrackerDirect();
    virtual ~tst_QSparqlTrackerDirect();

private:
    QSparqlResult* execQuery(QSparqlConnection &conn, const QSparqlQuery &q);
    void waitForQueryFinished(QSparqlResult* r);
    bool checkResultSize(QSparqlResult* r, int s);
    bool ensureQueryExecuting(QSparqlResult* r);

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void qsparqlresultrow();
    void insert_new_urn();

    void delete_unfinished_result();
    void delete_partially_iterated_result();
    void delete_nearly_finished_result();
    void cancel_insert_result();
    void cancel_insert_result_data();

    void concurrent_queries();
    void concurrent_queries_2();

    void insert_with_dbus_read_with_direct();

    void open_connection_twice();

    void result_immediately_finished();
    void result_immediately_finished2();

    void delete_connection_immediately();
    void delete_connection_before_a_wait();

    void create_2_connections();

    void unsupported_statement_type();

    void async_conn_opening();
    void async_conn_opening_data();
    void async_conn_opening_with_2_connections();
    void async_conn_opening_with_2_connections_data();
    void async_conn_opening_for_update();
    void async_conn_opening_for_update_data();

    void delete_later_with_select_result();
    void delete_later_with_update_result();
    void delete_result_while_update_query_is_executing();

    void query_with_data_ready_set();
    void query_with_data_ready_set_data();

    void destroy_connection_partially_iterated_results();

    void waitForFinished_after_dataReady();

    void test_threadpool_priority_select_results();
    void test_threadpool_priority_update_results();

private:
    QSharedPointer<QSignalSpy> dataReadySpy;
};

namespace {
class FinishedSignalReceiver : public QObject
{
    Q_OBJECT
    QList<QSparqlResult*> allResults;
    QSet<QObject*> pendingResults;
    QSignalMapper signalMapper;

public:

    QList<QSparqlResult*> resultOrder;

    FinishedSignalReceiver()
    {
        connect(&signalMapper, SIGNAL(mapped(QObject*)), this, SLOT(onFinished(QObject*)));
    }

    ~FinishedSignalReceiver()
    {
        qDeleteAll(allResults);
    }

    const QList<QSparqlResult*>& results() const
    {
        return allResults;
    }

    void append(QSparqlResult* r)
    {
        allResults.append(r);
        if (!r->isFinished()) {
            pendingResults.insert(r);
            connectFinishedSignal(r);
        }
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

private:
    void connectFinishedSignal(QSparqlResult* r)
    {
        // Use QSignalMapper to be able to figure out which result was
        // finished in onFinished().
        // Note that using sender() would not work as it returns null when
        // signal is emitted from another thread.
        signalMapper.setMapping(r, r);
        connect(r, SIGNAL(finished()), &signalMapper, SLOT(map()));
    }

private slots:
    void onFinished(QObject* r)
    {
        pendingResults.remove(r);
        resultOrder.append((QSparqlResult*)r);
    }
};

}  // namespace

tst_QSparqlTrackerDirect::tst_QSparqlTrackerDirect()
{
}

tst_QSparqlTrackerDirect::~tst_QSparqlTrackerDirect()
{
}

QSparqlResult* tst_QSparqlTrackerDirect::execQuery(QSparqlConnection &conn, const QSparqlQuery &q){
    QSparqlResult* r = conn.exec(q);
    return r;
}

void tst_QSparqlTrackerDirect::waitForQueryFinished(QSparqlResult* r)
{
    r->waitForFinished();
}

bool tst_QSparqlTrackerDirect::checkResultSize(QSparqlResult* r, int s){
    return (r->size() == s);
}

bool tst_QSparqlTrackerDirect::ensureQueryExecuting(QSparqlResult* r)
{
    dataReadySpy = QSharedPointer<QSignalSpy>(new QSignalSpy(r, SIGNAL(dataReady(int))));
    while (dataReadySpy->count() < 2)
        QTest::qWait(1);
    return (!r->isFinished());
}

void tst_QSparqlTrackerDirect::initTestCase()
{
    // For running the test without installing the plugins. Should work in
    // normal and vpath builds.
    QCoreApplication::addLibraryPath("../../../plugins");
    installMsgHandler();

    // clean any remainings
    QVERIFY(cleanData());

    QVERIFY(setupData());
}

void tst_QSparqlTrackerDirect::cleanupTestCase()
{
    QVERIFY(cleanData());
}

void tst_QSparqlTrackerDirect::init()
{
    setMsgLogLevel(QtDebugMsg);
}

void tst_QSparqlTrackerDirect::cleanup()
{
    dataReadySpy.clear();
}

void tst_QSparqlTrackerDirect::qsparqlresultrow()
{
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");
    QSparqlResult* r = conn.exec(q);
    CHECK_QSPARQL_RESULT(r);
    r->waitForFinished(); // this test is synchronous only
    CHECK_QSPARQL_RESULT(r);
    QCOMPARE(r->size(), 3);
    QVERIFY(r->next());
    QSparqlResultRow row = r->current();

    QCOMPARE(row.count(), 2);
    QCOMPARE(row.isEmpty(), false);

    QCOMPARE(row.indexOf("foo"), -1);
    QCOMPARE(row.indexOf("NG"), -1);
    QCOMPARE(row.indexOf(""), -1);
    QCOMPARE(row.indexOf("ng"), 1);

    QCOMPARE(row.contains("foo"), false);
    QCOMPARE(row.contains("NG"), false);
    QCOMPARE(row.contains(""), false);
    QCOMPARE(row.contains("ng"), true);

    QCOMPARE(row.variableName(0), QString::fromLatin1("u"));
    QCOMPARE(row.variableName(1), QString::fromLatin1("ng"));
    QCOMPARE(row.variableName(2), QString());
    QCOMPARE(row.variableName(-1), QString());

    row.clear();
    QCOMPARE(row.isEmpty(), true);
    QCOMPARE(row.contains("ng"), false);
    QCOMPARE(row.variableName(1), QString());
    QCOMPARE(row.indexOf("ng"), -1);

    delete r;
}

void tst_QSparqlTrackerDirect::insert_new_urn()
{
    // This test will leave unclean test data in tracker if it crashes.
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlQuery add("insert { ?:addeduri a nco:PersonContact; "
                     "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                     "nco:nameGiven \"addedname006\" .}",
                     QSparqlQuery::InsertStatement);
    const QSparqlBinding addeduri(conn.createUrn("addeduri"));
    add.bindValue(addeduri);
    QSparqlResult* r = conn.exec(add);
    CHECK_QSPARQL_RESULT(r);
    r->waitForFinished(); // this test is synchronous only
    CHECK_QSPARQL_RESULT(r);
    delete r;

    // Verify that the insertion succeeded
    QSparqlQuery q("select ?addeduri ?ng {?addeduri a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");
    QHash<QString, QSparqlBinding> contactNames;
    r = conn.exec(q);
    QVERIFY(r != 0);
    r->waitForFinished();
    QCOMPARE(r->size(), 4);
    while (r->next()) {
        contactNames[r->binding(1).value().toString()] = r->binding(0);
    }
    QCOMPARE(contactNames.size(), 4);
    QCOMPARE(contactNames["addedname006"].value().toString(),
             addeduri.value().toString());
    delete r;

    // Delete the uri
    QSparqlQuery del("delete { ?:addeduri a rdfs:Resource. }",
                     QSparqlQuery::DeleteStatement);

    del.bindValue(addeduri);
    r = conn.exec(del);
    CHECK_QSPARQL_RESULT(r);
    r->waitForFinished(); // this test is synchronous only
    CHECK_QSPARQL_RESULT(r);
    delete r;

    // Verify that it got deleted
    contactNames.clear();
    r = conn.exec(q);
    CHECK_QSPARQL_RESULT(r);
    r->waitForFinished();
    CHECK_QSPARQL_RESULT(r);
    QCOMPARE(r->size(), 3);
    while (r->next()) {
        contactNames[r->binding(1).value().toString()] = r->binding(0);
    }
    QCOMPARE(contactNames.size(), 3);
    delete r;
}

void tst_QSparqlTrackerDirect::delete_unfinished_result()
{
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");
    QSparqlResult* r = conn.exec(q);
    CHECK_QSPARQL_RESULT(r);
    delete r;
    // Spin the event loop so that the async callback is called.
    QTest::qWait(1000);
}

void tst_QSparqlTrackerDirect::delete_partially_iterated_result()
{
    const int testDataAmount = 3000;
    const QString testCaseTag("<qsparql-tracker-direct-tests-delete_partially_iterated_result>");
    QScopedPointer<TestData> testData(
            TestData::createTrackerTestData(testDataAmount, "<qsparql-tracker-direct-tests>", testCaseTag));
    QTest::qWait(1000);
    QVERIFY( testData->isOK() );
    QSparqlConnectionOptions opts;
    opts.setDataReadyInterval(1);
    QSparqlConnection conn("QTRACKER_DIRECT", opts);

    const int maxRounds = 20;
    int succesfulRounds = 0;
    for(int round=0; round < maxRounds; ++round) {
        QSparqlResult* r = conn.exec(testData->selectQuery());
        CHECK_QSPARQL_RESULT(r);
        // Verify that the query is really deleted mid-way through
        if (ensureQueryExecuting(r))
            ++succesfulRounds;
        delete r;
    }
    QVERIFY( succesfulRounds >= 5 );

    // And then spin the event loop so that the async callback is called...
    QTest::qWait(1000);
}

namespace {
class DataReadyListener : public QObject
{
    Q_OBJECT
public:
    DataReadyListener(QSparqlResult* r) : result(r), received(0)
    {
        connect(r, SIGNAL(dataReady(int)),
                SLOT(onDataReady(int)));
    }
public slots:
    void onDataReady(int tc)
    {
        if (result) {
            result->deleteLater();
            result = 0;
        }
        received = tc;
    }
public:
    QSparqlResult* result;
    int received;
};

}

void tst_QSparqlTrackerDirect::delete_nearly_finished_result()
{
    // This is a regression test for a crash bug. Running this shouldn't print
    // out warnings:

    // tst_qsparql_tracker_direct[17909]: GLIB CRITICAL ** GLib-GObject -
    // g_object_unref: assertion `G_IS_OBJECT (object)' failed

    // (It doens't always crash.) Detecting the warnings is a bit of a manual
    // work.

    qDebug() << "delete_nearly_finished_result: no GLIB_CRITICALs should be printed:";

    QSparqlConnectionOptions opts;
    opts.setDataReadyInterval(1);

    QSparqlConnection conn("QTRACKER_DIRECT", opts);
    // A big query returning a lot of results
    QSparqlQuery q("select ?u {?u a rdfs:Resource . }");

    QSparqlResult* r = conn.exec(q);
    CHECK_QSPARQL_RESULT(r);

    DataReadyListener listener(r); // this will delete the result

    // And then spin the event loop. Unfortunately, we don't have anything which
    // we could use for verifying we didn't unref the same object twice (it
    // doesn't always crash).
    QTest::qWait(3000);

}

void tst_QSparqlTrackerDirect::cancel_insert_result()
{
    // This test will leave unclean test data in tracker if it crashes.
    QSparqlConnection conn("QTRACKER_DIRECT");

    QFETCH(bool, waitForConnectionOpen);
    if (waitForConnectionOpen)
        QTest::qWait(2000);

    QSparqlQuery add("insert { <addeduri001> a nco:PersonContact; "
                     "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                     "nco:nameGiven \"addedname001\" .}",
                     QSparqlQuery::InsertStatement);

    QSparqlResult* r = conn.exec(add);
    CHECK_QSPARQL_RESULT(r);
    delete r; r = 0;
    QTest::qWait(3000);

    // Delete the uri
    QSparqlQuery del("delete { <addeduri001> a rdfs:Resource. }",
                     QSparqlQuery::DeleteStatement);

    r = conn.exec(del);
    CHECK_QSPARQL_RESULT(r);
    r->waitForFinished(); // this test is synchronous only
    CHECK_QSPARQL_RESULT(r);
    delete r;
}

void tst_QSparqlTrackerDirect::cancel_insert_result_data()
{
    QTest::addColumn<bool>("waitForConnectionOpen");
    QTest::newRow("Connection is not open") << false;
    QTest::newRow("Connection is open") << true;
}

void tst_QSparqlTrackerDirect::concurrent_queries()
{
    QSparqlConnection conn("QTRACKER_DIRECT");

    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");
    QSparqlResult* r1 = conn.exec(q);
    CHECK_QSPARQL_RESULT(r1);

    QSparqlResult* r2 = conn.exec(q);
    CHECK_QSPARQL_RESULT(r2);

    r1->waitForFinished();
    r2->waitForFinished();

    CHECK_QSPARQL_RESULT(r1);
    QCOMPARE(r1->size(), 3);
    delete r1;
    CHECK_QSPARQL_RESULT(r2);
    QCOMPARE(r2->size(), 3);
    delete r2;
}

void tst_QSparqlTrackerDirect::concurrent_queries_2()
{
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");
    QSparqlResult* r1 = conn.exec(q);
    CHECK_QSPARQL_RESULT(r1);

    QSparqlResult* r2 = conn.exec(q);
    CHECK_QSPARQL_RESULT(r2);

    while (r1->size() < 3 || r2->size() < 3)
        QTest::qWait(1000);

    CHECK_QSPARQL_RESULT(r1);
    QCOMPARE(r1->size(), 3);
    delete r1;
    CHECK_QSPARQL_RESULT(r2);
    QCOMPARE(r2->size(), 3);
    delete r2;
}

void tst_QSparqlTrackerDirect::insert_with_dbus_read_with_direct()
{
    // This test will leave unclean test data in tracker if it crashes.
    QSparqlConnection writeConn("QTRACKER");
    QSparqlConnection readConn("QTRACKER_DIRECT");

    // Insert data with writeconn
    QSparqlQuery add("insert { ?:addeduri a nco:PersonContact; "
                     "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                     "nco:nameGiven \"addedname006\" .}",
                     QSparqlQuery::InsertStatement);
    const QSparqlBinding addeduri(writeConn.createUrn("addeduri"));
    add.bindValue(addeduri);
    QSparqlResult* r = writeConn.exec(add);
    CHECK_QSPARQL_RESULT(r);
    r->waitForFinished(); // this test is synchronous only
    CHECK_QSPARQL_RESULT(r);
    delete r;

    // Verify that the insertion succeeded with readConn
    QSparqlQuery q("select ?addeduri ?ng {?addeduri a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");
    {
        QHash<QString, QSparqlBinding> contactNames;
        r = readConn.exec(q);
        CHECK_QSPARQL_RESULT(r);
        r->waitForFinished();
        CHECK_QSPARQL_RESULT(r);
        QCOMPARE(r->size(), 4);
        while (r->next()) {
            contactNames[r->binding(1).value().toString()] = r->binding(0);
        }
        QCOMPARE(contactNames.size(), 4);
        QCOMPARE(contactNames["addedname006"].value().toString(), addeduri.value().toString());
        delete r;
    }

    // Delete and re-insert data with writeConn
    QSparqlQuery deleteAndAdd("delete { ?:addeduri a rdfs:Resource. } "
                              "insert { ?:addeduri a nco:PersonContact; "
                              "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                              "nco:nameGiven \"addedname006\" .}",
                              QSparqlQuery::InsertStatement);
    deleteAndAdd.bindValue(addeduri);
    r = writeConn.exec(add);
    CHECK_QSPARQL_RESULT(r);
    r->waitForFinished(); // this test is synchronous only
    CHECK_QSPARQL_RESULT(r);
    delete r;

    // Verify once more that the insertion succeeded with readConn
    {
        QHash<QString, QSparqlBinding> contactNames;
        r = readConn.exec(q);
        CHECK_QSPARQL_RESULT(r);
        r->waitForFinished();
        CHECK_QSPARQL_RESULT(r);
        QCOMPARE(r->size(), 4);
        while (r->next()) {
            contactNames[r->binding(1).value().toString()] = r->binding(0);
        }
        QCOMPARE(contactNames.size(), 4);
        QCOMPARE(contactNames["addedname006"].value().toString(),
                 addeduri.value().toString());
        delete r;
    }

    // Delete the uri
    QSparqlQuery del("delete { ?:addeduri a rdfs:Resource. }",
                     QSparqlQuery::DeleteStatement);

    del.bindValue(addeduri);
    r = writeConn.exec(del);
    CHECK_QSPARQL_RESULT(r);
    r->waitForFinished(); // this test is synchronous only
    CHECK_QSPARQL_RESULT(r);
    delete r;
}

void tst_QSparqlTrackerDirect::open_connection_twice()
{
    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");
    {
        QSparqlConnection conn("QTRACKER_DIRECT");
        QSparqlResult* r = conn.exec(q);
        CHECK_QSPARQL_RESULT(r);
        r->waitForFinished();
        CHECK_QSPARQL_RESULT(r);
        QCOMPARE(r->size(), 3);
        delete r;
    } // conn goes out of scope

    {
        QSparqlConnection conn("QTRACKER_DIRECT");
        QSparqlResult* r = conn.exec(q);
        CHECK_QSPARQL_RESULT(r);
        r->waitForFinished();
        CHECK_QSPARQL_RESULT(r);
        QCOMPARE(r->size(), 3);
        delete r;
    }
}

void tst_QSparqlTrackerDirect::result_immediately_finished()
{
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");
    QSparqlResult* r = conn.exec(q);
    CHECK_QSPARQL_RESULT(r);

    // No matter how slow this thread is, the result shouldn't get finished
    // behind our back.
    sleep(3);

    QSignalSpy spy(r, SIGNAL(finished()));
    QTime timer;
    timer.start();
    while (spy.count() == 0 && timer.elapsed() < 3000) {
        QTest::qWait(100);
    }

    QCOMPARE(spy.count(), 1);
    delete r;
}

void tst_QSparqlTrackerDirect::result_immediately_finished2()
{
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");
    QSparqlResult* r = conn.exec(q);
    QSignalSpy spy(r, SIGNAL(finished()));

    CHECK_QSPARQL_RESULT(r);

    // No matter how slow this thread is, the result shouldn't get finished
    // behind our back.
    sleep(3);

    // But when we do waitForFinished, the "side effects" like emitting the
    // finished() signal should occur before it returns.
    r->waitForFinished();
    QCOMPARE(spy.count(), 1);

    // And they should not occur again even if we wait here a bit...
    QTest::qWait(1000);
    QCOMPARE(spy.count(), 1);

    delete r;
}

void tst_QSparqlTrackerDirect::delete_connection_immediately()
{
    QSparqlConnection conn("QTRACKER_DIRECT");
}

void tst_QSparqlTrackerDirect::delete_connection_before_a_wait()
{
    {
        QSparqlConnection conn("QTRACKER_DIRECT");
    }
    QTest::qWait(1000);
}

void tst_QSparqlTrackerDirect::create_2_connections()
{
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlConnection conn2("QTRACKER_DIRECT"); // this hangs
}

void tst_QSparqlTrackerDirect::unsupported_statement_type()
{
    // This test will print out warnings
    setMsgLogLevel(QtCriticalMsg);
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlQuery q("construct { ?u <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#nameGiven> ?ng } where {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-virtuoso-tests> ;"
                   "nco:nameGiven ?ng .}", QSparqlQuery::ConstructStatement);
    QSparqlResult* r = conn.exec(q);
    QVERIFY(r != 0);
    QVERIFY(r->hasError());
    delete r;
}

void tst_QSparqlTrackerDirect::async_conn_opening()
{
    QFETCH(int, delayBeforeFirst);
    QFETCH(int, delayBeforeSecond);

    QSparqlConnection conn("QTRACKER_DIRECT");

    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");

    if (delayBeforeFirst > 0)
        QTest::qWait(delayBeforeFirst);

    QSparqlResult* r1 = conn.exec(q);
    CHECK_QSPARQL_RESULT(r1);
    QSignalSpy spy1(r1, SIGNAL(finished()));

    if (delayBeforeFirst > 0)
        QTest::qWait(delayBeforeSecond);

    QSparqlResult* r2 = conn.exec(q);
    CHECK_QSPARQL_RESULT(r2);
    QSignalSpy spy2(r2, SIGNAL(finished()));

    // Check that we get the finished() signal
    QTime timer;
    timer.start();
    while ((spy1.count() == 0 || spy2.count() == 0) && timer.elapsed() < 5000) {
        QTest::qWait(1000);
    }
    QCOMPARE(spy1.count(), 1);
    QCOMPARE(spy2.count(), 1);

    // Check that the data is correct
    QHash<QString, QString> contactNames1, contactNames2;
    while (r1->next()) {
        QCOMPARE(r1->current().count(), 2);
        contactNames1[r1->value(0).toString()] = r1->value(1).toString();
    }

    while (r2->next()) {
        QCOMPARE(r2->current().count(), 2);
        contactNames2[r2->value(0).toString()] = r2->value(1).toString();
    }
    QCOMPARE(contactNames1.size(), 3);
    QCOMPARE(contactNames1["uri001"], QString("name001"));
    QCOMPARE(contactNames1["uri002"], QString("name002"));
    QCOMPARE(contactNames1["uri003"], QString("name003"));
    QCOMPARE(contactNames2.size(), 3);
    QCOMPARE(contactNames2["uri001"], QString("name001"));
    QCOMPARE(contactNames2["uri002"], QString("name002"));
    QCOMPARE(contactNames2["uri003"], QString("name003"));

    delete r1;
    delete r2;
}

void tst_QSparqlTrackerDirect::async_conn_opening_data()
{
    QTest::addColumn<int>("delayBeforeFirst");
    QTest::addColumn<int>("delayBeforeSecond");

    QTest::newRow("BothBeforeConnOpened")
        << 0 << 0;

    QTest::newRow("BothAfterConnOpened")
        << 2000 << 0;

    QTest::newRow("BeforeAndAfterConnOpened")
        << 0 << 2000;
}

void tst_QSparqlTrackerDirect::async_conn_opening_with_2_connections()
{
    QFETCH(int, delayBeforeCreatingSecondConnection);

    QSparqlConnection conn1("QTRACKER_DIRECT");

    if (delayBeforeCreatingSecondConnection > 0)
        QTest::qWait(delayBeforeCreatingSecondConnection);

    QSparqlConnection conn2("QTRACKER_DIRECT");

    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");

    QSparqlResult* r1 = conn1.exec(q);
    CHECK_QSPARQL_RESULT(r1);
    QSignalSpy spy1(r1, SIGNAL(finished()));

    QSparqlResult* r2 = conn2.exec(q);
    CHECK_QSPARQL_RESULT(r2);
    QSignalSpy spy2(r1, SIGNAL(finished()));

    // Check that we get the finished() signal
    QTime timer;
    timer.start();
    while ((spy1.count() == 0 || spy2.count() == 0) && timer.elapsed() < 5000) {
        QTest::qWait(1000);
    }

    // Check that the data is correct
    QHash<QString, QString> contactNames1, contactNames2;
    while (r1->next()) {
        QCOMPARE(r1->current().count(), 2);
        contactNames1[r1->value(0).toString()] = r1->value(1).toString();
    }

    while (r2->next()) {
        QCOMPARE(r2->current().count(), 2);
        contactNames2[r2->value(0).toString()] = r2->value(1).toString();
    }
    QCOMPARE(contactNames1.size(), 3);
    QCOMPARE(contactNames1["uri001"], QString("name001"));
    QCOMPARE(contactNames1["uri002"], QString("name002"));
    QCOMPARE(contactNames1["uri003"], QString("name003"));
    QCOMPARE(contactNames2.size(), 3);
    QCOMPARE(contactNames2["uri001"], QString("name001"));
    QCOMPARE(contactNames2["uri002"], QString("name002"));
    QCOMPARE(contactNames2["uri003"], QString("name003"));

    delete r1;
    delete r2;
}

void tst_QSparqlTrackerDirect::async_conn_opening_with_2_connections_data()
{
    QTest::addColumn<int>("delayBeforeCreatingSecondConnection");

    QTest::newRow("SecondCreatedBeforeFirstOpened")
        << 0;

    QTest::newRow("SecondCreatedAfterFirstOpened")
        << 2000;
}

void tst_QSparqlTrackerDirect::async_conn_opening_for_update()
{
    QFETCH(int, delayBeforeFirst);
    QFETCH(int, delayBeforeSecond);

    QSparqlConnection conn("QTRACKER_DIRECT");

    QSparqlQuery add1("insert { <addeduri007> a nco:PersonContact; "
                     "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                     "nco:nameGiven \"addedname007\" .}",
                     QSparqlQuery::InsertStatement);

    QSparqlQuery add2("insert { <addeduri008> a nco:PersonContact; "
                     "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                     "nco:nameGiven \"addedname008\" .}",
                     QSparqlQuery::InsertStatement);

    if (delayBeforeFirst > 0)
        QTest::qWait(delayBeforeFirst);

    QSparqlResult* r1 = conn.exec(add1);
    CHECK_QSPARQL_RESULT(r1);
    QSignalSpy spy1(r1, SIGNAL(finished()));

    if (delayBeforeFirst > 0)
        QTest::qWait(delayBeforeSecond);

    QSparqlResult* r2 = conn.exec(add2);
    CHECK_QSPARQL_RESULT(r2);
    QSignalSpy spy2(r2, SIGNAL(finished()));

    // Check that we get the finished() signal
    QTime timer;
    timer.start();
    while ((spy1.count() == 0 || spy2.count() == 0) && timer.elapsed() < 5000) {
        QTest::qWait(1000);
    }
    QCOMPARE(spy1.count(), 1);
    QCOMPARE(spy2.count(), 1);

    CHECK_QSPARQL_RESULT(r1);
    CHECK_QSPARQL_RESULT(r2);

    delete r1;
    delete r2;

    // Verify that the insertion succeeded
    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");
    QHash<QString, QString> contactNames;
    r1 = conn.exec(q);
    CHECK_QSPARQL_RESULT(r1);
    r1->waitForFinished();
    CHECK_QSPARQL_RESULT(r1);
    QCOMPARE(r1->size(), 5);
    while (r1->next()) {
        contactNames[r1->binding(0).value().toString()] =
            r1->binding(1).value().toString();
    }
    QCOMPARE(contactNames.size(), 5);
    QCOMPARE(contactNames["addeduri007"], QString("addedname007"));
    QCOMPARE(contactNames["addeduri008"], QString("addedname008"));
    delete r1;

    QSparqlQuery del1("delete { <addeduri007> a rdfs:Resource. }",
                     QSparqlQuery::DeleteStatement);

    r1 = conn.exec(del1);
    CHECK_QSPARQL_RESULT(r1);
    r1->waitForFinished(); // this test is synchronous only
    CHECK_QSPARQL_RESULT(r1);
    delete r1;

    QSparqlQuery del2("delete { <addeduri008> a rdfs:Resource. }",
                     QSparqlQuery::DeleteStatement);

    r2 = conn.exec(del2);
    CHECK_QSPARQL_RESULT(r2);
    r2->waitForFinished(); // this test is synchronous only
    CHECK_QSPARQL_RESULT(r2);
    delete r2;
}

void tst_QSparqlTrackerDirect::async_conn_opening_for_update_data()
{
    QTest::addColumn<int>("delayBeforeFirst");
    QTest::addColumn<int>("delayBeforeSecond");

    QTest::newRow("BothBeforeConnOpened")
        << 0 << 0;

    QTest::newRow("BothAfterConnOpened")
        << 2000 << 0;

    QTest::newRow("BeforeAndAfterConnOpened")
        << 0 << 2000;
}

namespace {

class DeleteResultOnFinished : public QObject
{
    Q_OBJECT
    public:
        DeleteResultOnFinished(QSparqlResult *result) : r(result) {}
        QSparqlResult *r;
    public Q_SLOTS:
        void onFinished()
        {
            r->deleteLater();
        }
};

}

void tst_QSparqlTrackerDirect::delete_later_with_select_result()
{
    // it doesn't matter what the query is; we don't look at what it returns
    QSparqlQuery query("select ?ie { ?ie a nie:InformationElement . } ");
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlResult* r = conn.exec(query);

    // Start monitoring the destroyed() signal
    QSignalSpy destroyedSpy(r, SIGNAL(destroyed()));

    // When we get the finished() signal of the result, do deleteLater.
    DeleteResultOnFinished resultDeleter(r);
    connect(r, SIGNAL(finished()), &resultDeleter, SLOT(onFinished()));

    QSignalSpy finishedSpy(r, SIGNAL(finished()));
    // Spin the event loop so that the finished() signal arrives
    QTime timer;
    timer.start();
    while (finishedSpy.count() == 0 && timer.elapsed() < 5000) {
        QTest::qWait(100);
    }
    QCOMPARE(finishedSpy.count(), 1);
    // Don't access r any more; it might already have been deleted.

    // Now spinning the event loop should make the DeferredDelete event is
    // processed.  (Note: don't do QCoreApplication::sendPostedEvents(0,
    // QEvent::DeferredDelete here, that will make buggy code pass, too.)
    QTest::qWait(100);

    QCOMPARE(destroyedSpy.count(), 1);
}

void tst_QSparqlTrackerDirect::delete_later_with_update_result()
{
    QSparqlQuery insert("insert {<testresource001> a nie:InformationElement ; "
                        "nie:isLogicalPartOf <qsparql-tracker-live-tests> .}",
                        QSparqlQuery::InsertStatement);
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlResult* r = conn.exec(insert);

    // Start monitoring the destroyed() signal
    QSignalSpy destroyedSpy(r, SIGNAL(destroyed()));

    // When we get the finished() signal of the result, do deleteLater.
    DeleteResultOnFinished resultDeleter(r);
    connect(r, SIGNAL(finished()), &resultDeleter, SLOT(onFinished()));

    QSignalSpy finishedSpy(r, SIGNAL(finished()));

    // Spin the event loop so that the finished() signal arrives
    QTime timer;
    timer.start();
    while (finishedSpy.count() == 0 && timer.elapsed() < 5000) {
        QTest::qWait(100);
    }
    QCOMPARE(finishedSpy.count(), 1);
    // Don't access r any more; it might already have been deleted.

    // Now spinning the event loop should make the DeferredDelete event is
    // processed.  (Note: don't do QCoreApplication::sendPostedEvents(0,
    // QEvent::DeferredDelete here, that will make buggy code pass, too.)
    QTest::qWait(100);

    QCOMPARE(destroyedSpy.count(), 1);

    QSparqlQuery clean("delete {<testresource001> a rdfs:Resource . }",
                       QSparqlQuery::DeleteStatement);
    r = conn.syncExec(clean);
    CHECK_QSPARQL_RESULT(r);
    delete r;
}

void tst_QSparqlTrackerDirect::delete_result_while_update_query_is_executing()
{
    const QSparqlQuery insert("insert {<testresource001> a nie:InformationElement ; "
                        "nie:isLogicalPartOf <qsparql-tracker-live-tests> .}",
                        QSparqlQuery::InsertStatement);
    QSparqlConnection conn("QTRACKER_DIRECT");

    // Wait for the connection to open to ensure the exec call below runs the
    // update immediately
    QTest::qWait(1000);

    QSparqlResult* r = conn.exec(insert);

    // Delete the result
    delete r;

    // Wait for the insertion to complete to check that the result being
    // deleted does not cause problems in the update completion handler
    QTest::qWait(1000);

    // There is nothing to verify through the API

    // Clean up test data
    const QSparqlQuery clean("delete {<testresource001> a rdfs:Resource . }",
                       QSparqlQuery::DeleteStatement);
    r = conn.syncExec(clean);
    CHECK_QSPARQL_RESULT(r);
    delete r;
}

void tst_QSparqlTrackerDirect::query_with_data_ready_set()
{
    QFETCH(int, testResourceCount);
    QFETCH(int, dataReadyInterval);
    QFETCH(int, expectedDataReadySignalCount);

    const QString deleteQuery("delete { ?r a nie:InformationElement } "
                              "where { ?r nie:isLogicalPartOf <qsparql-tracker-direct-data-ready-tests> .}");
    const QString insertTemplate("insert { <data_ready_testresource%1> a nie:InformationElement ; "
                                 "nie:isLogicalPartOf <qsparql-tracker-direct-data-ready-tests> .}");
    QString insertQuery;
    for (int i = 0; i < testResourceCount; ++i) {
        insertQuery.append(insertTemplate.arg(i));
    }

    QSparqlConnectionOptions connOptions;
    connOptions.setDataReadyInterval(dataReadyInterval);
    QSparqlConnection conn("QTRACKER_DIRECT", connOptions);

    QSparqlResult* r = conn.syncExec(QSparqlQuery(deleteQuery, QSparqlQuery::DeleteStatement));
    CHECK_QSPARQL_RESULT(r);
    delete r;

    if (!insertQuery.isEmpty()) {
        r = conn.syncExec(QSparqlQuery(insertQuery, QSparqlQuery::InsertStatement));
        CHECK_QSPARQL_RESULT(r);
        delete r;
    }

    const QString selectQuery("select ?r { ?r a nie:InformationElement ; "
                              "nie:isLogicalPartOf <qsparql-tracker-direct-data-ready-tests> .}");
    r = conn.exec(QSparqlQuery(selectQuery, QSparqlQuery::SelectStatement));
    CHECK_QSPARQL_RESULT(r);
    QSignalSpy finishedSpy(r, SIGNAL(finished()));
    QSignalSpy dataReadySpy(r, SIGNAL(dataReady(int)));

    QTime timer;
    timer.start();
    while (finishedSpy.count() == 0 && timer.elapsed() < 5000) {
        QTest::qWait(100);
    }
    QCOMPARE(finishedSpy.count(), 1);
    QCOMPARE(r->size(), testResourceCount);
    QCOMPARE(dataReadySpy.count(), expectedDataReadySignalCount);
    for (int i = 0, c = 0; i < dataReadySpy.count(); ++i) {
        QCOMPARE(dataReadySpy[i].count(), 1);
        c += dataReadyInterval;
        if (c > r->size())
            c = r->size();
        QCOMPARE(dataReadySpy[i][0].toInt(), c);
    }
    delete r;

    // Clean up test data
    r = conn.syncExec(QSparqlQuery(deleteQuery, QSparqlQuery::InsertStatement));
    CHECK_QSPARQL_RESULT(r);
    delete r;
}

void tst_QSparqlTrackerDirect::query_with_data_ready_set_data()
{
    QTest::addColumn<int>("testResourceCount");
    QTest::addColumn<int>("dataReadyInterval");
    QTest::addColumn<int>("expectedDataReadySignalCount");

    QTest::newRow("No results, data ready interval set to 1")
            << 0 << 1 << 0;

    QTest::newRow("No results, data ready interval set to >1")
            << 0 << 3 << 0;

    QTest::newRow("One result, data ready interval set to 1")
            << 1 << 1 << 1;

    QTest::newRow("One result, data ready interval set to >1")
            << 1 << 2 << 1;

    QTest::newRow("Query size >1, data ready interval set to 1")
            << 5 << 1 << 5;

    QTest::newRow("Query size >1 is equal to data ready interval")
            << 10 << 10 << 1;

    QTest::newRow("Query size >1 is divisible by data ready interval")
            << 15 << 5 << 3;

    QTest::newRow("Query size >1 is indivisible by data ready interval")
            << 17 << 5 << 4;
}

void tst_QSparqlTrackerDirect::destroy_connection_partially_iterated_results()
{
    setMsgLogLevel(QtCriticalMsg);
    const int testDataAmount = 3000;
    const QString testCaseTag("<qsparql-tracker-direct-tests-destroy_connection_partially_iterated_result>");
    QScopedPointer<TestData> testData(
            TestData::createTrackerTestData(testDataAmount, "<qsparql-tracker-direct-tests>", testCaseTag));
    QTest::qWait(1000);
    QVERIFY( testData->isOK() );
    QSparqlConnectionOptions opts;
    opts.setDataReadyInterval(1);

    //run this bit serveral times since we are checking for threading issues
    for(int i=0;i<20;++i) {
        QSparqlConnection *conn = new QSparqlConnection("QTRACKER_DIRECT", opts);

        const int maxRounds = 20;
        int succesfulRounds = 0;
        for(int round=0; round < maxRounds && succesfulRounds == 0; ++round) {
            QSparqlResult* r = conn->exec(testData->selectQuery());
            CHECK_QSPARQL_RESULT(r);
            r->setParent(this);
            // Verify that the connection is really closed mid-way through
            if (ensureQueryExecuting(r)) {
                delete conn; conn = 0;
                ++succesfulRounds;
                // Spin the event loop to ensure all asynchrnous activity has a chance to take place
                QTest::qWait(500);
            }
            // Finally delete the result
            delete r;
        }
        QCOMPARE( succesfulRounds, 1 );
    }
}

void tst_QSparqlTrackerDirect::waitForFinished_after_dataReady()
{
    setMsgLogLevel(QtCriticalMsg);
    const int testDataAmount = 3000;
    const QString testTag("<qsparql-tracker-direct-tests-waitForFinished_after_dataReady>");
    QScopedPointer<TestData> testData(TestData::createTrackerTestData(testDataAmount, "<qsparql-tracker-direct-tests>", testTag));
    QTest::qWait(1000);
    QVERIFY( testData->isOK() );
    QSparqlConnectionOptions opts;
    opts.setDataReadyInterval(1);
    QSparqlConnection conn("QTRACKER_DIRECT", opts);

    const int maxRounds = 20;
    int succesfulRounds = 0;
    for(int round=0; round < maxRounds; ++round) {
        QSparqlResult* r = conn.exec(testData->selectQuery());
        CHECK_QSPARQL_RESULT(r);

        // Verify that the query is really mid-way through
        if (!ensureQueryExecuting(r)) {
            // The query was not executing anymore as we want it to be so retry
            delete r;
        }
        else {
            r->waitForFinished();
            ++succesfulRounds;
            QCOMPARE(r->size(), testDataAmount);
            // Spin the event loop to ensure all asynchrnous activity has a chance to take place
            QTest::qWait(500);
            // Finally delete the result
            delete r;
        }
    }

    QVERIFY(succesfulRounds >= 5);
}

void tst_QSparqlTrackerDirect::test_threadpool_priority_select_results()
{
    const int testDataAmount = 3000;
    const QString testTag("<qsparql-tracker-direct-tests-test_threadpool_priority_select_results>");
    QScopedPointer<TestData> testData(TestData::createTrackerTestData(testDataAmount, "<qsparql-tracker-direct-tests>", testTag));
    QTest::qWait(2000);
    QVERIFY( testData->isOK() );

    // Use only one thread
    QSparqlConnectionOptions options;
    options.setMaxThreadCount(1);
    QSparqlConnection conn("QTRACKER_DIRECT", options);
    bool forwardOnly = false;
    QSparqlQuery select(QString("select ?u {?u a nmm:MusicPiece;"
                                "nie:isLogicalPartOf <qsparql-tracker-direct-tests-test_threadpool_priority_select_results> }"));

    // Do this twice, one with normal results, once with forwardOnly results
    for (int i=0;i<2;i++) {
        if (i==1) {
            forwardOnly = true;
        }
        FinishedSignalReceiver signalReceiver;

        QSparqlQueryOptions result1_options;
        QSparqlQueryOptions result2_options;
        QSparqlQueryOptions result3_options;
        // execute result 4 with normal priority by not setting anything
        QSparqlQueryOptions result5_options;

        result1_options.setPriority(QSparqlQueryOptions::NormalPriority);
        result2_options.setPriority(QSparqlQueryOptions::LowPriority);
        result2_options.setForwardOnly(forwardOnly);
        result3_options.setPriority(QSparqlQueryOptions::NormalPriority);
        result3_options.setForwardOnly(forwardOnly);
        result5_options.setPriority(QSparqlQueryOptions::HighPriority);
        result5_options.setForwardOnly(forwardOnly);

        // first result, default priority
        QSparqlResult *result1 =
            conn.exec(select, result1_options);
        signalReceiver.append(result1);
        // this result should be processed last
        QSparqlResult *result2 =
            conn.exec(select,result2_options);
        signalReceiver.append(result2);
        // this result should be processed after the first one
        // unless some high priority result comes along
        QSparqlResult *result3 =
            conn.exec(select, result3_options);
        signalReceiver.append(result3);
        // default is normal priority, so this should be processed after
        // result 3
        QSparqlResult *result4 =
            conn.exec(select);
        signalReceiver.append(result4);
        // insert a high priority result, should be bumped to the front
        // of the queue to be processed before 3, 4 and 2
        QSparqlResult *result5 =
            conn.exec(select, result5_options);
        signalReceiver.append(result5);

        // wait for them all to finish
        QVERIFY(signalReceiver.waitForAllFinished(8000));

        // order should be result1, result5, result3, result4, result2
        // however if result1 hasn't started before result5 it will
        // be result5, result1, result3, result4, result2 so check this
        // as a special case
        bool firstOrder = false;
        QList<QSparqlResult*> resultOrder = signalReceiver.resultOrder;
        if (resultOrder.at(0) == result1 || resultOrder.at(0) == result5) {
            firstOrder = true;
        }
        QVERIFY(firstOrder);
        QVERIFY(resultOrder.at(2) == result3);
        QVERIFY(resultOrder.at(3) == result4);
        QVERIFY(resultOrder.at(4) == result2);

        // finally just validate they all got the correct number of results
        foreach(QSparqlResult *result, resultOrder) {
            int resultSize = 0;
            if (forwardOnly) {
                while (result->next())
                    resultSize++;
            } else {
                resultSize = result->size();
            }
            QCOMPARE(resultSize, 3000);
        }
    }
}

void tst_QSparqlTrackerDirect::test_threadpool_priority_update_results()
{
    QSparqlConnectionOptions options;
    options.setMaxThreadCount(1);
    QSparqlConnection conn("QTRACKER_DIRECT", options);
    FinishedSignalReceiver signalReceiver;

    QString insertString = "insert { <newInsert-%1> a nco:PersonContact; nco:nameGiven 'name-%1' .}";
    QString selectString = "select <newInsert-%1> ?ng { <newInsert-%1> a nco:PersonContact; nco:nameGiven ?ng }";
    QString deleteString = "delete { <newInsert-%1> a nco:PersonContact }";
    QSparqlQueryOptions result1_options;
    QSparqlQueryOptions result2_options;
    QSparqlQueryOptions result3_options;
    QSparqlQueryOptions result4_options;
    QSparqlQueryOptions result5_options;
    QSparqlQueryOptions result6_options;

    result1_options.setPriority(QSparqlQueryOptions::NormalPriority);
    result2_options.setPriority(QSparqlQueryOptions::LowPriority);
    result3_options.setPriority(QSparqlQueryOptions::LowPriority);
    result4_options.setPriority(QSparqlQueryOptions::NormalPriority);
    result5_options.setPriority(QSparqlQueryOptions::NormalPriority);
    result6_options.setPriority(QSparqlQueryOptions::HighPriority);

    //result1, insert contact 1 - normal priority
    //result2, delete contact 2 - low priority
    //result3, delete contact 1 - low priority
    //result4, select contact 1 - normal priority
    //result5, select contact 2 - normal priority
    //result6, insert contact 2 - high priority
    QSparqlResult *result1 =
        conn.exec(QSparqlQuery(insertString.arg(1),QSparqlQuery::InsertStatement),result1_options);
    signalReceiver.append(result1);
    QSparqlResult *result2 =
        conn.exec(QSparqlQuery(deleteString.arg(1),QSparqlQuery::DeleteStatement),result2_options);
    signalReceiver.append(result2);
    QSparqlResult *result3 =
        conn.exec(QSparqlQuery(deleteString.arg(2),QSparqlQuery::DeleteStatement),result3_options);
    signalReceiver.append(result3);
    QSparqlResult *result4 =
        conn.exec(QSparqlQuery(selectString.arg(1)),result4_options);
    signalReceiver.append(result4);
    QSparqlResult *result5 =
        conn.exec(QSparqlQuery(selectString.arg(2)),result5_options);
    signalReceiver.append(result5);
    QSparqlResult *result6 =
        conn.exec(QSparqlQuery(insertString.arg(2),QSparqlQuery::InsertStatement),result6_options);
    signalReceiver.append(result6);

    signalReceiver.waitForAllFinished(8000);
    // contact 1 was selected with result 4, contact 2 with result 5, check these are correct
    result4->next();
    QCOMPARE(result4->value(0).toString(), QString("newInsert-1"));
    QCOMPARE(result4->value(1).toString(), QString("name-1"));
    result5->next();
    QCOMPARE(result5->value(0).toString(), QString("newInsert-2"));
    QCOMPARE(result5->value(1).toString(), QString("name-2"));

    // Check the inserts got deleted
    QSparqlResult *validateResult1 = conn.exec(QSparqlQuery(selectString.arg(1)));
    validateResult1->waitForFinished();
    QSparqlResult *validateResult2 = conn.exec(QSparqlQuery(selectString.arg(2)));
    validateResult2->waitForFinished();
    QCOMPARE(validateResult1->size(), 0);
    QCOMPARE(validateResult2->size(), 0);
}

QTEST_MAIN( tst_QSparqlTrackerDirect )
#include "tst_qsparql_tracker_direct.moc"
