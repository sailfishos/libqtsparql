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
#include <QtSparql>

#include "../messagerecorder.h"
#include "../testhelpers.h"

// define the amount of data we are going to insert for the tests
#define NUM_INSERTS 15

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

    void connection_test();
    void connection_test_data();

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

    void queryModel_test();
    void queryModel_test_data();

    void syncExec_waitForFinished_query_test();
    void syncExec_waitForFinished_query_test_data();

    void syncExec_waitForFinished_update_query_test();
    void syncExec_waitForFinished_update_query_test_data();
    
    void query_with_prefix();
    void query_with_prefix_data();

    void go_beyond_columns_number();
    void go_beyond_columns_number_data();

private:
    void insertTrackerTestData();
    void insertEndpointTestData();
    void cleanupTrackerTestData();
    void cleanupEndpointTestData();
    void add_query_test_data(const QString& connectionDriver, const QString& dataTagPrefix);
    void add_query_error_test_data(const QString& connectionDriver, const QString& dataTagPrefix, bool supportsConstruct);
    void add_query_destroy_connection_test_data(const QString& connectionDriver, const QString& dataTagPrefix);
    void add_update_query_test_data(const QString& connectionDriver, const QString& dataTagPrefix);
    void add_update_query_error_test_data(const QString& connectionDriver, const QString& dataTagPrefix, bool supportsConstruct);
    void add_update_query_destroy_connection_test_data(const QString& connectionDriver, const QString& dataTagPrefix);
    void add_ask_query_test_data(const QString& connectionDriver, const QString& dataTagPrefix);

private:
    MessageRecorder *msgRecorder;
    bool testEndpoint;
};

namespace {

const QString contactSelectQueryTemplate =
    "select ?u ?ng %1 {"
    "    ?u a nco:PersonContact; "
    "    nie:isLogicalPartOf <qsparql-api-tests> ;"
    "    nco:nameGiven ?ng .}";
const QString contactSelectNothingQuery =
    "select ?u ?ng {"
    "    ?u a nco:PersonContact; "
    "    nie:isLogicalPartOf <qsparql-api-tests-not-here> ;"
    "    nco:nameGiven ?ng .}";
const int contactSelectColumnCount = 2;

const QString contactInsertHeader =
    "insert %1 { <qsparql-api-tests> a nie:InformationElement.";
const QString contactInsertQueryTemplate =
    "<uri00%1> a nco:PersonContact;"
    "nie:isLogicalPartOf <qsparql-api-tests> ;"
    "nco:nameGiven \"name00%1\" .";
const int contactInsertAmount = 1;

const QString contactDeleteHeader =
    "delete %1 { <qsparql-api-tests> a nie:InformationElement .";
const QString contactDeleteQueryTemplate =
    "<uri00%1> a nco:PersonContact .";
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

const QString invalidQuery =
    "Invalid query";

class FinishedSignalReceiver : public QObject
{
    Q_OBJECT
    int numFinished;
    QObject* expectedSender;

public:
    FinishedSignalReceiver()
        : numFinished(0), expectedSender(0)
    { }

    void connectFinished(QObject* sender)
    {
        QVERIFY( QObject::connect(sender, SIGNAL(finished()), this, SLOT(onFinished())) );
        expectedSender = sender;
    }

    void waitForFinished(int timeoutMs)
    {
        bool timeout = false;
        numFinished = 0;
        QTime timeoutTimer;
        timeoutTimer.start();
        while (numFinished == 0 && !(timeout = (timeoutTimer.elapsed() > timeoutMs))) {
            QTest::qWait(qMax(20, timeoutMs / 100));
        }
        QVERIFY(!timeout);
    }

    void ensureOneFinishedReceived(int timeoutMs)
    {
        QCOMPARE( numFinished, 1 );
        QTest::qWait(timeoutMs);
        QCOMPARE( numFinished, 1 );
    }

public slots:
    void onFinished()
    {
        ++numFinished;
        QCOMPARE( sender(), expectedSender );
    }
};

void checkExecutionMethod(QSparqlResult* r, const int executionMethod, const bool useAsyncObject)
{
    QVERIFY(r);
    if (executionMethod == QSparqlQueryOptions::AsyncExec) {
        if (useAsyncObject) {
            // As per documentation requirement, only attempt to connect the
            // signals after first validating that there is no error
            if (!r->hasError()) {
                FinishedSignalReceiver signalReceiver;
                signalReceiver.connectFinished(r);
                signalReceiver.waitForFinished(2000);
                signalReceiver.ensureOneFinishedReceived(100);
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
    QList<QString> contactOrder;

    QVERIFY(r->pos() == QSparql::BeforeFirstRow);
    QVERIFY(!r->isValid());
    int resultSize = 0;
    while (r->next()) {
        QVERIFY(r->isValid());
        // keep a list of the uri's to be used to track the insert order
        contactOrder.append(r->value(0).toString());

        contactNamesValue[r->value(0).toString()] = r->value(1).toString();
        contactNamesBindings[r->binding(0).value().toString()] = r->binding(1).value().toString();
        contactNamesStringValue[r->stringValue(0)] = r->stringValue(1);
        ++resultSize;
    }
    QVERIFY(r->pos() == QSparql::AfterLastRow);
    QVERIFY(!r->isValid());
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
            QVERIFY(!r->isValid());
        } else {
            // Iterate the result backwards
            int resultSize = 0;
            while (r->previous()) {
                QVERIFY(r->pos() != QSparql::AfterLastRow);
                QVERIFY(r->isValid());
                QCOMPARE(contactNamesValue[r->value(0).toString()], r->value(1).toString());
                QCOMPARE(contactNamesBindings[r->binding(0).value().toString()], r->binding(1).value().toString());
                QCOMPARE(contactNamesStringValue[r->stringValue(0)], r->stringValue(1));
                ++resultSize;
            }
            QCOMPARE(resultSize, expectedResultsSize);
            // Now move forward one...
            QVERIFY(r->pos() == QSparql::BeforeFirstRow);
            QVERIFY(!r->isValid());
            QVERIFY(r->next());
            QVERIFY(r->pos() != QSparql::BeforeFirstRow);
            QVERIFY(r->isValid());
            QCOMPARE(contactNamesValue[contactOrder.at(0)], r->value(1).toString());
            QCOMPARE(contactNamesBindings[contactOrder.at(0)], r->binding(1).value().toString());
            QCOMPARE(contactNamesStringValue[contactOrder.at(0)], r->stringValue(1));
        }
    } else {
        //Make sure using the result doesn't crash
        QVERIFY(r->pos() < 0);
        QVERIFY(!r->isValid());
        QVERIFY(!r->next());
        QVERIFY(!r->previous());
        QVERIFY(r->pos() < 0);
        QVERIFY(!r->isValid());
    }
}

void validateBoolVariant_impl(bool& result, const QVariant& variant, const bool expectedResult)
{
    result = false;
    QVERIFY(variant.canConvert<bool>());
    QCOMPARE(variant.toBool(), expectedResult);
    result = true;
}

bool validateBoolVariant(const QVariant& variant, const bool expectedResult)
{
    bool result = false;
    validateBoolVariant_impl(result, variant, expectedResult);
    return result;
}

void validateCurrentBoolResult_impl(bool& result, QSparqlResult* r, const bool expectedResult)
{
    result = false;

    QVERIFY(!r->hasError());
    QVERIFY(r->isValid());
    QVERIFY(r->isBool());
    QCOMPARE(r->boolValue(), expectedResult);

    QCOMPARE(r->current().count(), 1);
    QVERIFY(validateBoolVariant(r->current().value(0), expectedResult));

    QVERIFY(validateBoolVariant(r->value(0), expectedResult));

    QVERIFY(r->binding(0).isValid());
    QVERIFY(r->binding(0).isLiteral());
    QVERIFY(validateBoolVariant(r->binding(0).value(), expectedResult));

    result = true;
}

bool validateCurrentBoolResult(QSparqlResult* r, const bool expectedResult)
{
    bool result = false;
    validateCurrentBoolResult_impl(result, r, expectedResult);
    return result;
}

void validateErrorResult(QSparqlResult *r, int expectedErrorType)
{
    QVERIFY(r);
    QVERIFY(r->hasError());
    QVERIFY(r->lastError().type() == expectedErrorType);

    // Check iteration behaviour
    QVERIFY(r->pos() < 0);
    QVERIFY(!r->isValid());
    QVERIFY(!r->next());
    QVERIFY(!r->previous());
    QVERIFY(r->pos() < 0);
    QVERIFY(!r->isValid());
}

QSparqlConnectionOptions getConnectionOptions(QString driver)
{
    QSparqlConnectionOptions options;
    if (driver == "QSPARQL_ENDPOINT") {
        options.setHostName("127.0.0.1");
        options.setPort(8890);
        return options;
    }
    return options;
}

QString getTemplateArguments(QString driver, QString query)
{
    if (driver == "QSPARQL_ENDPOINT") {
        if (query == "SELECT" || query == "DELETE") {
            return "FROM <http://virtuoso_endpoint/testgraph>";
        } else {
            return "INTO <http://virtuoso_endpoint/testgraph>";
        }
    } else {
        return "";
    }
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

    testEndpoint = false;
    cleanupTestCase();
    insertTrackerTestData();
    insertEndpointTestData();
}

void tst_QSparqlAPI::insertTrackerTestData()
{
    const QString insertQueryTemplate =
        "<uri00%1> a nco:PersonContact, nie:InformationElement ;"
        "nie:isLogicalPartOf <qsparql-api-tests> ;"
        "nco:nameGiven \"name00%1\" .";
    QString insertQuery = "insert { <qsparql-api-tests> a nie:InformationElement .";
    for (int item = 1; item <= NUM_INSERTS; item++) {
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

void tst_QSparqlAPI::insertEndpointTestData()
{
    const QString insertQueryTemplate =
        "<uri00%1> a nco:PersonContact, nie:InformationElement ;"
        "nie:isLogicalPartOf <qsparql-api-tests> ;"
        "nco:nameGiven \"name00%1\" .";
    QString insertQuery = "insert into <http://virtuoso_endpoint/testgraph> { <qsparql-api-tests> a nie:InformationElement .";
    for (int item = 1; item <= NUM_INSERTS; item++) {
        insertQuery.append( insertQueryTemplate.arg(item) );
    }
    insertQuery.append(" }");
    QSparqlConnectionOptions options = getConnectionOptions("QSPARQL_ENDPOINT");
    QSparqlConnection conn("QSPARQL_ENDPOINT", options);
    const QSparqlQuery q(insertQuery,QSparqlQuery::InsertStatement);
    QSparqlResult* r = conn.exec(q);
    r->waitForFinished();
    if (!r->hasError()) {
        testEndpoint = true;
    }
    delete r;

}

void tst_QSparqlAPI::cleanupTestCase()
{
    cleanupTrackerTestData();
    if (testEndpoint)
        cleanupEndpointTestData();
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

void tst_QSparqlAPI::cleanupEndpointTestData()
{
    QSparqlConnectionOptions options = getConnectionOptions("QSPARQL_ENDPOINT");
    QSparqlConnection conn("QSPARQL_ENDPOINT", options);
    const QSparqlQuery q("DELETE FROM <http://virtuoso_endpoint/testgraph> { ?u a nco:PersonContact . } "
                         "WHERE { ?u nie:isLogicalPartOf <qsparql-api-tests> .}",
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

void tst_QSparqlAPI::connection_test()
{
    QFETCH(QString, driverName);
    QFETCH(bool, isValid);
    
    QSparqlConnection conn(driverName);
    QCOMPARE(driverName, conn.driverName());
    QCOMPARE(conn.isValid(), isValid);
    // Connection in direct driver is asynchronous so we have to wait a while
    if(driverName == "QTRACKER_DIRECT")
        QTest::qWait(1000);
    QCOMPARE(conn.hasError(), !isValid);
}

void tst_QSparqlAPI::connection_test_data()
{
    QTest::addColumn<QString>("driverName");
    QTest::addColumn<bool>("isValid");
    
    QTest::newRow("dbus driver") << "QTRACKER" << true;
    QTest::newRow("direct driver") <<  "QTRACKER_DIRECT" << true;
    QTest::newRow("dummy driver") << "QTRACKER_DUMMY" << false;

}

void tst_QSparqlAPI::query_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, queryTemplate);
    QFETCH(int, expectedResultsSize);
    QFETCH(int, executionMethod);
    QFETCH(bool, useAsyncObject);
    QFETCH(bool, forwardOnly);

    QSparqlConnectionOptions options = getConnectionOptions(connectionDriver);
    QSparqlConnection conn(connectionDriver, options);

    //QTRACKER_DIRECT uses asynchronous conn opening, so we have to wait a while
    // to get valid hasError() state
    if(connectionDriver == "QTRACKER_DIRECT")
        QTest::qWait(1000);
    QCOMPARE(conn.hasError(), false);

    QSparqlQueryOptions queryOptions;
    queryOptions.setForwardOnly(forwardOnly);
    queryOptions.setExecutionMethod(QSparqlQueryOptions::ExecutionMethod(executionMethod));

    const QString queryString = queryTemplate.arg( getTemplateArguments(connectionDriver, "SELECT") );
    const QSparqlQuery q(queryString);

    QSparqlResult* r = conn.exec(q, queryOptions);
    checkExecutionMethod(r, executionMethod, useAsyncObject);
    validateResults(r, expectedResultsSize);

    delete r;
}

void tst_QSparqlAPI::add_query_test_data(const QString& connectionDriver, const QString& dataTagPrefix)
{
    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Query")))
        << connectionDriver
        << contactSelectQueryTemplate
        << NUM_INSERTS
        << int(QSparqlQueryOptions::AsyncExec)
        << false
        << false;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Query Forward Only")))
        << connectionDriver
        << contactSelectQueryTemplate
        << NUM_INSERTS
        << int(QSparqlQueryOptions::AsyncExec)
        << false
        << true;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async No Results Query")))
        << connectionDriver
        << contactSelectNothingQuery
        << 0
        << int(QSparqlQueryOptions::AsyncExec)
        << false
        << false;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async No Results Query Forward Only")))
        << connectionDriver
        << contactSelectNothingQuery
        << 0
        << int(QSparqlQueryOptions::AsyncExec)
        << false
        << true;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Sync Query")))
        << connectionDriver
        << contactSelectQueryTemplate
        << NUM_INSERTS
        << int(QSparqlQueryOptions::SyncExec)
        << false
        << false;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Sync No Results Query")))
        << connectionDriver
        << contactSelectNothingQuery
        << 0
        << int(QSparqlQueryOptions::SyncExec)
        << false
        << false;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Object Query")))
        << connectionDriver
        << contactSelectQueryTemplate
        << NUM_INSERTS
        << int(QSparqlQueryOptions::AsyncExec)
        << true
        << false;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Object Query Forward Only")))
        << connectionDriver
        << contactSelectQueryTemplate
        << NUM_INSERTS
        << int(QSparqlQueryOptions::AsyncExec)
        << true
        << true;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Object No Results Query")))
        << connectionDriver
        << contactSelectNothingQuery
        << 0
        << int(QSparqlQueryOptions::AsyncExec)
        << true
        << false;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Object No Results Query Forward Only")))
        << connectionDriver
        << contactSelectNothingQuery
        << 0
        << int(QSparqlQueryOptions::AsyncExec)
        << true
        << true;
}

void tst_QSparqlAPI::query_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<QString>("queryTemplate");
    QTest::addColumn<int>("expectedResultsSize");
    QTest::addColumn<int>("executionMethod");
    QTest::addColumn<bool>("useAsyncObject");
    QTest::addColumn<bool>("forwardOnly");
    add_query_test_data("QTRACKER_DIRECT", "Tracker Direct");
    add_query_test_data("QTRACKER", "Tracker DBus");
    if (testEndpoint)
        add_query_test_data("QSPARQL_ENDPOINT", "Endpoint");

}

void tst_QSparqlAPI::query_destroy_connection_after_finished_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, queryTemplate);
    QFETCH(int, expectedResultsSize);
    QFETCH(int, executionMethod);
    QFETCH(bool, useAsyncObject);

    QSparqlConnectionOptions options = getConnectionOptions(connectionDriver);
    QSparqlConnection* conn = new QSparqlConnection(connectionDriver, options);

    QSparqlQueryOptions queryOptions;
    queryOptions.setExecutionMethod(QSparqlQueryOptions::ExecutionMethod(executionMethod));

    const QString queryString = queryTemplate.arg( getTemplateArguments(connectionDriver, "SELECT") );
    const QSparqlQuery q(queryString);

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
    QFETCH(bool, forwardOnly);

    QSparqlConnectionOptions options = getConnectionOptions(connectionDriver);
    QSparqlConnection conn(connectionDriver, options);

    QSparqlQueryOptions queryOptions;
    queryOptions.setForwardOnly(forwardOnly);
    queryOptions.setExecutionMethod(QSparqlQueryOptions::ExecutionMethod(executionMethod));
    const QSparqlQuery q(query);

    QSparqlResult* r = conn.exec(q, queryOptions);

    checkExecutionMethod(r, executionMethod, useAsyncObject);
    validateErrorResult(r, expectedErrorType);
    QCOMPARE((*msgRecorder)[QtWarningMsg].count(), 1);

    delete r;
}

void tst_QSparqlAPI::add_query_error_test_data(const QString& connectionDriver, const QString& dataTagPrefix, bool supportsConstruct)
{
    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Empty Query")))
        << connectionDriver
        << ""
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << false
        << false;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Empty Query Forward Only")))
        << connectionDriver
        << ""
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << false
        << true;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Object Empty Query")))
        << connectionDriver
        << ""
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << true
        << false;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Object Empty Query Forward Only")))
        << connectionDriver
        << ""
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << true
        << true;


    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Sync Empty Query")))
        << connectionDriver
        << ""
        << int(QSparqlQueryOptions::SyncExec)
        << int(QSparqlError::StatementError)
        << false
        << false;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Invalid Query")))
        << connectionDriver
        << invalidQuery
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << false
        << false;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Invalid Query Forward Only")))
        << connectionDriver
        << invalidQuery
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << false
        << true;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Object Invalid Query")))
        << connectionDriver
        << invalidQuery
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << true
        << false;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Object Invalid Query Forward Only")))
        << connectionDriver
        << invalidQuery
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << true
        << true;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Sync Invalid Query")))
        << connectionDriver
        << invalidQuery
        << int(QSparqlQueryOptions::SyncExec)
        << int(QSparqlError::StatementError)
        << false
        << false;

    if (!supportsConstruct) {
        QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Unsupported Construct Query")))
            << connectionDriver
            << constructQuery
            << int(QSparqlQueryOptions::AsyncExec)
            << int(QSparqlError::BackendError)
            << false
            << false;

        QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Unsupported Construct Query Forward Only")))
            << connectionDriver
            << constructQuery
            << int(QSparqlQueryOptions::AsyncExec)
            << int(QSparqlError::BackendError)
            << false
            << false;

        QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Object Unsupported Construct Query")))
            << connectionDriver
            << constructQuery
            << int(QSparqlQueryOptions::AsyncExec)
            << int(QSparqlError::BackendError)
            << true
            << false;

        QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Object Unsupported Construct Query Forward Only")))
            << connectionDriver
            << constructQuery
            << int(QSparqlQueryOptions::AsyncExec)
            << int(QSparqlError::BackendError)
            << true
            << true;

        QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Sync Unsupported Construct Query")))
            << connectionDriver
            << constructQuery
            << int(QSparqlQueryOptions::SyncExec)
            << int(QSparqlError::BackendError)
            << false
            << false;
    }
}

void tst_QSparqlAPI::query_error_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<QString>("query");
    QTest::addColumn<int>("executionMethod");
    QTest::addColumn<int>("expectedErrorType");
    QTest::addColumn<bool>("useAsyncObject");
    QTest::addColumn<bool>("forwardOnly");
    add_query_error_test_data("QTRACKER_DIRECT", "Tracker Direct", false);
    add_query_error_test_data("QTRACKER", "Tracker DBus", false);
    if (testEndpoint)
        add_query_error_test_data("QSPARQL_ENDPOINT", "Endpoint", true);
}

void tst_QSparqlAPI::query_destroy_connection_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, query);
    QFETCH(int, expectedResultsSize);
    QFETCH(int, executionMethod);
    QFETCH(bool, useAsyncObject);
    QFETCH(bool, forwardOnly);

    QSparqlConnectionOptions options = getConnectionOptions(connectionDriver);
    QSparqlConnection *conn = new QSparqlConnection(connectionDriver, options);

    QSparqlQueryOptions queryOptions;
    queryOptions.setForwardOnly(forwardOnly);
    queryOptions.setExecutionMethod(QSparqlQueryOptions::ExecutionMethod(executionMethod));

    const QString queryString = query.arg( getTemplateArguments(connectionDriver, "SELECT") );
    const QSparqlQuery q(queryString);

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

void tst_QSparqlAPI::add_query_destroy_connection_test_data(const QString& connectionDriver, const QString& dataTagPrefix)
{
    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Query")))
        << connectionDriver
        << contactSelectQueryTemplate
        << NUM_INSERTS
        << int(QSparqlQueryOptions::AsyncExec)
        << false
        << false;
    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Query Forward Only")))
        << connectionDriver
        << contactSelectQueryTemplate
        << NUM_INSERTS
        << int(QSparqlQueryOptions::AsyncExec)
        << false
        << true;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Object Query")))
        << connectionDriver
        << contactSelectQueryTemplate
        << NUM_INSERTS
        << int(QSparqlQueryOptions::AsyncExec)
        << true
        << false;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Object Query Forward Only")))
        << connectionDriver
        << contactSelectQueryTemplate
        << NUM_INSERTS
        << int(QSparqlQueryOptions::AsyncExec)
        << true
        << true;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Sync Query")))
        << connectionDriver
        << contactSelectQueryTemplate
        << NUM_INSERTS
        << int(QSparqlQueryOptions::SyncExec)
        << false
        << false;
}

void tst_QSparqlAPI::query_destroy_connection_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<QString>("query");
    QTest::addColumn<int>("expectedResultsSize");
    QTest::addColumn<int>("executionMethod");
    QTest::addColumn<bool>("useAsyncObject");
    QTest::addColumn<bool>("forwardOnly");
    add_query_destroy_connection_test_data("QTRACKER_DIRECT", "Tracker Direct");
    add_query_destroy_connection_test_data("QTRACKER", "Tracker DBus");
    if (testEndpoint)
        add_query_destroy_connection_test_data("QSPARQL_ENDPOINT", "Endpoint");
}

void tst_QSparqlAPI::update_query_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, insertTemplate);
    QFETCH(QString, deleteTemplate);
    QFETCH(int, initialSize);
    QFETCH(int, contactInserts);
    QFETCH(int, contactDeletes);
    QFETCH(int, executionMethod);
    QFETCH(bool, useAsyncObject);

    QSparqlConnectionOptions options = getConnectionOptions(connectionDriver);
    QSparqlConnection conn(connectionDriver, options);

    const int expectedResultsSize = initialSize + contactInserts;

    QString insertQuery = contactInsertHeader.arg( getTemplateArguments(connectionDriver, "INSERT") ); //

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
    QString validateQuery = contactSelectQueryTemplate.arg(getTemplateArguments(connectionDriver, "SELECT"));
    r = conn.exec(QSparqlQuery(validateQuery), queryOptions);
    checkExecutionMethod(r, executionMethod, useAsyncObject);
    validateResults(r, expectedResultsSize);
    delete r;

    // Delete the insertion
    QString deleteQuery = contactDeleteHeader.arg( getTemplateArguments(connectionDriver, "DELETE") ); //"delete { <qsparql-api-tests> a nie:InformationElement .";
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

void tst_QSparqlAPI::add_update_query_test_data(const QString& connectionDriver, const QString& dataTagPrefix)
{
    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Update Query")))
        << connectionDriver
        << contactInsertQueryTemplate
        << contactDeleteQueryTemplate
        << NUM_INSERTS
        << contactInsertAmount
        << contactDeleteAmount
        << int(QSparqlQueryOptions::AsyncExec)
        << false;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Object Update Query")))
        << connectionDriver
        << contactInsertQueryTemplate
        << contactDeleteQueryTemplate
        << NUM_INSERTS
        << contactInsertAmount
        << contactDeleteAmount
        << int(QSparqlQueryOptions::AsyncExec)
        << true;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Sync Update Query")))
        << connectionDriver
        << contactInsertQueryTemplate
        << contactDeleteQueryTemplate
        << NUM_INSERTS
        << contactInsertAmount
        << contactDeleteAmount
        << int(QSparqlQueryOptions::SyncExec)
        << false;
}

void tst_QSparqlAPI::update_query_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<QString>("insertTemplate");
    QTest::addColumn<QString>("deleteTemplate");
    QTest::addColumn<int>("initialSize");
    QTest::addColumn<int>("contactInserts");
    QTest::addColumn<int>("contactDeletes");
    QTest::addColumn<int>("executionMethod");
    QTest::addColumn<bool>("useAsyncObject");
    add_update_query_test_data("QTRACKER_DIRECT", "Tracker Direct");
    add_update_query_test_data("QTRACKER", "Tracker DBus");
    if (testEndpoint)
        add_update_query_test_data("QSPARQL_ENDPOINT", "Endpoint");
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
        QSparqlConnectionOptions options = getConnectionOptions(connectionDriver);
        QSparqlConnection conn(connectionDriver, options);
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

void tst_QSparqlAPI::add_update_query_error_test_data(const QString& connectionDriver, const QString& dataTagPrefix, bool supportsConstruct)
{
    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Empty Query")))
        << connectionDriver
        << ""
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << false;

   QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Object Empty Query")))
        << connectionDriver
        << ""
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << true;

   QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Sync Empty Query")))
       << connectionDriver
       << ""
       << int(QSparqlQueryOptions::SyncExec)
       << int(QSparqlError::StatementError)
       << false;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Invalid Query")))
        << connectionDriver
        << invalidQuery
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << false;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Object Invalid Query")))
        << connectionDriver
        << invalidQuery
        << int(QSparqlQueryOptions::AsyncExec)
        << int(QSparqlError::StatementError)
        << true;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Sync Invalid Query")))
        << connectionDriver
        << invalidQuery
        << int(QSparqlQueryOptions::SyncExec)
        << int(QSparqlError::StatementError)
        << false;

    if (!supportsConstruct) {
        QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Unsupported Construct Query")))
            << connectionDriver
            << constructQuery
            << int(QSparqlQueryOptions::AsyncExec)
            << int(QSparqlError::StatementError)
            << false;

        QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Object Unsupported Construct Query")))
            << connectionDriver
            << constructQuery
            << int(QSparqlQueryOptions::AsyncExec)
            << int(QSparqlError::StatementError)
            << true;

        QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Sync Unsupported Construct Query")))
            << connectionDriver
            << constructQuery
            << int(QSparqlQueryOptions::SyncExec)
            << int(QSparqlError::StatementError)
            << false;
    }
}

void tst_QSparqlAPI::update_query_error_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<QString>("query");
    QTest::addColumn<int>("executionMethod");
    QTest::addColumn<int>("expectedErrorType");
    QTest::addColumn<bool>("useAsyncObject");
    add_update_query_error_test_data("QTRACKER_DIRECT", "Tracker Direct", false);
    add_update_query_error_test_data("QTRACKER", "Tracker DBus", false);
    if (testEndpoint)
        add_update_query_error_test_data("QSPARQL_ENDPOINT", "Endpoint", true);
}

void tst_QSparqlAPI::update_query_destroy_connection_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, query);
    QFETCH(int, executionMethod);
    QFETCH(bool, useAsyncObject);
    QFETCH(QString, deleteQuery);

    QSparqlConnectionOptions cleanupOptions = getConnectionOptions(connectionDriver);
    QSparqlConnection cleanupConn(connectionDriver, cleanupOptions);

    QSparqlQuery::StatementType queryType = QSparqlQuery::InsertStatement;
    for (int round = 1; round <= 2; ++round) {
        QSparqlConnectionOptions options = getConnectionOptions(connectionDriver);
        QSparqlConnection* conn = new QSparqlConnection(connectionDriver, options);
        QSparqlQueryOptions queryOptions;
        queryOptions.setExecutionMethod(QSparqlQueryOptions::ExecutionMethod(executionMethod));
        const QSparqlQuery q(query, queryType);

        QSparqlResult* r = conn->exec(q, queryOptions);
        const bool immediatelyFinished = r->isFinished();
        QVERIFY(!r->hasError());
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

void tst_QSparqlAPI::add_update_query_destroy_connection_test_data(const QString& connectionDriver, const QString& dataTagPrefix)
{
    QString insertQuery = contactInsertHeader.arg( getTemplateArguments(connectionDriver, "INSERT") );
    insertQuery.append(contactInsertQueryTemplate.arg(NUM_INSERTS+1));
    insertQuery.append(" }");

    QString deleteQuery = contactDeleteHeader.arg( getTemplateArguments(connectionDriver, "DELETE") );
    deleteQuery.append(contactDeleteQueryTemplate.arg(NUM_INSERTS+1));
    deleteQuery.append(" }");

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Query")))
        << connectionDriver
        << insertQuery
        << int(QSparqlQueryOptions::AsyncExec)
        << false
        << deleteQuery;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Object Query")))
        << connectionDriver
        << insertQuery
        << int(QSparqlQueryOptions::AsyncExec)
        << true
        << deleteQuery;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Sync Query")))
        << connectionDriver
        << insertQuery
        << int(QSparqlQueryOptions::SyncExec)
        << false
        << deleteQuery;
}

void tst_QSparqlAPI::update_query_destroy_connection_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<QString>("query");
    QTest::addColumn<int>("executionMethod");
    QTest::addColumn<bool>("useAsyncObject");
    QTest::addColumn<QString>("deleteQuery");
    add_update_query_destroy_connection_test_data("QTRACKER_DIRECT", "Tracker Direct");
    add_update_query_destroy_connection_test_data("QTRACKER", "Tracker DBus");
    if (testEndpoint)
        add_update_query_destroy_connection_test_data("QSPARQL_ENDPOINT", "Endpoint");
}

void tst_QSparqlAPI::ask_query_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, query);
    QFETCH(int, executionMethod);
    QFETCH(bool, expectedResult);
    QFETCH(bool, useAsyncObject);
    QFETCH(bool, forwardOnly);

    QSparqlConnectionOptions options = getConnectionOptions(connectionDriver);
    QSparqlConnection conn(connectionDriver, options);
    QVERIFY(conn.hasFeature(QSparqlConnection::AskQueries));

    const QSparqlQuery q(query, QSparqlQuery::AskStatement);

    QSparqlQueryOptions queryOptions;
    queryOptions.setForwardOnly(forwardOnly);
    queryOptions.setExecutionMethod(QSparqlQueryOptions::ExecutionMethod(executionMethod));

    QSparqlResult *r = conn.exec(q, queryOptions);

    checkExecutionMethod(r, executionMethod, useAsyncObject);
    const bool immediatelyFinished = r->isFinished();

    if (immediatelyFinished) {
        QVERIFY(!r->hasError());
        // Boolean results are retrieved immediately without the need to call next() for conveniency
        QVERIFY(r->isBool());
        QCOMPARE(r->boolValue(), expectedResult);
        QCOMPARE(r->size(), 1);
    }

    int resultSize = 0;
    while (r->next()) {
        if (++resultSize <= 1) {
            QVERIFY(validateCurrentBoolResult(r, expectedResult));
        }
        else {
            qDebug() << "Unexpected result row for an ask query:" << r->current();
        }
    }
    QVERIFY(r->isFinished());
    QCOMPARE(resultSize, 1);

    delete r;
}

void tst_QSparqlAPI::add_ask_query_test_data(const QString& connectionDriver, const QString& dataTagPrefix)
{
    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async True Query")))
        << connectionDriver
        << int(QSparqlQueryOptions::AsyncExec)
        << askQueryTrue
        << true
        << false
        << false;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Forward Only True Query")))
        << connectionDriver
        << int(QSparqlQueryOptions::AsyncExec)
        << askQueryTrue
        << true
        << false
        << true;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Object True Query")))
        << connectionDriver
        << int(QSparqlQueryOptions::AsyncExec)
        << askQueryTrue
        << true
        << true
        << false;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Object Forward Only True Query")))
        << connectionDriver
        << int(QSparqlQueryOptions::AsyncExec)
        << askQueryTrue
        << true
        << true
        << true;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Sync True Query")))
        << connectionDriver
        << int(QSparqlQueryOptions::SyncExec)
        << askQueryTrue
        << true
        << false
        << false;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async False Query")))
        << connectionDriver
        << int(QSparqlQueryOptions::AsyncExec)
        << askQueryFalse
        << false
        << false
        << false;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Forward Only False Query")))
        << connectionDriver
        << int(QSparqlQueryOptions::AsyncExec)
        << askQueryFalse
        << false
        << false
        << true;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Object False Query")))
        << connectionDriver
        << int(QSparqlQueryOptions::AsyncExec)
        << askQueryFalse
        << false
        << true
        << false;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Async Object Forward Only False Query")))
        << connectionDriver
        << int(QSparqlQueryOptions::AsyncExec)
        << askQueryFalse
        << false
        << true
        << true;

    QTest::newRow(qPrintable(QString(dataTagPrefix).append(" Sync False Query")))
        << connectionDriver
        << int(QSparqlQueryOptions::SyncExec)
        << askQueryFalse
        << false
        << false
        << false;
}

void tst_QSparqlAPI::ask_query_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<int>("executionMethod");
    QTest::addColumn<QString>("query");
    QTest::addColumn<bool>("expectedResult");
    QTest::addColumn<bool>("useAsyncObject");
    QTest::addColumn<bool>("forwardOnly");
    add_ask_query_test_data("QTRACKER_DIRECT", "Tracker Direct");
    add_ask_query_test_data("QTRACKER", "Tracker DBus");
    if (testEndpoint)
        add_ask_query_test_data("QSPARQL_ENDPOINT", "Endpoint");
}

void tst_QSparqlAPI::isFinished_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, queryTemplate);
    QFETCH(int, expectedResultsSize);
    QFETCH(int, executionMethod);
    QFETCH(bool, useAsyncObject);
    QFETCH(bool, forwardOnly);

    QSparqlConnectionOptions options = getConnectionOptions(connectionDriver);
    QSparqlConnection conn(connectionDriver, options);

    QSparqlQueryOptions queryOptions;
    queryOptions.setForwardOnly(forwardOnly);
    queryOptions.setExecutionMethod(QSparqlQueryOptions::ExecutionMethod(executionMethod));

    const QString queryString = queryTemplate.arg( getTemplateArguments(connectionDriver, "SELECT") );
    const QSparqlQuery q(queryString);

    QSparqlResult* r = conn.exec(q, queryOptions);

    checkExecutionMethod(r, executionMethod, useAsyncObject);

    // According to the documentation, isFinished() will be false for sync queries
    // until the results have been iterated
    // If the result has been set as ForwardOnly, it should behave in the same
    // way as a Sync result, so also check that here by also testing
    // if the result reports as ForwardOnly
    bool forwardOnlyResult = r->hasFeature(QSparqlResult::ForwardOnly);
    if ((executionMethod == QSparqlQueryOptions::SyncExec &&
        conn.hasFeature(QSparqlConnection::SyncExec)) || forwardOnlyResult) {
        QVERIFY(!r->isFinished());
        validateResults(r, expectedResultsSize);
        QVERIFY(r->isFinished());
    } else {
        QVERIFY(r->isFinished());
        validateResults(r, expectedResultsSize);
        QVERIFY(r->isFinished());
    }

    delete r;
}

void tst_QSparqlAPI::isFinished_test_data()
{
    query_test_data();
}

void tst_QSparqlAPI::queryModel_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, query);
    QFETCH(int, expectedResultsSize);

    QSparqlConnectionOptions options = getConnectionOptions(connectionDriver);
    QSparqlConnection conn(connectionDriver, options);
    QSparqlQueryModel model;

    model.setQuery(QSparqlQuery(query), conn);

    FinishedSignalReceiver signalReceiver;
    signalReceiver.connectFinished(&model);
    signalReceiver.waitForFinished(2000);
    signalReceiver.ensureOneFinishedReceived(100);

    QCOMPARE(model.rowCount(), expectedResultsSize);
    QCOMPARE(model.columnCount(), contactSelectColumnCount);

    // Test the results against a query to get the insertion order
    QList<QString> insertOrder;
    QSparqlResult *r = conn.syncExec(QSparqlQuery(query));
    while (r->next())
    {
        insertOrder.append(r->value(0).toString());
        insertOrder.append(r->value(1).toString());
    }

    // Verify the data in the model
    for (int i=0;i<model.rowCount();i++) {
        QSparqlResultRow row = model.resultRow(i);
        QCOMPARE(insertOrder.at(0), row.value(0).toString());
        // inserted as uri, then name, so just pop the front
        insertOrder.pop_front();
        QCOMPARE(insertOrder.at(0), row.value(1).toString());
        insertOrder.pop_front();
    }

    delete r;
}

void tst_QSparqlAPI::queryModel_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<QString>("query");
    QTest::addColumn<int>("expectedResultsSize");

    QTest::newRow("DBus Query Model")
        << "QTRACKER"
        << contactSelectQueryTemplate.arg("")
        << NUM_INSERTS;

    QTest::newRow("Tracker Direct Query Model")
        << "QTRACKER_DIRECT"
        << contactSelectQueryTemplate.arg("")
        << NUM_INSERTS;

    if (testEndpoint) {
        QTest::newRow("Endpoint Query Model")
            << "QSPARQL_ENDPOINT"
            << contactSelectQueryTemplate.arg( getTemplateArguments("QSPARQL_ENDPOINT", "SELECT") )
            << NUM_INSERTS;
    }
}

void tst_QSparqlAPI::syncExec_waitForFinished_query_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, query);
    QFETCH(int, expectedResultsSize);

    QSparqlConnectionOptions options = getConnectionOptions(connectionDriver);
    QSparqlConnection conn(connectionDriver, options);
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
        << contactSelectQueryTemplate.arg("")
        << NUM_INSERTS;

    QTest::newRow("DBus Sync Select")
        << "QTRACKER"
        << contactSelectQueryTemplate.arg("")
        << NUM_INSERTS;

    if (testEndpoint) {
        QTest::newRow("Endpoint Sync Select")
            << "QSPARQL_ENDPOINT"
            << contactSelectQueryTemplate.arg( getTemplateArguments("QSPARQL_ENDPOINT", "SELECT") )
            << NUM_INSERTS;
    }
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

    QSparqlConnectionOptions options = getConnectionOptions(connectionDriver);
    QSparqlConnection conn(connectionDriver, options);
    const int expectedResultsSize = initialSize + contactInserts;

    QString insertQuery = contactInsertHeader.arg( getTemplateArguments(connectionDriver, "INSERT") );
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
    r = conn.syncExec(QSparqlQuery(validateQuery));
    r->waitForFinished();
    validateResults(r, expectedResultsSize);
    delete r;

    // Delete the insertion
    QString deleteQuery = contactDeleteHeader.arg( getTemplateArguments(connectionDriver, "DELETE") );
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
        << contactSelectQueryTemplate.arg("")
        << NUM_INSERTS
        << contactInsertAmount
        << contactDeleteAmount;

    QTest::newRow("DBus Sync Update Query")
        << "QTRACKER"
        << contactInsertQueryTemplate
        << contactDeleteQueryTemplate
        << contactSelectQueryTemplate.arg("")
        << NUM_INSERTS
        << contactInsertAmount
        << contactDeleteAmount;

    if (testEndpoint) {
        QTest::newRow("Endpoint Sync Update Query")
            << "QSPARQL_ENDPOINT"
            << contactInsertQueryTemplate
            << contactDeleteQueryTemplate
            << contactSelectQueryTemplate.arg( getTemplateArguments("QSPARQL_ENDPOINT", "SELECT") )
            << NUM_INSERTS
            << contactInsertAmount
            << contactDeleteAmount;
    }
}

void tst_QSparqlAPI::query_with_prefix()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, queryTemplate);
    QFETCH(int, expectedResultsSize);
    QFETCH(int, executionMethod);
    QFETCH(bool, useAsyncObject);
    
    QSparqlConnectionOptions options = getConnectionOptions(connectionDriver);
    QSparqlConnection conn(connectionDriver, options);
    conn.addPrefix("alias", QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#"));
    
    QSparqlQueryOptions queryOptions;
    queryOptions.setExecutionMethod(QSparqlQueryOptions::ExecutionMethod(executionMethod));
    
    const QString queryString = queryTemplate.arg( getTemplateArguments(connectionDriver, "SELECT") ).
                                replace(QString("nco"), QString("alias"));
    const QSparqlQuery q(queryString);
    
    QSparqlResult* r = conn.exec(q, queryOptions);
    checkExecutionMethod(r, executionMethod, useAsyncObject);
    validateResults(r, expectedResultsSize);
    
    conn.clearPrefixes();
    
    r = conn.exec(q, queryOptions);
    checkExecutionMethod(r, executionMethod, useAsyncObject);
    QVERIFY(r->hasError());
    
    delete r;
}

void tst_QSparqlAPI::query_with_prefix_data()
{
    query_test_data();
}

void tst_QSparqlAPI::go_beyond_columns_number()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, queryTemplate);
    QFETCH(int, expectedResultsSize);
    QFETCH(int, executionMethod);
    QFETCH(bool, useAsyncObject);

    QSparqlConnectionOptions options = getConnectionOptions(connectionDriver);
    QSparqlConnection conn(connectionDriver, options);

    QSparqlQueryOptions queryOptions;
    queryOptions.setExecutionMethod(QSparqlQueryOptions::ExecutionMethod(executionMethod));

    const QString queryString = queryTemplate.arg( getTemplateArguments(connectionDriver, "SELECT") );
    const QSparqlQuery q(queryString);

    QSparqlResult* r = conn.exec(q, queryOptions);
    checkExecutionMethod(r, executionMethod, useAsyncObject);

    while (r->next()) {
        QCOMPARE(r->current().count(), 2);
        QCOMPARE(r->value(expectedResultsSize+1).toString(), QString());
        QCOMPARE(r->binding(expectedResultsSize+1).toString(), QString());
        QCOMPARE(r->binding(-2).toString(), QString());
    }
    delete r;
}

void tst_QSparqlAPI::go_beyond_columns_number_data()
{
    query_test_data();
}

QTEST_MAIN( tst_QSparqlAPI )
#include "tst_qsparql_api.moc"
