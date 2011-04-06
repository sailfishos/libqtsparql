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
    void query_contacts_sync();
    void ask_contacts_sync();

    void delete_partially_iterated_result();

    void concurrent_queries();

    void result_type_bool();

    void async_conn_opening();
    void async_conn_opening_data();
    void async_conn_opening_with_2_connections();
    void async_conn_opening_with_2_connections_data();
    void async_conn_opening_for_update();
    void async_conn_opening_for_update_data();

    void data_types();
    void explicit_data_types();
    void large_integer();
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
    qInstallMsgHandler(myMessageOutput);
}

void tst_QSparqlTrackerDirectSync::cleanupTestCase()
{
}

void tst_QSparqlTrackerDirectSync::init()
{
    testLogLevel = QtDebugMsg;
}

void tst_QSparqlTrackerDirectSync::cleanup()
{
}

void tst_QSparqlTrackerDirectSync::query_contacts_sync()
{
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");
    QSparqlResult* r = conn.syncExec(q);
    QVERIFY(r != 0);
    QVERIFY(!r->isFinished());
    CHECK_ERROR(r);
    QCOMPARE(r->size(), -1); // no size info for iterator-type results
    QHash<QString, QString> contactNames;
    while (r->next()) {
        QCOMPARE(r->current().count(), 2);
        QCOMPARE(r->stringValue(0), r->value(0).toString());
        QCOMPARE(r->stringValue(1), r->value(1).toString());

        contactNames[r->value(0).toString()] = r->value(1).toString();
    }
    QVERIFY(r->isFinished());
    QCOMPARE(contactNames.size(), 3);
    QCOMPARE(contactNames["uri001"], QString("name001"));
    QCOMPARE(contactNames["uri002"], QString("name002"));
    QCOMPARE(contactNames["uri003"], QString("name003"));

    CHECK_ERROR(r);
    delete r;
}

void tst_QSparqlTrackerDirectSync::ask_contacts_sync()
{
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlQuery q1("ask {<uri003> a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven \"name003\" .}", QSparqlQuery::AskStatement);
    QSparqlResult* r = conn.syncExec(q1);
    QVERIFY(r != 0);
    CHECK_ERROR(r);
    QVERIFY(!r->isFinished());
    QVERIFY(r->next());
    // We don't set the boolValue for iterator-type results
    QCOMPARE(r->value(0), QVariant(true));
    delete r;

    QSparqlQuery q2("ask {<uri005> a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven \"name005\" .}", QSparqlQuery::AskStatement);
    r = conn.syncExec(q2);
    QVERIFY(r != 0);
    CHECK_ERROR(r);
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
    QVERIFY(r != 0);
    CHECK_ERROR(r);
    QVERIFY(!r->isFinished());
    QVERIFY(r->next());

    delete r;
}

void tst_QSparqlTrackerDirectSync::result_type_bool()
{
    QSparqlConnection conn("QTRACKER_DIRECT");

    // Boolean result
    QSparqlResult* r = conn.syncExec(QSparqlQuery("select 1 > 0 { }"));
    QVERIFY(r != 0);
    CHECK_ERROR(r);
    QVERIFY(r->next());
    QVERIFY(r->value(0).toBool() == true);
    QVERIFY(r->value(0).type() == QVariant::Bool);
    delete r;
    r = conn.syncExec(QSparqlQuery("select 0 > 1 { }"));
    QVERIFY(r != 0);
    CHECK_ERROR(r);
    QVERIFY(r->next());
    QVERIFY(r->value(0).toBool() == false);
    QVERIFY(r->value(0).type() == QVariant::Bool);
    delete r;

    // Another type of boolean result
    r = conn.syncExec(QSparqlQuery("select ?b { <uri004> tracker:available ?b . }"));
    QVERIFY(r != 0);
    CHECK_ERROR(r);
    QVERIFY(r->next());
    QVERIFY(r->value(0).toBool() == true);
    QVERIFY(r->value(0).type() == QVariant::Bool);
    delete r;

    r = conn.syncExec(QSparqlQuery("select ?b { <uri005> tracker:available ?b . }"));
    QVERIFY(r != 0);
    CHECK_ERROR(r);
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
    QVERIFY(r1 != 0);
    CHECK_ERROR(r1);
    QVERIFY(!r1->isFinished());

    QSparqlResult* r2 = conn.syncExec(q);
    QVERIFY(r2 != 0);
    CHECK_ERROR(r2);
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
    CHECK_ERROR(r1);

    if (delayBeforeFirst > 0)
        QTest::qWait(delayBeforeSecond);

    QSparqlResult* r2 = conn.syncExec(q);
    CHECK_ERROR(r2);

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
    CHECK_ERROR(r1);

    QSparqlResult* r2 = conn2.syncExec(q);
    CHECK_ERROR(r2);

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
    CHECK_ERROR(r1);

    if (delayBeforeFirst > 0)
        QTest::qWait(delayBeforeSecond);

    QSparqlResult* r2 = conn.syncExec(add2);
    CHECK_ERROR(r2);

    delete r1;
    delete r2;

    // Verify that the insertion succeeded
    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");
    QHash<QString, QString> contactNames;
    r1 = conn.syncExec(q);
    QVERIFY(r1 != 0);
    CHECK_ERROR(r1);
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
    QVERIFY(r1 != 0);
    CHECK_ERROR(r1);
    delete r1;

    QSparqlQuery del2("delete { <addeduri008> a rdfs:Resource. }",
                     QSparqlQuery::DeleteStatement);

    r2 = conn.syncExec(del2);
    QVERIFY(r2 != 0);
    CHECK_ERROR(r2);
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

void tst_QSparqlTrackerDirectSync::data_types()
{
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlQuery dataTypes("select "
                           "<qsparql-tracker-direct-tests> "
                           "80 "
                           "23.4 "
                           "true "
                           "false "
                           "\"a string\" "
                           "{ }");
    QSparqlResult* r = conn.syncExec(dataTypes);
    QVERIFY(r != 0);
    CHECK_ERROR(r);

    QVERIFY(r->next());
    QCOMPARE(r->value(0),
             QVariant(QUrl::fromEncoded("qsparql-tracker-direct-tests")));
    QCOMPARE(r->value(1), QVariant(80));
    QCOMPARE(r->value(2), QVariant(23.4));
    QCOMPARE(r->value(3), QVariant(true));
    QCOMPARE(r->value(4), QVariant(false));
    QCOMPARE(r->value(5), QVariant(QString::fromLatin1("a string")));

    // urls don't have data type uris
    QCOMPARE(r->binding(0).dataTypeUri(),
            QUrl::fromEncoded(""));

    QCOMPARE(r->binding(1).dataTypeUri(),
             QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#integer"));

    QCOMPARE(r->binding(2).dataTypeUri(),
             QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#double"));

    QCOMPARE(r->binding(3).dataTypeUri(),
             QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#boolean"));
    QCOMPARE(r->binding(4).dataTypeUri(),
             QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#boolean"));

    QCOMPARE(r->binding(5).dataTypeUri(),
             QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#string"));

    delete r;
}

void tst_QSparqlTrackerDirectSync::explicit_data_types()
{
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlQuery explicitTypes("select "
                               "<qsparql-tracker-direct-tests> "
                               "\"80\"^^xsd:integer "
                               "\"-7\"^^xsd:int "
                               "\"23.4\"^^xsd:double "
                               "\"true\"^^xsd:boolean "
                               "\"false\"^^xsd:boolean "
                               "\"a string\"^^xsd:string "
                               "\"2011-03-28T09:36:00+02:00\"^^xsd:dateTime "
                               "{ }");
    QSparqlResult* r = conn.syncExec(explicitTypes);
    QVERIFY(r != 0);
    CHECK_ERROR(r);

    QVERIFY(r->next());
    QCOMPARE(r->value(0),
             QVariant(QUrl::fromEncoded("qsparql-tracker-direct-tests")));
    QCOMPARE(r->value(1), QVariant(80));
    QCOMPARE(r->value(2), QVariant(-7));
    QCOMPARE(r->value(3), QVariant(23.4));
    QCOMPARE(r->value(4), QVariant(true));
    QCOMPARE(r->value(5), QVariant(false));
    QCOMPARE(r->value(6), QVariant(QString::fromLatin1("a string")));

    // Tracker seems to return the datetime as a string
    QEXPECT_FAIL("", "Tracker returns dates as strings", Continue);
    QCOMPARE(r->value(7),
             QVariant(QDateTime::fromString("2011-03-28T09:36:00+02:00", Qt::ISODate)));

    // urls don't have data type uris
    QCOMPARE(r->binding(0).dataTypeUri(),
            QUrl::fromEncoded(""));

    QCOMPARE(r->binding(1).dataTypeUri(),
             QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#integer"));
    QCOMPARE(r->binding(2).dataTypeUri(),
             QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#integer"));

    QEXPECT_FAIL("", "Tracker returns doubles as strings", Continue);
    QCOMPARE(r->binding(3).dataTypeUri(),
             QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#double"));

    QCOMPARE(r->binding(4).dataTypeUri(),
             QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#boolean"));
    QCOMPARE(r->binding(5).dataTypeUri(),
             QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#boolean"));

    QCOMPARE(r->binding(6).dataTypeUri(),
             QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#string"));

    QEXPECT_FAIL("", "Tracker returns dates as strings", Continue);
    QCOMPARE(r->binding(7).dataTypeUri(),
             QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#dateTime"));

    delete r;
}

void tst_QSparqlTrackerDirectSync::large_integer()
{
    QSparqlQuery insert("insert {<mydataobject> a nie:DataObject, nie:InformationElement ; "
                        "nie:isLogicalPartOf <qsparql-tracker-live-tests> ;"
                        "nie:byteSize 5000000000 .}", QSparqlQuery::InsertStatement);
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlResult* r = conn.syncExec(insert);
    CHECK_ERROR(r);
    delete r;
    QSparqlQuery select("select ?do ?bs "
                        "{ ?do a nie:DataObject ; "
                        "nie:isLogicalPartOf <qsparql-tracker-live-tests> ;"
                        "nie:byteSize ?bs .}");
    r = conn.syncExec(select);

    CHECK_ERROR(r);
    QVERIFY(r->next());
    QCOMPARE(r->value(1), QVariant(Q_UINT64_C(5000000000)));
    delete r;

    QSparqlQuery clean("delete {<mydataobject> a rdfs:Resource . }",
                       QSparqlQuery::DeleteStatement);
    r = conn.syncExec(clean);
    CHECK_ERROR(r);
    delete r;
}

QTEST_MAIN( tst_QSparqlTrackerDirectSync )
#include "tst_qsparql_tracker_direct_sync.moc"
