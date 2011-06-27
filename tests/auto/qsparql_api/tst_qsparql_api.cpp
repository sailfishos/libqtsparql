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

#include <QtTest/QtTest>
#include <QtSparql/QtSparql>

#include "../messagerecorder.h"
#include "../testhelpers.h"

// define the amount of data we are going to insert for the tracker tests
#define NUM_TRACKER_INSERTS 15

class tst_QSparqlAPI : public QObject
{
    Q_OBJECT

public:
    tst_QSparqlAPI();
    ~tst_QSparqlAPI();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void query_test();
    void query_test_data();

    void query_destroy_connection_after_finished_test();
    void query_destroy_connection_after_finished_test_data();

    void query_error_test();
    void query_error_test_data();

    void query_destroy_connection_test();
    void query_destroy_connection_test_data();

    void update_query_test();
    void update_query_test_data();

    void update_query_error_test();
    void update_query_error_test_data();

    void update_query_destroy_connection_test();
    void update_query_destroy_connection_test_data();

    void ask_query_test();
    void ask_query_test_data();

    void isFinished_test();
    void isFinished_test_data();

    void result_iteration_test();
    void result_iteration_test_data();

    void queryModel_test();
    void queryModel_test_data();

    void syncExec_waitForFinished_query_test();
    void syncExec_waitForFinished_query_test_data();

    void syncExec_waitForFinished_update_query_test();
    void syncExec_waitForFinished_update_query_test_data();

private:
    void insertTrackerTestData();
    void cleanupTrackerTestData();

private:
    MessageRecorder *msgRecorder;
};

namespace {

const QString contactSelectQuery =
    "select ?u ?ng {"
    "    ?u a nco:PersonContact; "
    "    nie:isLogicalPartOf <qsparql-api-tests> ;"
    "    nco:nameGiven ?ng .}";

const QString contactSelectNothingQuery =
    "select ?u ?ng {"
    "    ?u a nco:PersonContact; "
    "    nie:isLogicalPartOf <qsparql-api-tests-not-here> ;"
    "    nco:nameGiven ?ng .}";

const int contactSelectColumnCount = 2;

const QString contactInsertQueryTemplate =
    "<uri00%1> a nco:PersonContact; "
    "nie:isLogicalPartOf <qsparql-api-tests> ;"
    "nco:nameGiven \"name00%1\" .";

const int contactInsertAmount = 1;

const QString contactDeleteQueryTemplate =
    "<uri00%1> a rdfs:Resource .";

const int contactDeleteAmount = 1;

const QString askQueryTrue =
    "ask {"
    "    <uri001> a nco:PersonContact, nie:InformationElement ;"
    "    nie:isLogicalPartOf <qsparql-api-tests> ;"
    "    nco:nameGiven \"name001\" .}";

const QString askQueryFalse =
    "ask {"
    "    <uri001> a nco:PersonContact, nie:InformationElement ;"
    "    nie:isLogicalPartOf <qsparql-api-tests> ;"
    "    nco:nameGiven \"name002\" .}";

const QString constructQuery =
    "CONSTRUCT { ?s ?p ?o }"
    "    WHERE { GRAPH <http://example.org/aGraph> { ?s ?p ?o } . }";

class FinishedSignalReceiver : public QObject
{
    Q_OBJECT
    bool finished;

public:
    FinishedSignalReceiver() : finished(false)
    { }

    void waitForFinished()
    {
        while (!finished) {
            QTest::qWait(100);
        }
    }

public slots:
    void onFinished() { finished = true; }
};

void checkExecutionMethod(QSparqlResult* r, const int executionMethod, const bool useAsyncObject)
{
    QVERIFY(r);
    if (executionMethod == QSparqlQueryOptions::AsyncExec) {
        if (useAsyncObject) {
            // As per documentation requirement, only attempt to connect the
            // signals after first validating that there is no error
            if (!r->hasError()) {
                FinishedSignalReceiver signalObject;
                QObject::connect(r, SIGNAL(finished()), &signalObject, SLOT(onFinished()));
                signalObject.waitForFinished();
            }
        } else {
            r->waitForFinished();
        }
    }
}

void validateResults(QSparqlResult *r, const int expectedResultsSize)
{
    if (r->hasFeature(QSparqlResult::QuerySize))
        QCOMPARE(r->size(), expectedResultsSize);

    QHash<QString, QString> contactNamesValue;
    QHash<QString, QString> contactNamesBindings;
    QHash<QString, QString> contactNamesStringValue;

    QVERIFY(r->pos() == QSparql::BeforeFirstRow);
    int resultSize = 0;
    while (r->next()) {
        contactNamesValue[r->value(0).toString()] = r->value(1).toString();
        contactNamesBindings[r->binding(0).value().toString()] = r->binding(1).value().toString();
        contactNamesStringValue[r->stringValue(0)] = r->stringValue(1);
        ++resultSize;
    }
    QVERIFY(r->pos() == QSparql::AfterLastRow);
    QCOMPARE(resultSize, expectedResultsSize);

    // Checks for if we expect some results
    if (expectedResultsSize != 0) {
        for(int i=1; i<=expectedResultsSize; i++)
        {
            QCOMPARE(contactNamesValue[QString("uri00%1").arg(i)], QString("name00%1").arg(i));
            QCOMPARE(contactNamesValue[QString("uri00%1").arg(i)], contactNamesBindings[QString("uri00%1").arg(i)]);
            QCOMPARE(contactNamesValue[QString("uri00%1").arg(i)], contactNamesStringValue[QString("uri00%1").arg(i)]);
        }

        if (r->hasFeature(QSparqlResult::ForwardOnly)) {
            QVERIFY(!r->previous());
            QVERIFY(!r->next());
            QVERIFY(r->pos() == QSparql::AfterLastRow);
        } else {
            // Iterate the result backwards
            int resultSize = 0;
            while (r->previous()) {
                QVERIFY(r->pos() != QSparql::AfterLastRow);
                QCOMPARE(contactNamesValue[r->value(0).toString()], r->value(1).toString());
                QCOMPARE(contactNamesBindings[r->binding(0).value().toString()], r->binding(1).value().toString());
                QCOMPARE(contactNamesStringValue[r->stringValue(0)], r->stringValue(1));
                ++resultSize;
            }
            QCOMPARE(resultSize, expectedResultsSize);
            // Now move forward one...
            QVERIFY(r->pos() == QSparql::BeforeFirstRow);
            QVERIFY(r->next());
            QVERIFY(r->pos() != QSparql::BeforeFirstRow);
            QCOMPARE(contactNamesValue["uri001"], r->value(1).toString());
            QCOMPARE(contactNamesBindings["uri001"], r->binding(1).value().toString());
            QCOMPARE(contactNamesStringValue["uri001"], r->stringValue(1));
        }
    } else {
        //Make sure using the result doesn't crash
        QVERIFY(r->pos() < 0);
        QVERIFY(!r->next());
        QVERIFY(!r->previous());
        QVERIFY(r->pos() < 0);
    }
}

void validateErrorResult(QSparqlResult *r, int expectedErrorType)
{
    QVERIFY(r);
    QVERIFY(r->hasError());
    QVERIFY(r->lastError().type() == expectedErrorType);

    // Check iteration behaviour
    QVERIFY(r->pos() < 0);
    QVERIFY(!r->next());
    QVERIFY(!r->previous());
    QVERIFY(r->pos() < 0);
}

} // end unnamed namespace

tst_QSparqlAPI::tst_QSparqlAPI()
{
}

tst_QSparqlAPI::~tst_QSparqlAPI()
{
}

void tst_QSparqlAPI::initTestCase()
{
    // For running the test without installing the plugins. Should work in
    // normal and vpath builds.
    QCoreApplication::addLibraryPath("../../../plugins");

    cleanupTestCase();
    insertTrackerTestData();
}

void tst_QSparqlAPI::insertTrackerTestData()
{
    const QString insertQueryTemplate =
        "<uri00%1> a nco:PersonContact, nie:InformationElement ;"
        "nie:isLogicalPartOf <qsparql-api-tests> ;"
        "nco:nameGiven \"name00%1\" .";
    QString insertQuery = "insert { <qsparql-api-tests> a nie:InformationElement .";
    for (int item = 1; item <= NUM_TRACKER_INSERTS; item++) {
        insertQuery.append( insertQueryTemplate.arg(item) );
    }
    insertQuery.append(" }");

    QSparqlConnection conn("QTRACKER");
    const QSparqlQuery q(insertQuery,QSparqlQuery::InsertStatement);
    QSparqlResult* r = conn.exec(q);
    QVERIFY(!r->hasError());
    r->waitForFinished();
    QVERIFY(!r->hasError());
    delete r;
}

void tst_QSparqlAPI::cleanupTestCase()
{
    cleanupTrackerTestData();
}

void tst_QSparqlAPI::cleanupTrackerTestData()
{
    QSparqlConnection conn("QTRACKER");
    const QSparqlQuery q("DELETE { ?u a rdfs:Resource . } "
                         "  WHERE { ?u nie:isLogicalPartOf <qsparql-api-tests> . }"
                         "DELETE { <qsparql-api-tests> a rdfs:Resource . }",
                    QSparqlQuery::DeleteStatement);
    QSparqlResult* r = conn.exec(q);
    QVERIFY(!r->hasError());
    r->waitForFinished();
    QVERIFY(!r->hasError());
    delete r;
}

void tst_QSparqlAPI::init()
{
    msgRecorder = new MessageRecorder;
    msgRecorder->addMsgTypeToRecord(QtWarningMsg);
}

void tst_QSparqlAPI::cleanup()
{
    delete msgRecorder;
    msgRecorder = 0;
}

void tst_QSparqlAPI::query_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, query);
    QFETCH(int, expectedResultsSize);
    QFETCH(int, executionMethod);
    QFETCH(bool, useAsyncObject);

    QSparqlConnection conn(connectionDriver);

    QSparqlQueryOptions queryOptions;
    queryOptions.setExecutionMethod(QSparqlQueryOptions::ExecutionMethod(executionMethod));
    const QSparqlQuery q(query);

    QSparqlResult* r = conn.exec(q, queryOptions);
    checkExecutionMethod(r, executionMethod, useAsyncObject);
    validateResults(r, expectedResultsSize);

    delete r;
}

void tst_QSparqlAPI::query_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<QString>("query");
    QTest::addColumn<int>("expectedResultsSize");
    QTest::addColumn<int>("executionMethod");
    QTest::addColumn<bool>("useAsyncObject");

    QTest::newRow("DBus Async Query")
        << "QTRACKER"
        << contactSelectQuery
        << NUM_TRACKER_INSERTS
        << int(QSparqlQueryOptions::AsyncExec)
        << false;

    QTest::newRow("DBus Async No Results Query")
        << "QTRACKER"
        << contactSelectNothingQuery
        << 0
        << int(QSparqlQueryOptions::AsyncExec)
        << false;

    QTest::newRow("DBus Sync Query")
        << "QTRACKER"
        << contactSelectQuery
        << NUM_TRACKER_INSERTS
        << int(QSparqlQueryOptions::SyncExec)
        << false;

    QTest::newRow("DBus Sync No Results Query")
        << "QTRACKER"
        << contactSelectNothingQuery
        << 0
        << int(QSparqlQueryOptions::SyncExec)
        << false;

    QTest::newRow("DBus Async Object Query")
        << "QTRACKER"
        << contactSelectQuery
        << NUM_TRACKER_INSERTS
        << int(QSparqlQueryOptions::AsyncExec)
        << true;

    QTest::newRow("DBus Async Object No Results Query")
        << "QTRACKER"
        << contactSelectNothingQuery
        << 0
        << int(QSparqlQueryOptions::AsyncExec)
        << true;

    QTest::newRow("Tracker Direct Async Query")
        << "QTRACKER_DIRECT"
        << contactSelectQuery
        << NUM_TRACKER_INSERTS
        << int(QSparqlQueryOptions::AsyncExec)
        << false;

    QTest::newRow("Tracker Direct Async No Results Query")
        << "QTRACKER_DIRECT"
        << contactSelectNothingQuery
        << 0
        << int(QSparqlQueryOptions::AsyncExec)
        << false;

    QTest::newRow("Tracker Direct Async Object Query")
        << "QTRACKER_DIRECT"
        << contactSelectQuery
        << NUM_TRACKER_INSERTS
        << int(QSparqlQueryOptions::AsyncExec)
        << true;

    QTest::newRow("Tracker Direct Async Object No Results Query")
        << "QTRACKER_DIRECT"
        << contactSelectNothingQuery
        << 0
        << int(QSparqlQueryOptions::AsyncExec)
        << true;

    QTest::newRow("Tracker Direct Sync Query")
        << "QTRACKER_DIRECT"
        << contactSelectQuery
        << NUM_TRACKER_INSERTS
        << int(QSparqlQueryOptions::SyncExec)
        << false;

    QTest::newRow("Tracker Direct Sync No Results Query")
        << "QTRACKER_DIRECT"
        << contactSelectNothingQuery
        << 0
        << int(QSparqlQueryOptions::SyncExec)
        << false;

}

void tst_QSparqlAPI::query_destroy_connection_after_finished_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, query);
    QFETCH(int, expectedResultsSize);
    QFETCH(int, executionMethod);
    QFETCH(bool, useAsyncObject);

    QSparqlConnection* conn = new QSparqlConnection(connectionDriver);

    QSparqlQueryOptions queryOptions;
    queryOptions.setExecutionMethod(QSparqlQueryOptions::ExecutionMethod(executionMethod));
    const QSparqlQuery q(query);

    QSparqlResult* r = conn->exec(q, queryOptions);

    // Re-parent the result to release from the ownership of conn
    r->setParent(this);

    checkExecutionMethod(r, executionMethod, useAsyncObject);
    if (r->isFinished())
    {
        // Result is finished after wait: destroy connection and validate result
        delete conn; conn = 0;
    }

    // Check that result is not in error and valid
    QVERIFY( !r->hasError() );
    validateResults(r, expectedResultsSize);

    QVERIFY( r->isFinished() );
    // Destroying connection after validation must not raise an error in the result
    delete conn; conn = 0;
    QVERIFY( !r->hasError() );

    delete r;
}

void tst_QSparqlAPI::query_destroy_connection_after_finished_test_data()
{
    // Reuse test data from query_test
    query_test_data();
}

void tst_QSparqlAPI::query_error_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, query);
    QFETCH(int, executionMethod);
    QFETCH(int, expectedErrorType);
    QFETCH(bool, useAsyncObject);

    QSparqlConnection conn(connectionDriver);

    QSparqlQueryOptions queryOptions;
    queryOptions.setExecutionMethod(QSparqlQueryOptions::ExecutionMethod(executionMethod));
    const QSparqlQuery q(query);

    QSparqlResult* r = conn.exec(q, queryOptions);

    checkExecutionMethod(r, executionMethod, useAsyncObject);
    validateErrorResult(r, expectedErrorType);
    QCOMPARE((*msgRecorder)[QtWarningMsg].count(), 1);

    delete r;
}

void tst_QSparqlAPI::query_error_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<QString>("query");
    QTest::addColumn<int>("executionMethod");
    QTest::addColumn<int>("expectedErrorType");
    QTest::addColumn<bool>("useAsyncObject");

    QTest::newRow("DBus Blank Query")
        << "QTRACKER"
        << ""
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << false;

   QTest::newRow("DBus Blank Async Object Query")
        << "QTRACKER"
        << ""
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << true;

    QTest::newRow("DBus Invalid Query")
        << "QTRACKER"
        << "invalid query"
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << false;

    QTest::newRow("DBus Invalid Async Object Query")
        << "QTRACKER"
        << "invalid query"
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << true;

    QTest::newRow("DBus Invalid Construct Query")
        << "QTRACKER"
        << constructQuery
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::BackendError)
        << false;

    QTest::newRow("DBus Invalid Construct Async Object Query")
        << "QTRACKER"
        << constructQuery
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::BackendError)
        << true;

    QTest::newRow("Tracker Direct Async Blank Query")
        << "QTRACKER_DIRECT"
        << ""
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << false;

    QTest::newRow("Tracker Direct Async Object Blank Query")
        << "QTRACKER_DIRECT"
        << ""
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << true;

    QTest::newRow("Tracker Direct Sync Blank Query")
        << "QTRACKER_DIRECT"
        << ""
        << int(QSparqlQueryOptions::SyncExec)
        << int(QSparqlError::StatementError)
        << false;

    QTest::newRow("Tracker Direct Async Invalid Query")
        << "QTRACKER_DIRECT"
        << "invalid query"
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << false;

    QTest::newRow("Tracker Direct Async Object Invalid Query")
        << "QTRACKER_DIRECT"
        << "invalid query"
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << true;

    QTest::newRow("Tracker Direct Sync Invalid Query")
        << "QTRACKER_DIRECT"
        << "invalid query"
        << int(QSparqlQueryOptions::SyncExec)
        << int(QSparqlError::StatementError)
        << false;

    QTest::newRow("Tracker Direct Async Construct Query")
        << "QTRACKER_DIRECT"
        << constructQuery
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::BackendError)
        << false;

    QTest::newRow("Tracker Direct Async Object Construct Query")
        << "QTRACKER_DIRECT"
        << constructQuery
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::BackendError)
        << true;

    QTest::newRow("Tracker Direct Sync Construct Query")
        << "QTRACKER_DIRECT"
        << constructQuery
        << int(QSparqlQueryOptions::SyncExec)
        << int(QSparqlError::BackendError)
        << false;
}

void tst_QSparqlAPI::query_destroy_connection_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, query);
    QFETCH(int, expectedResultsSize);
    QFETCH(int, executionMethod);
    QFETCH(bool, useAsyncObject);

    QSparqlConnection* conn = new QSparqlConnection(connectionDriver);

    QSparqlQueryOptions queryOptions;
    queryOptions.setExecutionMethod(QSparqlQueryOptions::ExecutionMethod(executionMethod));
    const QSparqlQuery q(query);

    QSparqlResult* r = conn->exec(q, queryOptions);
    const bool immediatelyFinished = r->isFinished();

    // Re-parent the result to release it from the connection's ownership
    r->setParent(this);
    delete conn; conn = 0;

    if (immediatelyFinished) {
        // If the query completes immediately, before the connection is
        // destroyed, it should not be in error and should contain valid data
        QVERIFY(!r->hasError());
        checkExecutionMethod(r, executionMethod, useAsyncObject);
        validateResults(r, expectedResultsSize);
    }
    else {
        // If the result was not finished immediately after exec, it should be in
        // error after connection close
        QVERIFY(r->hasError());
        QVERIFY(r->lastError().type() == QSparqlError::ConnectionError);

        checkExecutionMethod(r, executionMethod, useAsyncObject);
        QVERIFY(r->hasError());
        QVERIFY(r->lastError().type() == QSparqlError::ConnectionError);

        validateResults(r, 0);
    }

    // There must always be a warning about connection closed before result
    QCOMPARE((*msgRecorder)[QtWarningMsg].count(), 1);

    delete r;
}

void tst_QSparqlAPI::query_destroy_connection_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<QString>("query");
    QTest::addColumn<int>("expectedResultsSize");
    QTest::addColumn<int>("executionMethod");
    QTest::addColumn<bool>("useAsyncObject");

    QTest::newRow("DBus Async Query")
        << "QTRACKER"
        << contactSelectQuery
        << NUM_TRACKER_INSERTS
        << int(QSparqlQueryOptions::AsyncExec)
        << false;

    QTest::newRow("DBus Async Query")
        << "QTRACKER"
        << contactSelectQuery
        << NUM_TRACKER_INSERTS
        << int(QSparqlQueryOptions::AsyncExec)
        << true;

    QTest::newRow("DBus Sync Query")
        << "QTRACKER"
        << contactSelectQuery
        << NUM_TRACKER_INSERTS
        << int(QSparqlQueryOptions::SyncExec)
        << true;

    QTest::newRow("Tracker Direct Async Query")
        << "QTRACKER_DIRECT"
        << contactSelectQuery
        << NUM_TRACKER_INSERTS
        << int(QSparqlQueryOptions::AsyncExec)
        << false;

    QTest::newRow("Tracker Direct Async Query")
        << "QTRACKER_DIRECT"
        << contactSelectQuery
        << NUM_TRACKER_INSERTS
        << int(QSparqlQueryOptions::AsyncExec)
        << true;

    QTest::newRow("Tracker Direct Sync Query")
        << "QTRACKER_DIRECT"
        << contactSelectQuery
        << NUM_TRACKER_INSERTS
        << int(QSparqlQueryOptions::SyncExec)
        << false;
}

void tst_QSparqlAPI::update_query_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, insertTemplate);
    QFETCH(QString, deleteTemplate);
    QFETCH(QString, validateQuery);
    QFETCH(int, initialSize);
    QFETCH(int, contactInserts);
    QFETCH(int, contactDeletes);
    QFETCH(int, executionMethod);
    QFETCH(bool, useAsyncObject);

    QSparqlConnection conn(connectionDriver);
    const int expectedResultsSize = initialSize + contactInserts;

    QString insertQuery = "insert { <qsparql-api-tests> a nie:InformationElement .";
    for (int item = initialSize+1; item <= expectedResultsSize; item++) {
        insertQuery.append( insertTemplate.arg(item) );
    }
    insertQuery.append(" }");

    QSparqlQueryOptions queryOptions;
    queryOptions.setExecutionMethod(QSparqlQueryOptions::ExecutionMethod(executionMethod));
    const QSparqlQuery q(insertQuery, QSparqlQuery::InsertStatement);

    QSparqlResult* r = conn.exec(q, queryOptions);

    checkExecutionMethod(r, executionMethod, useAsyncObject);

    // Update query binding values will return empty bindings
    QCOMPARE(QString(""), r->binding(0).value().toString());
    QCOMPARE(QString(""), r->binding(1).value().toString());
    // Check the value() for the same thing
    QCOMPARE(QString(""), r->value(0).toString());
    // for updates, current() should return a empty result row
    QCOMPARE(QSparqlResultRow(), r->current());
    // and size should be 0
    if (r->hasFeature(QSparqlResult::QuerySize) )
        QCOMPARE(r->size(), 0);
    delete r;

    // Verify the insertion
    r = conn.exec(QSparqlQuery(validateQuery), queryOptions);
    checkExecutionMethod(r, executionMethod, useAsyncObject);
    validateResults(r, expectedResultsSize);
    delete r;

    // Delete the insertion
    QString deleteQuery = "delete { <qsparql-api-tests> a nie:InformationElement .";
    for (int item = initialSize+1; item <= expectedResultsSize; item++) {
        deleteQuery.append( deleteTemplate.arg(item) );
    }
    deleteQuery.append(" }");

    const QSparqlQuery delQuery(deleteQuery, QSparqlQuery::DeleteStatement);
    r = conn.exec(delQuery, queryOptions);
    checkExecutionMethod(r, executionMethod, useAsyncObject);
    delete r;

    // Now verify deletion
    r = conn.exec(QSparqlQuery(validateQuery), queryOptions);
    checkExecutionMethod(r, executionMethod, useAsyncObject);
    validateResults(r, expectedResultsSize - contactDeletes);
    delete r;
}

void tst_QSparqlAPI::update_query_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<QString>("insertTemplate");
    QTest::addColumn<QString>("deleteTemplate");
    QTest::addColumn<QString>("validateQuery");
    QTest::addColumn<int>("initialSize");
    QTest::addColumn<int>("contactInserts");
    QTest::addColumn<int>("contactDeletes");
    QTest::addColumn<int>("executionMethod");
    QTest::addColumn<bool>("useAsyncObject");

    QTest::newRow("DBus Update Query")
        << "QTRACKER"
        << contactInsertQueryTemplate
        << contactDeleteQueryTemplate
        << contactSelectQuery
        << NUM_TRACKER_INSERTS
        << contactInsertAmount
        << contactDeleteAmount
        << int(QSparqlQueryOptions::AsyncExec)
        << false;

    QTest::newRow("DBus Update Async Object Query")
        << "QTRACKER"
        << contactInsertQueryTemplate
        << contactDeleteQueryTemplate
        << contactSelectQuery
        << NUM_TRACKER_INSERTS
        << contactInsertAmount
        << contactDeleteAmount
        << int(QSparqlQueryOptions::AsyncExec)
        << true;

    QTest::newRow("Tracker Direct Async Update Query")
        << "QTRACKER_DIRECT"
        << contactInsertQueryTemplate
        << contactDeleteQueryTemplate
        << contactSelectQuery
        << NUM_TRACKER_INSERTS
        << contactInsertAmount
        << contactDeleteAmount
        << int(QSparqlQueryOptions::AsyncExec)
        << false;

    QTest::newRow("Tracker Direct Async Object Update Query")
        << "QTRACKER_DIRECT"
        << contactInsertQueryTemplate
        << contactDeleteQueryTemplate
        << contactSelectQuery
        << NUM_TRACKER_INSERTS
        << contactInsertAmount
        << contactDeleteAmount
        << int(QSparqlQueryOptions::AsyncExec)
        << true;

    QTest::newRow("Tracker Direct Sync Update Query")
        << "QTRACKER_DIRECT"
        << contactInsertQueryTemplate
        << contactDeleteQueryTemplate
        << contactSelectQuery
        << NUM_TRACKER_INSERTS
        << contactInsertAmount
        << contactDeleteAmount
        << int(QSparqlQueryOptions::SyncExec)
        << false;
}

void tst_QSparqlAPI::update_query_error_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, query);
    QFETCH(int, executionMethod);
    QFETCH(int, expectedErrorType);
    QFETCH(bool, useAsyncObject);

    QSparqlQuery::StatementType queryType = QSparqlQuery::InsertStatement;
    for (int round = 1; round <= 2; ++round) {
        QSparqlConnection conn(connectionDriver);
        QSparqlQueryOptions queryOptions;
        queryOptions.setExecutionMethod(QSparqlQueryOptions::ExecutionMethod(executionMethod));
        const QSparqlQuery q(query, queryType);

        QSparqlResult* r = conn.exec(q, queryOptions);

        checkExecutionMethod(r, executionMethod, useAsyncObject);
        validateErrorResult(r, expectedErrorType);
        QCOMPARE((*msgRecorder)[QtWarningMsg].count(), round);

        delete r;
        // also check delete statments
        queryType = QSparqlQuery::DeleteStatement;
    }
}

void tst_QSparqlAPI::update_query_error_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<QString>("query");
    QTest::addColumn<int>("executionMethod");
    QTest::addColumn<int>("expectedErrorType");
    QTest::addColumn<bool>("useAsyncObject");

    QTest::newRow("DBus Blank Query")
        << "QTRACKER"
        << ""
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << false;

   QTest::newRow("DBus Blank Async Object Query")
        << "QTRACKER"
        << ""
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << true;

    QTest::newRow("DBus Invalid Query")
        << "QTRACKER"
        << "invalid query"
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << false;

    QTest::newRow("DBus Invalid Async Object Query")
        << "QTRACKER"
        << "invalid query"
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << true;

    QTest::newRow("DBus Invalid Construct Query")
        << "QTRACKER"
        << constructQuery
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << false;

    QTest::newRow("DBus Invalid Construct Async Object Query")
        << "QTRACKER"
        << constructQuery
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << true;

    QTest::newRow("Tracker Direct Async Blank Query")
        << "QTRACKER_DIRECT"
        << ""
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << false;

    QTest::newRow("Tracker Direct Async Object Blank Query")
        << "QTRACKER_DIRECT"
        << ""
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << true;

    QTest::newRow("Tracker Direct Sync Blank Query")
        << "QTRACKER_DIRECT"
        << ""
        << int(QSparqlQueryOptions::SyncExec)
        << int(QSparqlError::StatementError)
        << false;

    QTest::newRow("Tracker Direct Async Invalid Query")
        << "QTRACKER_DIRECT"
        << "invalid query"
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << false;

    QTest::newRow("Tracker Direct Async Object Invalid Query")
        << "QTRACKER_DIRECT"
        << "invalid query"
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << true;

    QTest::newRow("Tracker Direct Sync Invalid Query")
        << "QTRACKER_DIRECT"
        << "invalid query"
        << int(QSparqlQueryOptions::SyncExec)
        << int(QSparqlError::StatementError)
        << false;

    QTest::newRow("Tracker Direct Async Construct Query")
        << "QTRACKER_DIRECT"
        << constructQuery
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << false;

    QTest::newRow("Tracker Direct Async Object Construct Query")
        << "QTRACKER_DIRECT"
        << constructQuery
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << true;

    QTest::newRow("Tracker Direct Sync Construct Query")
        << "QTRACKER_DIRECT"
        << constructQuery
        << int(QSparqlQueryOptions::SyncExec)
        << int(QSparqlError::StatementError)
        << false;
}

void tst_QSparqlAPI::update_query_destroy_connection_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, query);
    QFETCH(int, executionMethod);
    QFETCH(bool, useAsyncObject);
    QFETCH(QString, deleteQuery);

    QSparqlConnection cleanupConn("QTRACKER");
    QSparqlQuery::StatementType queryType = QSparqlQuery::InsertStatement;
    for (int round = 1; round <= 2; ++round) {
        QSparqlConnection* conn = new QSparqlConnection(connectionDriver);
        QSparqlQueryOptions queryOptions;
        queryOptions.setExecutionMethod(QSparqlQueryOptions::ExecutionMethod(executionMethod));
        const QSparqlQuery q(query, queryType);

        QSparqlResult* r = conn->exec(q, queryOptions);
        const bool immediatelyFinished = r->isFinished();
        // Re-parent the result to release it from the connection's ownership
        r->setParent(this);
        delete conn; conn = 0;

        checkExecutionMethod(r, executionMethod, useAsyncObject);

        if (!immediatelyFinished) {
            // If the result was not immediately finished after exec it should have an error
            QVERIFY(r->hasError());
            QVERIFY(r->lastError().type() == QSparqlError::ConnectionError);
        }
        validateResults(r, 0);
        // Check we got a warning
        QCOMPARE((*msgRecorder)[QtWarningMsg].count(), round);
        delete r;

        QSparqlResult* cleanupResult = cleanupConn.syncExec(
                QSparqlQuery(deleteQuery, QSparqlQuery::DeleteStatement));
        CHECK_QSPARQL_RESULT(cleanupResult);
        delete cleanupResult;

        // also check delete statments
        queryType = QSparqlQuery::DeleteStatement;
    }
}

void tst_QSparqlAPI::update_query_destroy_connection_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<QString>("query");
    QTest::addColumn<int>("executionMethod");
    QTest::addColumn<bool>("useAsyncObject");
    QTest::addColumn<QString>("deleteQuery");

    QString insertQuery = "insert { <qsparql-api-tests> a nie:InformationElement .";
    insertQuery.append(contactInsertQueryTemplate.arg(NUM_TRACKER_INSERTS+1));
    insertQuery.append(" }");

    QString deleteQuery = "delete { <qsparql-api-tests> a nie:InformationElement .";
    deleteQuery.append(contactDeleteQueryTemplate.arg(NUM_TRACKER_INSERTS+1));
    deleteQuery.append(" }");

    QTest::newRow("DBus Async Query")
        << "QTRACKER"
        << insertQuery
        << int(QSparqlQueryOptions::AsyncExec)
        << false
        << deleteQuery;

    QTest::newRow("DBus Async Object Query")
        << "QTRACKER"
        << insertQuery
        << int(QSparqlQueryOptions::AsyncExec)
        << true
        << deleteQuery;

    QTest::newRow("DBus Sync Query")
        << "QTRACKER"
        << insertQuery
        << int(QSparqlQueryOptions::SyncExec)
        << false
        << deleteQuery;

    QTest::newRow("Tracker Direct Async Query")
        << "QTRACKER_DIRECT"
        << insertQuery
        << int(QSparqlQueryOptions::AsyncExec)
        << false
        << deleteQuery;

    QTest::newRow("Tracker Direct Async Object Query")
        << "QTRACKER_DIRECT"
        << insertQuery
        << int(QSparqlQueryOptions::AsyncExec)
        << true
        << deleteQuery;

    QTest::newRow("Tracker Direct Sync Query")
        << "QTRACKER_DIRECT"
        << insertQuery
        << int(QSparqlQueryOptions::SyncExec)
        << false
        << deleteQuery;
}

void tst_QSparqlAPI::ask_query_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, query);
    QFETCH(int, executionMethod);
    QFETCH(bool, expectedResult);
    QFETCH(bool, useAsyncObject);

    QSparqlConnection conn(connectionDriver);
    QVERIFY(conn.hasFeature(QSparqlConnection::AskQueries));

    const QSparqlQuery q(query, QSparqlQuery::AskStatement);

    QSparqlQueryOptions queryOptions;
    queryOptions.setExecutionMethod(QSparqlQueryOptions::ExecutionMethod(executionMethod));

    QSparqlResult *r = conn.exec(q, queryOptions);

    checkExecutionMethod(r, executionMethod, useAsyncObject);

    while (!r->isFinished())
        r->next();

    QVERIFY(r->isFinished());
    QCOMPARE(r->boolValue(), expectedResult);

    delete r;
}

void tst_QSparqlAPI::ask_query_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<int>("executionMethod");
    QTest::addColumn<QString>("query");
    QTest::addColumn<bool>("expectedResult");
    QTest::addColumn<bool>("useAsyncObject");

    QTest::newRow("Tracker Direct Async True Query")
        << "QTRACKER_DIRECT"
        << int(QSparqlQueryOptions::AsyncExec)
        << askQueryTrue
        << true
        << false;

    QTest::newRow("Tracker Direct Async Object True Query")
        << "QTRACKER_DIRECT"
        << int(QSparqlQueryOptions::AsyncExec)
        << askQueryTrue
        << true
        << true;

    QTest::newRow("Tracker Direct Sync True Query")
        << "QTRACKER_DIRECT"
        << int(QSparqlQueryOptions::SyncExec)
        << askQueryTrue
        << true
        << false;

    QTest::newRow("Tracker Direct Async False Query")
        << "QTRACKER_DIRECT"
        << int(QSparqlQueryOptions::AsyncExec)
        << askQueryFalse
        << false
        << false;

    QTest::newRow("Tracker Direct Async Object False Query")
        << "QTRACKER_DIRECT"
        << int(QSparqlQueryOptions::AsyncExec)
        << askQueryFalse
        << false
        << true;

    QTest::newRow("Tracker Direct Sync False Query")
        << "QTRACKER_DIRECT"
        << int(QSparqlQueryOptions::SyncExec)
        << askQueryFalse
        << false
        << false;

    QTest::newRow("DBus Async True Ask Query")
        << "QTRACKER"
        << int(QSparqlQueryOptions::AsyncExec)
        << askQueryTrue
        << true
        << false;

    QTest::newRow("DBus Async True Ask Async Object Query")
        << "QTRACKER"
        << int(QSparqlQueryOptions::AsyncExec)
        << askQueryTrue
        << true
        << true;

    QTest::newRow("DBus Async False Ask Query")
        << "QTRACKER"
        << int(QSparqlQueryOptions::AsyncExec)
        << askQueryFalse
        << false
        << false;

    QTest::newRow("DBus Async False Ask Async Object Query")
        << "QTRACKER"
        << int(QSparqlQueryOptions::AsyncExec)
        << askQueryFalse
        << false
        << true;

}

void tst_QSparqlAPI::isFinished_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, query);
    QFETCH(int, expectedResultsSize);
    QFETCH(int, executionMethod);
    QFETCH(bool, useAsyncObject);

    QSparqlConnection conn(connectionDriver);

    QSparqlQueryOptions queryOptions;
    queryOptions.setExecutionMethod(QSparqlQueryOptions::ExecutionMethod(executionMethod));
    const QSparqlQuery q(query);

    QSparqlResult* r = conn.exec(q, queryOptions);

    checkExecutionMethod(r, executionMethod, useAsyncObject);

    // According to the documentation, isFinished() will be false for sync queries
    // until the results have been iterated
    if (executionMethod == QSparqlQueryOptions::SyncExec) {
        QVERIFY(!r->isFinished());
        validateResults(r, expectedResultsSize);
        QVERIFY(r->isFinished());
    } else {
        QVERIFY(r->isFinished());
        validateResults(r, expectedResultsSize);
    }

    delete r;
}

void tst_QSparqlAPI::isFinished_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<QString>("query");
    QTest::addColumn<int>("expectedResultsSize");
    QTest::addColumn<int>("executionMethod");
    QTest::addColumn<bool>("useAsyncObject");

    QTest::newRow("DBus Async Query")
        << "QTRACKER"
        << contactSelectQuery
        << NUM_TRACKER_INSERTS
        << int(QSparqlQueryOptions::AsyncExec)
        << false;

   QTest::newRow("DBus Async Object Query")
        << "QTRACKER"
        << contactSelectQuery
        << NUM_TRACKER_INSERTS
        << int(QSparqlQueryOptions::AsyncExec)
        << true;

    QTest::newRow("Tracker Direct Async Query")
        << "QTRACKER_DIRECT"
        << contactSelectQuery
        << NUM_TRACKER_INSERTS
        << int(QSparqlQueryOptions::AsyncExec)
        << false;

    QTest::newRow("Tracker Direct Async Object Query")
        << "QTRACKER_DIRECT"
        << contactSelectQuery
        << NUM_TRACKER_INSERTS
        << int(QSparqlQueryOptions::AsyncExec)
        << true;

    QTest::newRow("Tracker Direct Sync Query")
        << "QTRACKER_DIRECT"
        << contactSelectQuery
        << NUM_TRACKER_INSERTS
        << int(QSparqlQueryOptions::SyncExec)
        << false;
}

void tst_QSparqlAPI::result_iteration_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, query);
    QFETCH(int, expectedResultsSize);
    QFETCH(int, executionMethod);
    QFETCH(bool, useAsyncObject);

    QSparqlConnection conn(connectionDriver);

    QSparqlQueryOptions queryOptions;
    queryOptions.setExecutionMethod(QSparqlQueryOptions::ExecutionMethod(executionMethod));
    const QSparqlQuery q(query);

    QSparqlResult* r = conn.exec(q, queryOptions);

    checkExecutionMethod(r, executionMethod, useAsyncObject);

    validateResults(r, expectedResultsSize);
    delete r;
}

void tst_QSparqlAPI::result_iteration_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<QString>("query");
    QTest::addColumn<int>("expectedResultsSize");
    QTest::addColumn<int>("executionMethod");
    QTest::addColumn<bool>("useAsyncObject");

    QTest::newRow("DBus Async Query")
        << "QTRACKER"
        << contactSelectQuery
        << NUM_TRACKER_INSERTS
        << int(QSparqlQueryOptions::AsyncExec)
        << false;

    QTest::newRow("DBus Async Object Query")
        << "QTRACKER"
        << contactSelectQuery
        << NUM_TRACKER_INSERTS
        << int(QSparqlQueryOptions::AsyncExec)
        << true;

    QTest::newRow("Tracker Direct Async Query")
        << "QTRACKER_DIRECT"
        << contactSelectQuery
        << NUM_TRACKER_INSERTS
        << int(QSparqlQueryOptions::AsyncExec)
        << false;

    QTest::newRow("Tracker Direct Async Object Query")
        << "QTRACKER_DIRECT"
        << contactSelectQuery
        << NUM_TRACKER_INSERTS
        << int(QSparqlQueryOptions::AsyncExec)
        << true;

    QTest::newRow("Tracker Direct Sync Query")
        << "QTRACKER_DIRECT"
        << contactSelectQuery
        << NUM_TRACKER_INSERTS
        << int(QSparqlQueryOptions::SyncExec)
        << false;
}

void tst_QSparqlAPI::queryModel_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, query);
    QFETCH(int, expectedResultsSize);

    QSparqlConnection conn(connectionDriver);
    QSparqlQueryModel model;

    model.setQuery(QSparqlQuery(query), conn);

    QSignalSpy spy(&model, SIGNAL(finished()));
    while (spy.count() == 0)
        QTest::qWait(100);

    QCOMPARE(model.rowCount(), expectedResultsSize);
    QCOMPARE(model.columnCount(), contactSelectColumnCount);

    // Verify the data in the model
    for (int i=0;i<model.rowCount();i++) {
        QSparqlResultRow row = model.resultRow(i);
        QCOMPARE(QString("uri00%1").arg(i+1), row.value(0).toString());
        QCOMPARE(QString("name00%1").arg(i+1), row.value(1).toString());
    }
}

void tst_QSparqlAPI::queryModel_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<QString>("query");
    QTest::addColumn<int>("expectedResultsSize");

    QTest::newRow("DBus Query Model")
        << "QTRACKER"
        << contactSelectQuery
        << NUM_TRACKER_INSERTS;

    QTest::newRow("Tracker Direct Query Model")
        << "QTRACKER_DIRECT"
        << contactSelectQuery
        << NUM_TRACKER_INSERTS;
}

void tst_QSparqlAPI::syncExec_waitForFinished_query_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, query);
    QFETCH(int, expectedResultsSize);

    QSparqlConnection conn(connectionDriver);
    const QSparqlQuery q(query);
    QSparqlResult* r = conn.syncExec(q);
    QVERIFY(!r->hasError());
    r->waitForFinished();
    QVERIFY(!r->hasError());

    validateResults(r, expectedResultsSize);
    delete r;
}

void tst_QSparqlAPI::syncExec_waitForFinished_query_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<QString>("query");
    QTest::addColumn<int>("expectedResultsSize");

    QTest::newRow("Tracker Direct Sync Select")
        << "QTRACKER_DIRECT"
        << contactSelectQuery
        << NUM_TRACKER_INSERTS;

    QTest::newRow("DBus Sync Select")
        << "QTRACKER"
        << contactSelectQuery
        << NUM_TRACKER_INSERTS;
}

void tst_QSparqlAPI::syncExec_waitForFinished_update_query_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, insertTemplate);
    QFETCH(QString, deleteTemplate);
    QFETCH(QString, validateQuery);
    QFETCH(int, initialSize);
    QFETCH(int, contactInserts);
    QFETCH(int, contactDeletes);

    QSparqlConnection conn(connectionDriver);
    const int expectedResultsSize = initialSize + contactInserts;

    QString insertQuery = "insert { <qsparql-api-tests> a nie:InformationElement .";
    for (int item = initialSize+1; item <= expectedResultsSize; item++) {
        insertQuery.append( insertTemplate.arg(item) );
    }
    insertQuery.append(" }");

    const QSparqlQuery q(insertQuery, QSparqlQuery::InsertStatement);
    QSparqlResult* r = conn.syncExec(q);
    QVERIFY(!r->hasError());
    r->waitForFinished();
    QVERIFY(!r->hasError());
    delete r;

    // Verify the insertion
    r = conn.syncExec(QSparqlQuery(contactSelectQuery));
    r->waitForFinished();
    validateResults(r, expectedResultsSize);
    delete r;

    // Delete the insertion
    QString deleteQuery = "delete { <qsparql-api-tests> a nie:InformationElement .";
    for (int item = initialSize+1; item <= expectedResultsSize; item++) {
        deleteQuery.append( deleteTemplate.arg(item) );
    }
    deleteQuery.append(" }");

    QSparqlQuery delQuery(deleteQuery, QSparqlQuery::DeleteStatement);
    r = conn.syncExec(delQuery);
    QVERIFY(!r->hasError());
    r->waitForFinished();
    QVERIFY(!r->hasError());
    delete r;

    // Now verify deletion
    r = conn.syncExec(QSparqlQuery(validateQuery));
    r->waitForFinished();
    validateResults(r, expectedResultsSize - contactDeletes);
    delete r;
}

void tst_QSparqlAPI::syncExec_waitForFinished_update_query_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<QString>("insertTemplate");
    QTest::addColumn<QString>("deleteTemplate");
    QTest::addColumn<QString>("validateQuery");
    QTest::addColumn<int>("initialSize");
    QTest::addColumn<int>("contactInserts");
    QTest::addColumn<int>("contactDeletes");

    QTest::newRow("Tracker Direct Sync Update Query")
        << "QTRACKER_DIRECT"
        << contactInsertQueryTemplate
        << contactDeleteQueryTemplate
        << contactSelectQuery
        << NUM_TRACKER_INSERTS
        << contactInsertAmount
        << contactDeleteAmount;

    QTest::newRow("DBus Sync Update Query")
        << "QTRACKER"
        << contactInsertQueryTemplate
        << contactDeleteQueryTemplate
        << contactSelectQuery
        << NUM_TRACKER_INSERTS
        << contactInsertAmount
        << contactDeleteAmount;
}

QTEST_MAIN( tst_QSparqlAPI )
#include "tst_qsparql_api.moc"
