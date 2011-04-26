/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (ivan.frade@nokia.com)
**
** This file is part of the test suite of the QtSparql module (not yet part of the Qt Toolkit).
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
** Nokia at ivan.frade@nokia.com.
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

#include "../testhelpers.h"
#include "../tracker_direct_common.h"

#include <QtTest/QtTest>
#include <QtSparql/QtSparql>

//const QString qtest(qTableName( "qtest", __FILE__ )); // FIXME: what's this

//TESTED_FILES=

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

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void qsparqlresultrow();
    void query_contacts_async();
    void ask_contacts();
    void insert_and_delete_contact_async();
    void insert_new_urn();

    void delete_unfinished_result();
    void delete_partially_iterated_result();
    void delete_nearly_finished_result();
    void cancel_insert_result();
    
    void concurrent_queries();
    void concurrent_queries_2();

    void insert_with_dbus_read_with_direct();

    void open_connection_twice();

    void result_immediately_finished();
    void result_immediately_finished2();
    
    void delete_connection_immediately();
    void delete_connection_before_a_wait();

    void go_beyond_columns_number();

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
};

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

void tst_QSparqlTrackerDirect::initTestCase()
{
    // clean any remainings
    QVERIFY(cleanData() != 0);
    // For running the test without installing the plugins. Should work in
    // normal and vpath builds.
    QCoreApplication::addLibraryPath("../../../plugins");
    installMsgHandler();
    QVERIFY(setupData() != 0);
}

void tst_QSparqlTrackerDirect::cleanupTestCase()
{
    QVERIFY(cleanData() != 0);
}

void tst_QSparqlTrackerDirect::init()
{
    setMsgLogLevel(QtDebugMsg);
}

void tst_QSparqlTrackerDirect::cleanup()
{
}

void tst_QSparqlTrackerDirect::qsparqlresultrow()
{
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");
    QSparqlResult* r = conn.exec(q);
    QVERIFY(r != 0);
    r->waitForFinished(); // this test is synchronous only
    CHECK_ERROR(r);
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

void tst_QSparqlTrackerDirect::query_contacts_async()
{
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");
    QSparqlResult* r = conn.exec(q);
    QVERIFY(r != 0);
    CHECK_ERROR(r);

    QTime timer;
    timer.start();
    QSignalSpy spy(r, SIGNAL(finished()));
    while (spy.count() == 0 && timer.elapsed() < 5000) {
        QTest::qWait(100);
    }

    QCOMPARE(spy.count(), 1);

    CHECK_ERROR(r);
    QCOMPARE(r->size(), 3);
    QHash<QString, QString> contactNames;
    while (r->next()) {
        QCOMPARE(r->current().count(), 2);
        contactNames[r->value(0).toString()] = r->value(1).toString();
    }
    QCOMPARE(contactNames.size(), 3);
    QCOMPARE(contactNames["uri001"], QString("name001"));
    QCOMPARE(contactNames["uri002"], QString("name002"));
    QCOMPARE(contactNames["uri003"], QString("name003"));
    delete r;
}

void tst_QSparqlTrackerDirect::ask_contacts()
{
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlQuery q1("ask {<uri003> a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven \"name003\" .}", QSparqlQuery::AskStatement);
    QSparqlResult* r = conn.exec(q1);
    QVERIFY(r != 0);
    r->waitForFinished(); // this test is synchronous only
    CHECK_ERROR(r);
    QCOMPARE(r->boolValue(), true);
    delete r;

    QSparqlQuery q2("ask {<uri005> a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven \"name005\" .}", QSparqlQuery::AskStatement);
    r = conn.exec(q2);
    QVERIFY(r != 0);
    r->waitForFinished(); // this test is synchronous only
    CHECK_ERROR(r);
    QCOMPARE(r->boolValue(), false);
    delete r;
}

void tst_QSparqlTrackerDirect::insert_and_delete_contact_async()
{
    // This test will leave unclean test data in tracker if it crashes.
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlQuery add("insert { <addeduri001> a nco:PersonContact; "
                     "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                     "nco:nameGiven \"addedname001\" .}",
                     QSparqlQuery::InsertStatement);

    QSparqlResult* r = conn.exec(add);
    QVERIFY(r != 0);
    CHECK_ERROR(r);

    QTime timer;
    timer.start();
    QSignalSpy insertSpy(r, SIGNAL(finished()));
    while (insertSpy.count() == 0 && timer.elapsed() < 5000) {
        QTest::qWait(100);
    }
    QCOMPARE(insertSpy.count(), 1);

    CHECK_ERROR(r);
    delete r;

    // Verify that the insertion succeeded
    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");
    QHash<QString, QString> contactNames;
    r = conn.exec(q);
    QVERIFY(r != 0);
    r->waitForFinished();
    QCOMPARE(r->size(), 4);
    while (r->next()) {
        contactNames[r->binding(0).value().toString()] =
            r->binding(1).value().toString();
    }
    QCOMPARE(contactNames.size(), 4);
    QCOMPARE(contactNames["addeduri001"], QString("addedname001"));
    delete r;

    // Delete the uri
    QSparqlQuery del("delete { <addeduri001> a rdfs:Resource. }",
                     QSparqlQuery::DeleteStatement);

    r = conn.exec(del);
    QVERIFY(r != 0);
    CHECK_ERROR(r);

    timer.restart();
    QSignalSpy deleteSpy(r, SIGNAL(finished()));
    while (deleteSpy.count() == 0 && timer.elapsed() < 5000) {
        QTest::qWait(100);
    }
    QCOMPARE(deleteSpy.count(), 1);

    CHECK_ERROR(r);
    delete r;

    // Verify that it got deleted
    contactNames.clear();
    r = conn.exec(q);
    QVERIFY(r != 0);
    r->waitForFinished();
    CHECK_ERROR(r);
    QCOMPARE(r->size(), 3);
    while (r->next()) {
        contactNames[r->binding(0).value().toString()] =
            r->binding(1).value().toString();
    }
    QCOMPARE(contactNames.size(), 3);
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
    QVERIFY(r != 0);
    r->waitForFinished(); // this test is synchronous only
    CHECK_ERROR(r);
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
        // qDebug() << r->binding(0).toString() << r->binding(1).toString();
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
    QVERIFY(r != 0);
    r->waitForFinished(); // this test is synchronous only
    CHECK_ERROR(r);
    delete r;

    // Verify that it got deleted
    contactNames.clear();
    r = conn.exec(q);
    QVERIFY(r != 0);
    r->waitForFinished();
    CHECK_ERROR(r);
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
    QVERIFY(r != 0);
    CHECK_ERROR(r);
    delete r;
    // Spin the event loop so that the async callback is called.
    QTest::qWait(1000);
}

void tst_QSparqlTrackerDirect::delete_partially_iterated_result()
{
    const int testDataAmount = 2000;
    const QString testTag("<qsparql-tracker-direct-tests-delete_partially_iterated_result>");
    QScopedPointer<TestData> testData(createTestData(testDataAmount, testTag));
    QTest::qWait(1000);
    QVERIFY( testData->isOK() );
    QSparqlConnectionOptions opts;
    opts.setDataReadyInterval(1);
    QSparqlConnection conn("QTRACKER_DIRECT", opts);
    const QSparqlQuery q(
        QString("select tracker:id(?musicPiece) ?title ?performer ?album ?duration ?created "
            "{ "
            "?musicPiece a nmm:MusicPiece; "
            "nie:isLogicalPartOf %1; "
            "nie:title ?title; "
            "nmm:performer ?performer; "
            "nmm:musicAlbum ?album; "
            "nfo:duration ?duration; "
            "nie:contentCreated ?created. "
            "} order by ?title ?created").arg(testTag));

    QSparqlResult* r = conn.exec(q);
    QVERIFY(r != 0);
    CHECK_ERROR(r);

    // Wait for some dataReady signals
    QSignalSpy spy(r, SIGNAL(dataReady(int)));
    while (spy.count() < 2)
        QTest::qWait(1);
    // Verify that the query is really deleted mid-way through
    QVERIFY(!r->isFinished());
    delete r;

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
        //qDebug() << "Ready" << tc;
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
    QVERIFY(r != 0);
    CHECK_ERROR(r);

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
    QSparqlQuery add("insert { <addeduri001> a nco:PersonContact; "
                     "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                     "nco:nameGiven \"addedname001\" .}",
                     QSparqlQuery::InsertStatement);

    QSparqlResult* r = conn.exec(add);
    QVERIFY(r != 0);
    CHECK_ERROR(r);
    delete r;
    QTest::qWait(3000);

    // Delete the uri
    QSparqlQuery del("delete { <addeduri001> a rdfs:Resource. }",
                     QSparqlQuery::DeleteStatement);

    r = conn.exec(del);
    QVERIFY(r != 0);
    r->waitForFinished(); // this test is synchronous only
    CHECK_ERROR(r);
    delete r;
}

void tst_QSparqlTrackerDirect::concurrent_queries()
{
    QSparqlConnection conn("QTRACKER_DIRECT");

    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");
    QSparqlResult* r1 = conn.exec(q);
    QVERIFY(r1 != 0);
    CHECK_ERROR(r1);

    QSparqlResult* r2 = conn.exec(q);
    QVERIFY(r2 != 0);
    CHECK_ERROR(r2);

    r1->waitForFinished();
    r2->waitForFinished();

    CHECK_ERROR(r1);
    QCOMPARE(r1->size(), 3);
    delete r1;
    CHECK_ERROR(r2);
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
    QVERIFY(r1 != 0);
    CHECK_ERROR(r1);

    QSparqlResult* r2 = conn.exec(q);
    QVERIFY(r2 != 0);
    CHECK_ERROR(r2);

    while (r1->size() < 3 || r2->size() < 3)
        QTest::qWait(1000);

    CHECK_ERROR(r1);
    QCOMPARE(r1->size(), 3);
    delete r1;
    CHECK_ERROR(r2);
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
    QVERIFY(r);
    CHECK_ERROR(r);
    r->waitForFinished(); // this test is synchronous only
    CHECK_ERROR(r);
    delete r;

    // Verify that the insertion succeeded with readConn
    QSparqlQuery q("select ?addeduri ?ng {?addeduri a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");
    {
        QHash<QString, QSparqlBinding> contactNames;
        r = readConn.exec(q);
        QVERIFY(r);
        r->waitForFinished();
        CHECK_ERROR(r);
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
    QVERIFY(r);
    r->waitForFinished(); // this test is synchronous only
    CHECK_ERROR(r);
    delete r;

    // Verify once more that the insertion succeeded with readConn
    {
        QHash<QString, QSparqlBinding> contactNames;
        r = readConn.exec(q);
        QVERIFY(r);
        r->waitForFinished();
        CHECK_ERROR(r);
        QCOMPARE(r->size(), 4);
        while (r->next()) {
            // qDebug() << r->binding(0).toString() << r->binding(1).toString();
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
    qDebug() << r->query();
    QVERIFY(r);
    r->waitForFinished(); // this test is synchronous only
    CHECK_ERROR(r);
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
        QVERIFY(r != 0);
        r->waitForFinished();
        CHECK_ERROR(r);
        QCOMPARE(r->size(), 3);
        delete r;
    } // conn goes out of scope

    {
        QSparqlConnection conn("QTRACKER_DIRECT");
        QSparqlResult* r = conn.exec(q);
        QVERIFY(r != 0);
        r->waitForFinished();
        CHECK_ERROR(r);
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
    QVERIFY(r != 0);
    CHECK_ERROR(r);

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

    QVERIFY(r != 0);
    CHECK_ERROR(r);

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
    // QSparqlConnection conn("QTRACKER_DIRECT");
}

void tst_QSparqlTrackerDirect::delete_connection_before_a_wait()
{
    {
        QSparqlConnection conn("QTRACKER_DIRECT");
    }
    QTest::qWait(1000);
}

void tst_QSparqlTrackerDirect::go_beyond_columns_number()
{
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");
    QSparqlResult* r = conn.exec(q);
    QVERIFY(r != 0);
    r->waitForFinished(); // this test is synchronous only
    CHECK_ERROR(r);
    QCOMPARE(r->size(), 3);
    while (r->next()) {
        QCOMPARE(r->current().count(), 2);
        QCOMPARE(r->value(5).toString(), QString());
        QCOMPARE(r->binding(5).toString(), QString());
    }
    delete r;
}

void tst_QSparqlTrackerDirect::create_2_connections()
{
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlConnection conn2("QTRACKER_DIRECT"); // this hangs
}

void tst_QSparqlTrackerDirect::unsupported_statement_type()
{
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
    CHECK_ERROR(r1);
    QSignalSpy spy1(r1, SIGNAL(finished()));

    if (delayBeforeFirst > 0)
        QTest::qWait(delayBeforeSecond);

    QSparqlResult* r2 = conn.exec(q);
    CHECK_ERROR(r2);
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
    CHECK_ERROR(r1);
    QSignalSpy spy1(r1, SIGNAL(finished()));

    QSparqlResult* r2 = conn2.exec(q);
    CHECK_ERROR(r2);
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
    CHECK_ERROR(r1);
    QSignalSpy spy1(r1, SIGNAL(finished()));

    if (delayBeforeFirst > 0)
        QTest::qWait(delayBeforeSecond);

    QSparqlResult* r2 = conn.exec(add2);
    CHECK_ERROR(r2);
    QSignalSpy spy2(r2, SIGNAL(finished()));

    // Check that we get the finished() signal
    QTime timer;
    timer.start();
    while ((spy1.count() == 0 || spy2.count() == 0) && timer.elapsed() < 5000) {
        QTest::qWait(1000);
    }
    QCOMPARE(spy1.count(), 1);
    QCOMPARE(spy2.count(), 1);

    CHECK_ERROR(r1);
    CHECK_ERROR(r2);

    delete r1;
    delete r2;

    // Verify that the insertion succeeded
    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");
    QHash<QString, QString> contactNames;
    r1 = conn.exec(q);
    QVERIFY(r1 != 0);
    r1->waitForFinished();
    CHECK_ERROR(r1);
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
    QVERIFY(r1 != 0);
    r1->waitForFinished(); // this test is synchronous only
    CHECK_ERROR(r1);
    delete r1;

    QSparqlQuery del2("delete { <addeduri008> a rdfs:Resource. }",
                     QSparqlQuery::DeleteStatement);

    r2 = conn.exec(del2);
    QVERIFY(r2 != 0);
    r2->waitForFinished(); // this test is synchronous only
    CHECK_ERROR(r2);
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
    public Q_SLOTS:
    void onFinished()
    {
        sender()->deleteLater();
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
    DeleteResultOnFinished resultDeleter;
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
    DeleteResultOnFinished resultDeleter;
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
    CHECK_ERROR(r);
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
    CHECK_ERROR(r);
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
    CHECK_ERROR(r);
    delete r;

    if (!insertQuery.isEmpty()) {
        r = conn.syncExec(QSparqlQuery(insertQuery, QSparqlQuery::InsertStatement));
        CHECK_ERROR(r);
        delete r;
    }

    const QString selectQuery("select ?r { ?r a nie:InformationElement ; "
                              "nie:isLogicalPartOf <qsparql-tracker-direct-data-ready-tests> .}");
    r = conn.exec(QSparqlQuery(selectQuery, QSparqlQuery::SelectStatement));
    CHECK_ERROR(r);
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

    // Clean up test data
    r = conn.syncExec(QSparqlQuery(deleteQuery, QSparqlQuery::InsertStatement));
    CHECK_ERROR(r);
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

QTEST_MAIN( tst_QSparqlTrackerDirect )
#include "tst_qsparql_tracker_direct.moc"
