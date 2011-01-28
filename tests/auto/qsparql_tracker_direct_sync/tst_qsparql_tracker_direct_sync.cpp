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

#include <QtTest/QtTest>
#include <QtSparql/QtSparql>

//const QString qtest(qTableName( "qtest", __FILE__ )); // FIXME: what's this

//TESTED_FILES=

class tst_QSparqlTrackerDirectSync : public QObject
{
    Q_OBJECT

public:
    tst_QSparqlTrackerDirectSync();
    virtual ~tst_QSparqlTrackerDirectSync();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void query_contacts_sync();
    void ask_contacts_sync();
    void insert_and_delete_contact_sync();

    void query_with_error_sync();

    void iterate_result_sync();

    void delete_partially_iterated_result();

    void concurrent_queries();

    void result_type_bool();

    void special_chars();
};

namespace {
int testLogLevel = QtWarningMsg;
void myMessageOutput(QtMsgType type, const char *msg)
{
    switch (type) {
    case QtDebugMsg:
        if (testLogLevel <= 0)
            fprintf(stderr, "QDEBUG : %s\n", msg);
        break;
    case QtWarningMsg:
        if (testLogLevel <= 1)
            fprintf(stderr, "QWARN  : %s\n", msg);
        break;
    case QtCriticalMsg:
        if (testLogLevel <= 2)
            fprintf(stderr, "QCRITICAL: %s\n", msg);
        break;
    case QtFatalMsg:
        if (testLogLevel <= 3)
            fprintf(stderr, "QFATAL : %s\n", msg);
        abort();
    }
}
} // end unnamed namespace

tst_QSparqlTrackerDirectSync::tst_QSparqlTrackerDirectSync()
{
}

tst_QSparqlTrackerDirectSync::~tst_QSparqlTrackerDirectSync()
{
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
    QCOMPARE(r->hasError(), false);
    QVERIFY(!r->isFinished());
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

    QCOMPARE(r->hasError(), false);
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
    QCOMPARE(r->hasError(), false);
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
    QCOMPARE(r->hasError(), false);
    QVERIFY(!r->isFinished());
    QVERIFY(r->next());
    // We don't set the boolValue for iterator-type results
    QCOMPARE(r->value(0), QVariant(false));
    delete r;
}

void tst_QSparqlTrackerDirectSync::insert_and_delete_contact_sync()
{
    // This test will leave unclean test data in tracker if it crashes.
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlQuery add("insert { <addeduri001> a nco:PersonContact; "
                     "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                     "nco:nameGiven \"addedname001\" .}",
                     QSparqlQuery::InsertStatement);

    // Update is really done in syncExec; no need to call any other functions of
    // r.
    QSparqlResult* r = conn.syncExec(add);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    QVERIFY(r->isFinished()); // update is immediately finished
    delete r;

    // Verify that the insertion succeeded
    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");
    QHash<QString, QString> contactNames;
    r = conn.syncExec(q);
    QVERIFY(r != 0);
    QVERIFY(!r->isFinished());
    QCOMPARE(r->hasError(), false);

    // No size information
    QCOMPARE(r->size(), -1);
    while (r->next()) {
        contactNames[r->value(0).toString()] =
            r->value(1).toString();
    }
    QVERIFY(r->isFinished());
    QCOMPARE(contactNames.size(), 4);
    QCOMPARE(contactNames["addeduri001"], QString("addedname001"));
    delete r;

    // Delete the uri
    QSparqlQuery del("delete { <addeduri001> a rdfs:Resource. }",
                     QSparqlQuery::DeleteStatement);

    r = conn.syncExec(del);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    QVERIFY(r->isFinished());
    delete r;

    // Verify that it got deleted
    contactNames.clear();
    r = conn.syncExec(q);
    QVERIFY(r != 0);
    // No size information
    QCOMPARE(r->size(), -1);
    QCOMPARE(r->hasError(), false);
    QVERIFY(!r->isFinished());
    while (r->next()) {
        // A different way for retrieving the results
        contactNames[r->binding(0).value().toString()] =
            r->binding(1).value().toString();
    }
    QVERIFY(r->isFinished());
    QCOMPARE(contactNames.size(), 3);
    delete r;
}

void tst_QSparqlTrackerDirectSync::query_with_error_sync()
{
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlQuery q("this is not a valid query");
    QSparqlResult* r = conn.syncExec(q);
    QVERIFY(r != 0);
    QVERIFY(r->isFinished());
    QCOMPARE(r->hasError(), true);
    QCOMPARE(r->lastError().type(), QSparqlError::StatementError);
    delete r;
}

void tst_QSparqlTrackerDirectSync::iterate_result_sync()
{
    // This test will print out warnings
    testLogLevel = QtCriticalMsg;
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");
    QSparqlResult* r = conn.syncExec(q);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    QVERIFY(!r->isFinished());
    QCOMPARE(r->size(), -1);

    QVERIFY(r->pos() == QSparql::BeforeFirstRow);
    // This is not a valid position
    for (int i=-1; i <= 2; ++i) {
        QCOMPARE(r->binding(i), QSparqlBinding());
        QVERIFY(r->value(i).isNull());
    }
    QCOMPARE(r->current(), QSparqlResultRow());

    for (int i=0; i<3; ++i) {
        QVERIFY(r->next());
        QCOMPARE(r->pos(), i);

        QVERIFY(r->binding(-1).value().isNull());
        QVERIFY(r->binding(0).value().isNull() == false);
        QVERIFY(r->binding(1).value().isNull() == false);
        QVERIFY(r->binding(2).value().isNull());

        QVERIFY(r->value(-1).isNull());
        QVERIFY(r->value(0).isNull() == false);
        QVERIFY(r->value(1).isNull() == false);
        QVERIFY(r->value(2).isNull());
    }
    QVERIFY(!r->next());
    QVERIFY(r->pos() == QSparql::AfterLastRow);
    // This is not a valid position
    for (int i=-1; i <= 2; ++i) {
        QCOMPARE(r->binding(i), QSparqlBinding());
        QVERIFY(r->value(i).isNull());
    }
    QCOMPARE(r->current(), QSparqlResultRow());
    QVERIFY(r->isFinished());

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
    QCOMPARE(r->hasError(), false);
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
    QCOMPARE(r->hasError(), false);
    QVERIFY(r->next());
    QVERIFY(r->value(0).toBool() == true);
    QVERIFY(r->value(0).type() == QVariant::Bool);
    delete r;
    r = conn.syncExec(QSparqlQuery("select 0 > 1 { }"));
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    QVERIFY(r->next());
    QVERIFY(r->value(0).toBool() == false);
    QVERIFY(r->value(0).type() == QVariant::Bool);
    delete r;

    // Another type of boolean result
    r = conn.syncExec(QSparqlQuery("select ?b { <uri004> tracker:available ?b . }"));
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    QVERIFY(r->next());
    QVERIFY(r->value(0).toBool() == true);
    QVERIFY(r->value(0).type() == QVariant::Bool);
    delete r;

    r = conn.syncExec(QSparqlQuery("select ?b { <uri005> tracker:available ?b . }"));
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
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
    QCOMPARE(r1->hasError(), false);
    QVERIFY(!r1->isFinished());

    QSparqlResult* r2 = conn.syncExec(q);
    QVERIFY(r2 != 0);
    QCOMPARE(r2->hasError(), false);
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

void tst_QSparqlTrackerDirectSync::special_chars()
{
    // This test will leave unclean test data in tracker if it crashes.
    QSparqlConnection conn("QTRACKER_DIRECT");
    QString withSpecialChars("foo\u2780\u2781\u2782");
    QSparqlQuery add(QString("insert { <addeduri002> a nco:PersonContact; "
                             "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                             "nco:nameGiven \"%1\" .}").arg(withSpecialChars),
                             QSparqlQuery::InsertStatement);

    QSparqlResult* r = conn.syncExec(add);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    QVERIFY(r->isFinished());
    if (r->hasError()) {
        qDebug() << r->lastError().message();
    }
    QCOMPARE(r->hasError(), false);
    delete r;

    // Verify that the insertion succeeded
    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");
    QHash<QString, QString> contactNames;
    r = conn.syncExec(q);
    QVERIFY(r != 0);
    QVERIFY(!r->isFinished());
    QCOMPARE(r->size(), -1);
    while (r->next()) {
        contactNames[r->value(0).toString()] =
            r->value(1).toString();
    }
    QVERIFY(r->isFinished());
    QCOMPARE(contactNames.size(), 4);
    QCOMPARE(contactNames["addeduri002"], withSpecialChars);
    delete r;

    // Delete the uri
    QSparqlQuery del("delete { <addeduri002> a rdfs:Resource. }",
                     QSparqlQuery::DeleteStatement);

    r = conn.syncExec(del);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    delete r;
}

QTEST_MAIN( tst_QSparqlTrackerDirectSync )
#include "tst_qsparql_tracker_direct_sync.moc"
