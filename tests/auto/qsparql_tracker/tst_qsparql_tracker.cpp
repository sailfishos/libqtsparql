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

#include <QtTest/QtTest>
#include <QtSparql/QtSparql>

class tst_QSparqlTracker : public QObject
{
    Q_OBJECT

public:
    tst_QSparqlTracker();
    virtual ~tst_QSparqlTracker();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void insert_new_urn();

    void batch_update();

    void iterate_result();

    void delete_unfinished_result();

    void go_beyond_columns_number();
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

tst_QSparqlTracker::tst_QSparqlTracker()
{
}

tst_QSparqlTracker::~tst_QSparqlTracker()
{
}

void tst_QSparqlTracker::initTestCase()
{
    // For running the test without installing the plugins. Should work in
    // normal and vpath builds.
    QCoreApplication::addLibraryPath("../../../plugins");
    qInstallMsgHandler(myMessageOutput);

    // clean any remainings
    cleanupTestCase();

    const QString insertQueryTemplate =
        "<uri00%1> a nco:PersonContact, nie:InformationElement ;"
        "nie:isLogicalPartOf <qsparql-tracker-tests> ;"
        "nco:nameGiven \"name00%1\" .";
    QString insertQuery = "insert { <qsparql-tracker-tests> a nie:InformationElement .";
    for (int item = 1; item <= 3; item++) {
        insertQuery.append( insertQueryTemplate.arg(item) );
    }
    insertQuery.append(" }");

    QSparqlConnection conn("QTRACKER");
    QSparqlQuery q(insertQuery,
                   QSparqlQuery::InsertStatement);
    QSparqlResult* r = conn.exec(q);
    CHECK_QSPARQL_RESULT(r);
    r->waitForFinished();
    CHECK_QSPARQL_RESULT(r);
    delete r;
}

void tst_QSparqlTracker::cleanupTestCase()
{
    QSparqlConnection conn("QTRACKER");
    QSparqlQuery q("DELETE { ?u a rdfs:Resource . } "
                    "WHERE { ?u nie:isLogicalPartOf <qsparql-tracker-tests> . }"
                    "DELETE { <qsparql-tracker-tests> a rdfs:Resource . }",
                    QSparqlQuery::DeleteStatement);
    QSparqlResult* r = conn.exec(q);
    CHECK_QSPARQL_RESULT(r);
    r->waitForFinished();
    CHECK_QSPARQL_RESULT(r);
    delete r;
}

void tst_QSparqlTracker::init()
{
    testLogLevel = QtDebugMsg;
}

void tst_QSparqlTracker::cleanup()
{
}

void tst_QSparqlTracker::insert_new_urn()
{
    // This test will leave unclean test data in tracker if it crashes.
    QSparqlConnection conn("QTRACKER");
    QSparqlQuery add("insert { ?:addeduri a nco:PersonContact; "
                     "nie:isLogicalPartOf <qsparql-tracker-tests> ;"
                     "nco:nameGiven \"addedname006\" .}",
                     QSparqlQuery::InsertStatement);
    add.bindValue(conn.createUrn("addeduri"));
    QSparqlResult* r = conn.exec(add);
    CHECK_QSPARQL_RESULT(r);
    r->waitForFinished(); // this test is synchronous only
    CHECK_QSPARQL_RESULT(r);
    delete r;

    // Verify that the insertion succeeded
    QSparqlQuery q("select ?addeduri ?ng {?addeduri a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-tests> ;"
                   "nco:nameGiven ?ng .}");
    QHash<QString, QSparqlBinding> contactNames;
    r = conn.exec(q);
    CHECK_QSPARQL_RESULT(r);
    r->waitForFinished();
    CHECK_QSPARQL_RESULT(r);
    QCOMPARE(r->size(), 4);
    while (r->next()) {
        contactNames[r->binding(1).value().toString()] = r->binding(0);
    }
    QCOMPARE(contactNames.size(), 4);
    // We can only compare the first 9 chars because the rest is a new uuid string
    QCOMPARE(contactNames["addedname006"].value().toString().mid(0, 9), QString("urn:uuid:"));
    delete r;

    // Delete the uri.
    QSparqlQuery del("delete { ?:addeduri a rdfs:Resource. }",
                     QSparqlQuery::DeleteStatement);

    // Note that the tracker driver only returns strings, and so we can't just
    // substitute the binding returned by the select query into the delete query,
    // and need to do a bit of hackery here
    del.bindValue("addeduri", QUrl(contactNames["addedname006"].value().toString()));
    r = conn.exec(del);
    CHECK_QSPARQL_RESULT(r);
    r->waitForFinished(); // this test is synchronous only
    CHECK_QSPARQL_RESULT(r);
    delete r;

    // Verify that it got deleted
    contactNames.clear();
    r = conn.exec(q);
    QVERIFY(r != 0);
    r->waitForFinished();
    QCOMPARE(r->size(), 3);
    while (r->next()) {
        contactNames[r->binding(1).value().toString()] = r->binding(0);
    }
    QCOMPARE(contactNames.size(), 3);
    delete r;
}

void tst_QSparqlTracker::batch_update()
{
    // The "batch" option has been removed from API documentation as it is replaced by
    // QSparqlQueryOption::LowPriority. The implemenation and this test needs to be kept
    // for backward compatibility.
    QSparqlConnectionOptions opts;
    opts.setOption(QString::fromLatin1("batch"), QVariant(true));
    // This test will leave unclean test data into tracker if it crashes.
    QSparqlConnection conn("QTRACKER", opts);
    QSparqlQuery add("insert { <addeduri002> a nco:PersonContact; "
                     "nie:isLogicalPartOf <qsparql-tracker-tests> ;"
                     "nco:nameGiven \"addedname002\" .}",
                     QSparqlQuery::InsertStatement);

    QSparqlResult* r = conn.exec(add);
    CHECK_QSPARQL_RESULT(r);
    r->waitForFinished(); // this test is syncronous only
    CHECK_QSPARQL_RESULT(r);
    delete r;

    // Verify that the insertion succeeded
    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-tests> ;"
                   "nco:nameGiven ?ng .}");
    QHash<QString, QString> contactNames;
    r = conn.exec(q);
    CHECK_QSPARQL_RESULT(r);
    r->waitForFinished();
    CHECK_QSPARQL_RESULT(r);
    QCOMPARE(r->size(), 4);
    while (r->next()) {
        contactNames[r->value(0).toString()] = r->value(1).toString();
    }
    QCOMPARE(contactNames.size(), 4);
    QCOMPARE(contactNames["addeduri002"], QString("addedname002"));
    delete r;

    // Delete the uri
    QSparqlQuery del("delete { <addeduri002> a rdfs:Resource. }",
                     QSparqlQuery::DeleteStatement);

    r = conn.exec(del);
    CHECK_QSPARQL_RESULT(r);
    r->waitForFinished(); // this test is syncronous only
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
        contactNames[r->value(0).toString()] = r->value(1).toString();
    }
    QCOMPARE(contactNames.size(), 3);
    delete r;
}

void tst_QSparqlTracker::iterate_result()
{
    // This test will print out warnings
    testLogLevel = QtCriticalMsg;
    QSparqlConnection conn("QTRACKER");
    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-tests> ;"
                   "nco:nameGiven ?ng .}");
    QSparqlResult* r = conn.exec(q);
    CHECK_QSPARQL_RESULT(r);
    r->waitForFinished(); // this test is syncronous only
    CHECK_QSPARQL_RESULT(r);
    QCOMPARE(r->size(), 3);

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

    delete r;
}

void tst_QSparqlTracker::delete_unfinished_result()
{
    QSparqlConnection conn("QTRACKER");
    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-tests> ;"
                   "nco:nameGiven ?ng .}");
    QSparqlResult* r = conn.exec(q);
    CHECK_QSPARQL_RESULT(r);
    delete r;
    QTest::qWait(1000);
}

void tst_QSparqlTracker::go_beyond_columns_number()
{
    // This test will print out warnings
    testLogLevel = QtCriticalMsg;
    QSparqlConnection conn("QTRACKER");
    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-tests> ;"
                   "nco:nameGiven ?ng .}");
    QSparqlResult* r = conn.exec(q);
    CHECK_QSPARQL_RESULT(r);
    r->waitForFinished(); // this test is syncronous only
    CHECK_QSPARQL_RESULT(r);
    QCOMPARE(r->size(), 3);
    while (r->next()) {
        QCOMPARE(r->current().count(), 2);
        QCOMPARE(r->value(5).toString(), QString());
        QCOMPARE(r->binding(5).toString(), QString());
    }
    delete r;
}

QTEST_MAIN( tst_QSparqlTracker )
#include "tst_qsparql_tracker.moc"
