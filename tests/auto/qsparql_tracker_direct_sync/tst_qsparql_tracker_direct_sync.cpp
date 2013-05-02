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
#include <QtSparql>

class tst_QSparqlTrackerDirectSync : public TrackerDirectCommon
{
    Q_OBJECT

public:
    tst_QSparqlTrackerDirectSync();
    virtual ~tst_QSparqlTrackerDirectSync();

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
    void ask_contacts_sync();

    void delete_partially_iterated_result();
    void reparented_results();

    void concurrent_queries();

    void result_type_bool();

    void async_conn_opening();
    void async_conn_opening_data();
    void async_conn_opening_with_2_connections();
    void async_conn_opening_with_2_connections_data();
    void async_conn_opening_for_update();
    void async_conn_opening_for_update_data();

};

tst_QSparqlTrackerDirectSync::tst_QSparqlTrackerDirectSync()
{
}

tst_QSparqlTrackerDirectSync::~tst_QSparqlTrackerDirectSync()
{
}

QSparqlResult* tst_QSparqlTrackerDirectSync::execQuery(QSparqlConnection &conn, const QSparqlQuery &q){
    QSparqlResult* r = conn.syncExec(q);
    return r;
}

void tst_QSparqlTrackerDirectSync::waitForQueryFinished(QSparqlResult* r)
{
    Q_UNUSED(r);
}

bool tst_QSparqlTrackerDirectSync::checkResultSize(QSparqlResult* r, int s){
    Q_UNUSED(s);
    return (r->size() == -1);
}

void tst_QSparqlTrackerDirectSync::initTestCase()
{
    // For running the test without installing the plugins. Should work in
    // normal and vpath builds.
    QCoreApplication::addLibraryPath("../../../plugins");
    installMsgHandler();

    // clean any remainings
    QVERIFY(cleanData());

    QVERIFY(setupData());
}

void tst_QSparqlTrackerDirectSync::cleanupTestCase()
{
    QVERIFY(cleanData());
}

void tst_QSparqlTrackerDirectSync::init()
{
    setMsgLogLevel(QtDebugMsg);
}

void tst_QSparqlTrackerDirectSync::cleanup()
{
}

void tst_QSparqlTrackerDirectSync::ask_contacts_sync()
{
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlQuery q1("ask {<uri003> a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven \"name003\" .}", QSparqlQuery::AskStatement);
    QSparqlResult* r = conn.syncExec(q1);
    CHECK_QSPARQL_RESULT(r);
    QVERIFY(!r->isFinished());
    QVERIFY(r->next());
    // We don't set the boolValue for iterator-type results
    QCOMPARE(r->value(0), QVariant(true));
    delete r;

    QSparqlQuery q2("ask {<uri005> a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven \"name005\" .}", QSparqlQuery::AskStatement);
    r = conn.syncExec(q2);
    CHECK_QSPARQL_RESULT(r);
    QVERIFY(!r->isFinished());
    QVERIFY(r->next());
    // We don't set the boolValue for iterator-type results
    QCOMPARE(r->value(0), QVariant(false));
    delete r;
}

void tst_QSparqlTrackerDirectSync::delete_partially_iterated_result()
{
    QSparqlConnectionOptions opts;
    opts.setOption("dataReadyInterval", 1);

    QSparqlConnection conn("QTRACKER_DIRECT", opts);
    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");
    QSparqlResult* r = conn.syncExec(q);
    CHECK_QSPARQL_RESULT(r);
    QVERIFY(!r->isFinished());
    QVERIFY(r->next());

    delete r;
}

void tst_QSparqlTrackerDirectSync::reparented_results()
{
    // We're not that interested in actual results here, we have enough
    // tests for that (qsparql_api), just that deleting the connection + results
    // doesn't cause any issues
    // Set the log level to Critical since there is a lot of warning output related
    // to deleting the connection before a result completed.
    setMsgLogLevel(QtCriticalMsg);

    QSparqlConnection *connection = new QSparqlConnection("QTRACKER_DIRECT");
    QList<QPointer<QSparqlResult> > resultList;
    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");

    for (int i=0;i<50;i++) {
        QSparqlResult *result = 0;
        // have a mix of syncExec, exec with options
        if (i%3 == 0) {
            QSparqlQueryOptions options;
            options.setExecutionMethod(QSparqlQueryOptions::SyncExec);
            result = connection->exec(q, options);
        } else {
            result = connection->syncExec(q);
        }
        resultList.append(result);
        // also reparent half of them
        if (i%2 == 0)
            result->setParent(this);
    }

    // Delete the connection, check that non-null results (there should be 25)
    // don't crash when nexting...
    delete connection;
    int reparentedResults = 0;
    Q_FOREACH (QPointer<QSparqlResult> result, resultList) {
        if (!result.isNull()) {
            reparentedResults++;
            // Check some behaviour of these results to make sure everything
            // got cleaned up correctly in the result, after deleting the connection
            //
            // hasError() should be true since we deleted the result
            // before we read any data (isFinished() was false)
            QVERIFY(result->hasError());
            // next() should be false since there is no longer a
            // connection to use
            QVERIFY(!result->next());
            // try and read some value from it, should be a null
            // QVariant
            QCOMPARE(result->value(0), QVariant());
            QCOMPARE(result->value(0).toString(), QString());
            // check the binding, again this should be null
            QCOMPARE(result->binding(0), QSparqlBinding());
            delete result;
        }
    }
    QCOMPARE(reparentedResults, 25);
    // now make sure all pointers are null
    Q_FOREACH (QPointer<QSparqlResult> result, resultList) {
        QVERIFY(result.isNull());
    }
}

void tst_QSparqlTrackerDirectSync::result_type_bool()
{
    QSparqlConnection conn("QTRACKER_DIRECT");

    // Boolean result
    QSparqlResult* r = conn.syncExec(QSparqlQuery("select 1 > 0 { }"));
    CHECK_QSPARQL_RESULT(r);
    QVERIFY(r->next());
    QVERIFY(r->value(0).toBool() == true);
    QVERIFY(r->value(0).type() == QVariant::Bool);
    delete r;
    r = conn.syncExec(QSparqlQuery("select 0 > 1 { }"));
    CHECK_QSPARQL_RESULT(r);
    QVERIFY(r->next());
    QVERIFY(r->value(0).toBool() == false);
    QVERIFY(r->value(0).type() == QVariant::Bool);
    delete r;

    // Another type of boolean result
    r = conn.syncExec(QSparqlQuery("select ?b { <uri004> tracker:available ?b . }"));
    CHECK_QSPARQL_RESULT(r);
    QVERIFY(r->next());
    QVERIFY(r->value(0).toBool() == true);
    QVERIFY(r->value(0).type() == QVariant::Bool);
    delete r;

    r = conn.syncExec(QSparqlQuery("select ?b { <uri005> tracker:available ?b . }"));
    CHECK_QSPARQL_RESULT(r);
    QVERIFY(r->next());
    QVERIFY(r->value(0).toBool() == false);
    QVERIFY(r->value(0).type() == QVariant::Bool);
    delete r;
}

void tst_QSparqlTrackerDirectSync::concurrent_queries()
{
    QSparqlConnection conn("QTRACKER_DIRECT");

    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");
    QSparqlResult* r1 = conn.syncExec(q);
    CHECK_QSPARQL_RESULT(r1);
    QVERIFY(!r1->isFinished());

    QSparqlResult* r2 = conn.syncExec(q);
    CHECK_QSPARQL_RESULT(r2);
    QVERIFY(!r2->isFinished());

    QHash<QString, QString> contactNames1, contactNames2;
    for (int i = 0; i < 3; ++i) {
        QVERIFY(r1->next());
        QVERIFY(r2->next());
        contactNames1[r1->value(0).toString()] = r1->value(1).toString();
        contactNames2[r2->value(0).toString()] = r2->value(1).toString();
    }
    QVERIFY(!r1->next());
    QVERIFY(!r2->next());
    QVERIFY(r1->isFinished());
    QVERIFY(r2->isFinished());
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

void tst_QSparqlTrackerDirectSync::async_conn_opening()
{
    QFETCH(int, delayBeforeFirst);
    QFETCH(int, delayBeforeSecond);

    QSparqlConnection conn("QTRACKER_DIRECT");

    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");

    if (delayBeforeFirst > 0)
        QTest::qWait(delayBeforeFirst);

    QSparqlResult* r1 = conn.syncExec(q);
    CHECK_QSPARQL_RESULT(r1);

    if (delayBeforeFirst > 0)
        QTest::qWait(delayBeforeSecond);

    QSparqlResult* r2 = conn.syncExec(q);
    CHECK_QSPARQL_RESULT(r2);

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

void tst_QSparqlTrackerDirectSync::async_conn_opening_data()
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

void tst_QSparqlTrackerDirectSync::async_conn_opening_with_2_connections()
{
    QFETCH(int, delayBeforeCreatingSecondConnection);

    QSparqlConnection conn1("QTRACKER_DIRECT");

    if (delayBeforeCreatingSecondConnection > 0)
        QTest::qWait(delayBeforeCreatingSecondConnection);

    QSparqlConnection conn2("QTRACKER_DIRECT");

    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");

    QSparqlResult* r1 = conn1.syncExec(q);
    CHECK_QSPARQL_RESULT(r1);

    QSparqlResult* r2 = conn2.syncExec(q);
    CHECK_QSPARQL_RESULT(r2);

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

void tst_QSparqlTrackerDirectSync::async_conn_opening_with_2_connections_data()
{
    QTest::addColumn<int>("delayBeforeCreatingSecondConnection");

    QTest::newRow("SecondCreatedBeforeFirstOpened")
        << 0;

    QTest::newRow("SecondCreatedAfterFirstOpened")
        << 2000;
}

void tst_QSparqlTrackerDirectSync::async_conn_opening_for_update()
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

    QSparqlResult* r1 = conn.syncExec(add1);
    CHECK_QSPARQL_RESULT(r1);

    if (delayBeforeFirst > 0)
        QTest::qWait(delayBeforeSecond);

    QSparqlResult* r2 = conn.syncExec(add2);
    CHECK_QSPARQL_RESULT(r2);

    delete r1;
    delete r2;

    // Verify that the insertion succeeded
    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");
    QHash<QString, QString> contactNames;
    r1 = conn.syncExec(q);
    CHECK_QSPARQL_RESULT(r1);
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

    r1 = conn.syncExec(del1);
    CHECK_QSPARQL_RESULT(r1);
    delete r1;

    QSparqlQuery del2("delete { <addeduri008> a rdfs:Resource. }",
                     QSparqlQuery::DeleteStatement);

    r2 = conn.syncExec(del2);
    CHECK_QSPARQL_RESULT(r2);
    delete r2;
}

void tst_QSparqlTrackerDirectSync::async_conn_opening_for_update_data()
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

QTEST_MAIN( tst_QSparqlTrackerDirectSync )
#include "tst_qsparql_tracker_direct_sync.moc"
