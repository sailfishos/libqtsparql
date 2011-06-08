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

// define the amount of data we are going to insert for the tests
#define NUMBER_OF_INSERTS 15

//const QString qtest(qTableName( "qtest", __FILE__ )); // FIXME: what's this

//TESTED_FILES=

class MessageRecorder {
public:
    MessageRecorder()
    {
        selfPtr = this;
        prevMsgHandler = qInstallMsgHandler(&MessageRecorder::msgHandler);
    }

    ~MessageRecorder()
    {
        qInstallMsgHandler(prevMsgHandler);
        selfPtr = 0;
    }

    void addMsgTypeToRecord(QtMsgType type)      { msgsToRecord.insert(type);    }
    bool hasMsgsOfType(QtMsgType type) const     { return !msgs[type].isEmpty(); }
    QStringList msgsOfType(QtMsgType type) const { return msgs[type];            }
    QStringList operator[](QtMsgType type) const { return msgs[type];            }

private:
    static MessageRecorder* selfPtr;

    void handleMsg(QtMsgType type, const char *msg)
    {
        if (msgsToRecord.contains(type))
            msgs[type] << QString(msg);
        else
            (*prevMsgHandler)(type, msg);
    }

    static void msgHandler(QtMsgType type, const char *msg)
    {
        selfPtr->handleMsg(type, msg);
    }

private:
    QtMsgHandler prevMsgHandler;
    QSet<QtMsgType> msgsToRecord;
    QMap<QtMsgType, QStringList> msgs;
};
MessageRecorder* MessageRecorder::selfPtr = 0;

class tst_QSparqlAPI : public QObject
{
    Q_OBJECT

public:
    tst_QSparqlAPI();
    virtual ~tst_QSparqlAPI();

    MessageRecorder *msgRecorder;
    //void validateResults(QSparqlResult* result, const int expectedResultsSize);

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void query_test();
    void query_test_data();

    void query_error_test();
    void query_error_test_data();

    void update_query_test();
    void update_query_test_data();

    void update_query_error_test();
    void update_query_error_test_data();

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
};

namespace {

const QString contactSelectQuery =
    "select ?u ?ng {?u a nco:PersonContact; "
    "nie:isLogicalPartOf <qsparql-api-tests> ;"
    "nco:nameGiven ?ng .}";

const QString contactSelectNothingQuery =
    "select ?u ?ng {?u a nco:PersonContact; "
    "nie:isLogicalPartOf <qsparql-api-tests-not-here> ;"
    "nco:nameGiven ?ng .}";

const int contactSelectColumnCount = 2;

const QString contactInsertQuery =
    "insert { <uri00%1> a nco:PersonContact; "
    "nie:isLogicalPartOf <qsparql-api-tests> ;"
    "nco:nameGiven \"name00%1\" .}";

const QString contactDeleteQuery =
    "delete { <uri00%1> a rdfs:Resource. }";

const QString askQueryTrue =
    "ask { <uri001> a nco:PersonContact, nie:InformationElement ;"
    "nie:isLogicalPartOf <qsparql-api-tests> ;"
    "nco:nameGiven \"name001\" .}";

const QString askQueryFalse =
    "ask { <uri001> a nco:PersonContact, nie:InformationElement ;"
    "nie:isLogicalPartOf <qsparql-api-tests> ;"
    "nco:nameGiven \"name002\" .}";

const QString constructQuery =
    "CONSTRUCT { ?s ?p ?o } WHERE { GRAPH <http://example.org/aGraph> { ?s ?p ?o } . }";

    class MySignalObject : public QObject
    {
        Q_OBJECT
        public slots:
            void onFinished() { emit finished(); }
        signals:
            void finished();
    };

void checkExecutionMethod(const int executionMethod, const bool asyncObject, QSparqlResult* r)
{
    if (executionMethod == QSparqlQueryOptions::AsyncExec) {
        if (asyncObject) {
            // As per documentation requirement, only attempt to connect the
            // signals after first validating that there is no error
            if (!r->hasError()) {
                MySignalObject signalObject;
                QObject::connect(r, SIGNAL(finished()), &signalObject, SLOT(onFinished()));
                QSignalSpy spy(&signalObject, SIGNAL(finished()));
                while (spy.count() == 0) {
                    QTest::qWait(100);
                }
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
    while (r->next()) {
        contactNamesValue[r->value(0).toString()] = r->value(1).toString();
        contactNamesBindings[r->binding(0).value().toString()] = r->binding(1).value().toString();
        contactNamesStringValue[r->stringValue(0)] = r->stringValue(1);
    }
    QVERIFY(r->pos() == QSparql::AfterLastRow);
    // Verify that the values are correct, and also verify that the values
    // are correct between usage
    QCOMPARE(contactNamesValue.size(), expectedResultsSize);
    QCOMPARE(contactNamesBindings.size(), expectedResultsSize);
    QCOMPARE(contactNamesStringValue.size(), expectedResultsSize);
    // Checks for if we expect some results
    if (expectedResultsSize != 0) {
        for(int i=1;i<=expectedResultsSize;i++)
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
            QVERIFY(r->pos() == QSparql::AfterLastRow);
            QVERIFY(r->previous());
            QVERIFY(r->pos() != QSparql::AfterLastRow);
            for (int i=expectedResultsSize;i>=1;i--) {
                QCOMPARE(contactNamesValue[r->value(0).toString()], r->value(1).toString());
                QCOMPARE(contactNamesBindings[r->binding(0).value().toString()], r->binding(1).value().toString());
                QCOMPARE(contactNamesStringValue[r->stringValue(0)], r->stringValue(1));
                if (i>1)
                    QVERIFY(r->previous());
                else
                    QVERIFY(!r->previous());
            }

            // Now move forward one...
            QVERIFY(r->pos() == QSparql::BeforeFirstRow);
            QVERIFY(r->next());
            QVERIFY(r->pos() != QSparql::BeforeFirstRow);
            QCOMPARE(contactNamesValue["uri001"], r->value(1).toString());
            QCOMPARE(contactNamesBindings["uri001"], r->binding(1).value().toString());
            QCOMPARE(contactNamesStringValue["uri001"], r->stringValue(1));
        }
    }
    // TODO: Figure out some sane checks/changes that are needed for r->next(), r->previous()
    // behaviour on queries with no results
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

    // clean any remainings
    cleanupTestCase();

    const QString insertQueryTemplate =
        "<uri00%1> a nco:PersonContact, nie:InformationElement ;"
        "nie:isLogicalPartOf <qsparql-api-tests> ;"
        "nco:nameGiven \"name00%1\" .";
    QString insertQuery = "insert { <qsparql-api-tests> a nie:InformationElement .";
    for (int item = 1; item <= NUMBER_OF_INSERTS; item++) {
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

void tst_QSparqlAPI::cleanupTestCase()
{
    QSparqlConnection conn("QTRACKER");
    QSparqlQuery q("DELETE { ?u a rdfs:Resource . } "
                    "WHERE { ?u nie:isLogicalPartOf <qsparql-api-tests> . }"
                    "DELETE { <qsparql-api-tests> a rdfs:Resource . }",
                    QSparqlQuery::DeleteStatement);
    QSparqlResult* r = conn.exec(q);
    CHECK_QSPARQL_RESULT(r);
    r->waitForFinished();
    CHECK_QSPARQL_RESULT(r);
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
    QFETCH(int, executionMethod);
    QFETCH(bool, asyncObject);
    QFETCH(int, expectedResultsSize);

    QSparqlConnection conn(connectionDriver);

    QSparqlQueryOptions queryOptions;
    queryOptions.setExecutionMethod((QSparqlQueryOptions::ExecutionMethod)executionMethod);
    QSparqlQuery q(query);

    QSparqlResult* r = conn.exec(q,queryOptions);
    CHECK_QSPARQL_RESULT(r);

    checkExecutionMethod(executionMethod, asyncObject, r);

    CHECK_QSPARQL_RESULT(r);

    validateResults(r, expectedResultsSize);

    delete r;
}

void tst_QSparqlAPI::query_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<QString>("query");
    QTest::addColumn<int>("expectedResultsSize");
    QTest::addColumn<int>("executionMethod");
    QTest::addColumn<bool>("asyncObject");

    QTest::newRow("DBus Async Query")
        << "QTRACKER"
        << contactSelectQuery
        << NUMBER_OF_INSERTS
        << (int)QSparqlQueryOptions::AsyncExec
        << false;

    QTest::newRow("DBus Async No Results Query")
        << "QTRACKER"
        << contactSelectNothingQuery
        << 0
        << (int)QSparqlQueryOptions::AsyncExec
        << false;

    QTest::newRow("DBus Sync Query")
        << "QTRACKER"
        << contactSelectQuery
        << NUMBER_OF_INSERTS
        << (int)QSparqlQueryOptions::SyncExec
        << false;

    QTest::newRow("DBus Sync No Results Query")
        << "QTRACKER"
        << contactSelectNothingQuery
        << 0
        << (int)QSparqlQueryOptions::SyncExec
        << false;

    QTest::newRow("DBus Async Object Query")
        << "QTRACKER"
        << contactSelectQuery
        << NUMBER_OF_INSERTS
        << (int)QSparqlQueryOptions::AsyncExec
        << true;

    QTest::newRow("DBus Async Object No Results Query")
        << "QTRACKER"
        << contactSelectNothingQuery
        << 0
        << (int)QSparqlQueryOptions::AsyncExec
        << true;

    QTest::newRow("Tracker Direct Async Query")
        << "QTRACKER_DIRECT"
        << contactSelectQuery
        << NUMBER_OF_INSERTS
        << (int)QSparqlQueryOptions::AsyncExec
        << false;

    QTest::newRow("Tracker Direct Async No Results Query")
        << "QTRACKER_DIRECT"
        << contactSelectNothingQuery
        << 0
        << (int)QSparqlQueryOptions::AsyncExec
        << false;

    QTest::newRow("Tracker Direct Async Object Query")
        << "QTRACKER_DIRECT"
        << contactSelectQuery
        << NUMBER_OF_INSERTS
        << (int)QSparqlQueryOptions::AsyncExec
        << true;

    QTest::newRow("Tracker Direct Async Object No Results Query")
        << "QTRACKER_DIRECT"
        << contactSelectNothingQuery
        << 0
        << (int)QSparqlQueryOptions::AsyncExec
        << true;

    QTest::newRow("Tracker Direct Sync Query")
        << "QTRACKER_DIRECT"
        << contactSelectQuery
        << NUMBER_OF_INSERTS
        << (int)QSparqlQueryOptions::SyncExec
        << false;

    QTest::newRow("Tracker Direct Sync No Results Query")
        << "QTRACKER_DIRECT"
        << contactSelectNothingQuery
        << 0
        << (int)QSparqlQueryOptions::SyncExec
        << false;

}

void tst_QSparqlAPI::query_error_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, query);
    QFETCH(int, executionMethod);
    QFETCH(int, expectedErrorType);
    QFETCH(bool, asyncObject);

    QSparqlConnection conn(connectionDriver);

    QSparqlQueryOptions queryOptions;
    queryOptions.setExecutionMethod((QSparqlQueryOptions::ExecutionMethod)executionMethod);
    QSparqlQuery q(query);

    QSparqlResult* r = conn.exec(q,queryOptions);

    checkExecutionMethod(executionMethod, asyncObject, r);

    QCOMPARE((*msgRecorder)[QtWarningMsg].count(), 1);
    QVERIFY(r->lastError().type() == (QSparqlError::ErrorType)expectedErrorType);
    QVERIFY(r->hasError());

    delete r;
}

void tst_QSparqlAPI::query_error_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<QString>("query");
    QTest::addColumn<int>("executionMethod");
    QTest::addColumn<int>("expectedErrorType");
    QTest::addColumn<bool>("asyncObject");

    QTest::newRow("DBus Blank Query")
        << "QTRACKER"
        << ""
        << (int)QSparqlQueryOptions::AsyncExec
        << (int)QSparqlError::StatementError
        << false;

   QTest::newRow("DBus Blank Async Object Query")
        << "QTRACKER"
        << ""
        << (int)QSparqlQueryOptions::AsyncExec
        << (int)QSparqlError::StatementError
        << true;

    QTest::newRow("DBus Invalid Query")
        << "QTRACKER"
        << "invalid query"
        << (int)QSparqlQueryOptions::AsyncExec
        << (int)QSparqlError::StatementError
        << false;

    QTest::newRow("DBus Invalid Async Object Query")
        << "QTRACKER"
        << "invalid query"
        << (int)QSparqlQueryOptions::AsyncExec
        << (int)QSparqlError::StatementError
        << true;

    QTest::newRow("DBus Invalid Construct Query")
        << "QTRACKER"
        << constructQuery
        << (int)QSparqlQueryOptions::AsyncExec
        << (int)QSparqlError::BackendError
        << false;

    QTest::newRow("DBus Invalid Construct Async Object Query")
        << "QTRACKER"
        << constructQuery
        << (int)QSparqlQueryOptions::AsyncExec
        << (int)QSparqlError::BackendError
        << true;

    QTest::newRow("Tracker Direct Async Blank Query")
        << "QTRACKER_DIRECT"
        << ""
        << (int)QSparqlQueryOptions::AsyncExec
        << (int)QSparqlError::StatementError
        << false;

    QTest::newRow("Tracker Direct Async Object Blank Query")
        << "QTRACKER_DIRECT"
        << ""
        << (int)QSparqlQueryOptions::AsyncExec
        << (int)QSparqlError::StatementError
        << true;

    QTest::newRow("Tracker Direct Sync Blank Query")
        << "QTRACKER_DIRECT"
        << ""
        << (int)QSparqlQueryOptions::SyncExec
        << (int)QSparqlError::StatementError
        << false;

    QTest::newRow("Tracker Direct Async Invalid Query")
        << "QTRACKER_DIRECT"
        << "invalid query"
        << (int)QSparqlQueryOptions::AsyncExec
        << (int)QSparqlError::StatementError
        << false;

    QTest::newRow("Tracker Direct Async Object Invalid Query")
        << "QTRACKER_DIRECT"
        << "invalid query"
        << (int)QSparqlQueryOptions::AsyncExec
        << (int)QSparqlError::StatementError
        << true;

    QTest::newRow("Tracker Direct Sync Invalid Query")
        << "QTRACKER_DIRECT"
        << "invalid query"
        << (int)QSparqlQueryOptions::SyncExec
        << (int)QSparqlError::StatementError
        << false;

    QTest::newRow("Tracker Direct Async Construct Query")
        << "QTRACKER_DIRECT"
        << constructQuery
        << (int)QSparqlQueryOptions::AsyncExec
        << (int)QSparqlError::BackendError
        << false;

    QTest::newRow("Tracker Direct Async Object Construct Query")
        << "QTRACKER_DIRECT"
        << constructQuery
        << (int)QSparqlQueryOptions::AsyncExec
        << (int)QSparqlError::BackendError
        << true;

    QTest::newRow("Tracker Direct Sync Construct Query")
        << "QTRACKER_DIRECT"
        << constructQuery
        << (int)QSparqlQueryOptions::SyncExec
        << (int)QSparqlError::BackendError
        << false;
}

void tst_QSparqlAPI::update_query_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, query);
    QFETCH(int, expectedResultsSize);
    QFETCH(int, executionMethod);
    QFETCH(bool, asyncObject);

    QSparqlConnection conn(connectionDriver);

    QSparqlQueryOptions queryOptions;
    queryOptions.setExecutionMethod((QSparqlQueryOptions::ExecutionMethod)executionMethod);
    QSparqlQuery q(query.arg(expectedResultsSize), QSparqlQuery::InsertStatement);

    QSparqlResult* r = conn.exec(q,queryOptions);
    CHECK_QSPARQL_RESULT(r);

    checkExecutionMethod(executionMethod, asyncObject, r);

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
    CHECK_QSPARQL_RESULT(r);
    delete r;

    // Verify the insertion
    r = conn.exec(QSparqlQuery(contactSelectQuery), queryOptions);

    checkExecutionMethod(executionMethod, asyncObject, r);
    validateResults(r, expectedResultsSize);

    delete r;

    r = conn.exec(QSparqlQuery(QString("delete { <uri00%1> a rdfs:Resource. }").arg(expectedResultsSize),
                  QSparqlQuery::DeleteStatement), queryOptions);

    CHECK_QSPARQL_RESULT(r);

    checkExecutionMethod(executionMethod, asyncObject, r);

    CHECK_QSPARQL_RESULT(r);
    delete r;

    // Now verify deletion
    r = conn.exec(QSparqlQuery(contactSelectQuery), queryOptions);

    checkExecutionMethod(executionMethod, asyncObject, r);
    validateResults(r, expectedResultsSize-1);
    delete r;
}

void tst_QSparqlAPI::update_query_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<QString>("query");
    QTest::addColumn<int>("expectedResultsSize");
    QTest::addColumn<int>("executionMethod");
    QTest::addColumn<bool>("asyncObject");

    QTest::newRow("DBus Update Query")
        << "QTRACKER"
        << contactInsertQuery
        << NUMBER_OF_INSERTS + 1
        << (int)QSparqlQueryOptions::AsyncExec
        << false;

    QTest::newRow("DBus Update Async Object Query")
        << "QTRACKER"
        << contactInsertQuery
        << NUMBER_OF_INSERTS + 1
        << (int)QSparqlQueryOptions::AsyncExec
        << true;

    QTest::newRow("Tracker Direct Async Update Query")
        << "QTRACKER_DIRECT"
        << contactInsertQuery
        << NUMBER_OF_INSERTS + 1
        << (int)QSparqlQueryOptions::AsyncExec
        << false;

    QTest::newRow("Tracker Direct Async Object Update Query")
        << "QTRACKER_DIRECT"
        << contactInsertQuery
        << NUMBER_OF_INSERTS + 1
        << (int)QSparqlQueryOptions::AsyncExec
        << true;

    QTest::newRow("Tracker Direct Sync Update Query")
        << "QTRACKER_DIRECT"
        << contactInsertQuery
        << NUMBER_OF_INSERTS + 1
        << (int)QSparqlQueryOptions::SyncExec
        << false;

}

void tst_QSparqlAPI::update_query_error_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, query);
    QFETCH(int, executionMethod);
    QFETCH(int, expectedErrorType);
    QFETCH(bool, asyncObject);

    QSparqlConnection conn(connectionDriver);

    QSparqlQueryOptions queryOptions;
    queryOptions.setExecutionMethod((QSparqlQueryOptions::ExecutionMethod)executionMethod);
    QSparqlQuery q(query, QSparqlQuery::InsertStatement);

    QSparqlResult* r = conn.exec(q,queryOptions);

    checkExecutionMethod(executionMethod, asyncObject, r);

    QVERIFY(r->lastError().type() == (QSparqlError::ErrorType)expectedErrorType);
    QVERIFY(r->hasError());
    // Check we got a warning
    QCOMPARE((*msgRecorder)[QtWarningMsg].count(), 1);

    delete r;

    // also check delete statments
    QSparqlQuery delQuery(query, QSparqlQuery::DeleteStatement);

    r = conn.exec(delQuery,queryOptions);
    checkExecutionMethod(executionMethod, asyncObject, r);

    QVERIFY(r->lastError().type() == (QSparqlError::ErrorType)expectedErrorType);
    QVERIFY(r->hasError());
    // Second warning
    QCOMPARE((*msgRecorder)[QtWarningMsg].count(), 2);
    delete r;
}

void tst_QSparqlAPI::update_query_error_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<QString>("query");
    QTest::addColumn<int>("executionMethod");
    QTest::addColumn<int>("expectedErrorType");
    QTest::addColumn<bool>("asyncObject");

    QTest::newRow("DBus Blank Query")
        << "QTRACKER"
        << ""
        << (int)QSparqlQueryOptions::AsyncExec
        << (int)QSparqlError::StatementError
        << false;

   QTest::newRow("DBus Blank Async Object Query")
        << "QTRACKER"
        << ""
        << (int)QSparqlQueryOptions::AsyncExec
        << (int)QSparqlError::StatementError
        << true;

    QTest::newRow("DBus Invalid Query")
        << "QTRACKER"
        << "invalid query"
        << (int)QSparqlQueryOptions::AsyncExec
        << (int)QSparqlError::StatementError
        << false;

    QTest::newRow("DBus Invalid Async Object Query")
        << "QTRACKER"
        << "invalid query"
        << (int)QSparqlQueryOptions::AsyncExec
        << (int)QSparqlError::StatementError
        << true;

    QTest::newRow("DBus Invalid Construct Query")
        << "QTRACKER"
        << constructQuery
        << (int)QSparqlQueryOptions::AsyncExec
        << (int)QSparqlError::StatementError
        << false;

    QTest::newRow("DBus Invalid Construct Async Object Query")
        << "QTRACKER"
        << constructQuery
        << (int)QSparqlQueryOptions::AsyncExec
        << (int)QSparqlError::StatementError
        << true;

    QTest::newRow("Tracker Direct Async Blank Query")
        << "QTRACKER_DIRECT"
        << ""
        << (int)QSparqlQueryOptions::AsyncExec
        << (int)QSparqlError::StatementError
        << false;

    QTest::newRow("Tracker Direct Async Object Blank Query")
        << "QTRACKER_DIRECT"
        << ""
        << (int)QSparqlQueryOptions::AsyncExec
        << (int)QSparqlError::StatementError
        << true;

    QTest::newRow("Tracker Direct Sync Blank Query")
        << "QTRACKER_DIRECT"
        << ""
        << (int)QSparqlQueryOptions::SyncExec
        << (int)QSparqlError::StatementError
        << false;

    QTest::newRow("Tracker Direct Async Invalid Query")
        << "QTRACKER_DIRECT"
        << "invalid query"
        << (int)QSparqlQueryOptions::AsyncExec
        << (int)QSparqlError::StatementError
        << false;

    QTest::newRow("Tracker Direct Async Object Invalid Query")
        << "QTRACKER_DIRECT"
        << "invalid query"
        << (int)QSparqlQueryOptions::AsyncExec
        << (int)QSparqlError::StatementError
        << true;

    QTest::newRow("Tracker Direct Sync Invalid Query")
        << "QTRACKER_DIRECT"
        << "invalid query"
        << (int)QSparqlQueryOptions::SyncExec
        << (int)QSparqlError::StatementError
        << false;

    QTest::newRow("Tracker Direct Async Construct Query")
        << "QTRACKER_DIRECT"
        << constructQuery
        << (int)QSparqlQueryOptions::AsyncExec
        << (int)QSparqlError::StatementError
        << false;

    QTest::newRow("Tracker Direct Async Object Construct Query")
        << "QTRACKER_DIRECT"
        << constructQuery
        << (int)QSparqlQueryOptions::AsyncExec
        << (int)QSparqlError::StatementError
        << true;

    QTest::newRow("Tracker Direct Sync Construct Query")
        << "QTRACKER_DIRECT"
        << constructQuery
        << (int)QSparqlQueryOptions::SyncExec
        << (int)QSparqlError::StatementError
        << false;
}


void tst_QSparqlAPI::ask_query_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, query);
    QFETCH(int, executionMethod);
    QFETCH(bool, expectedResult);
    QFETCH(bool, asyncObject);

    QSparqlConnection conn(connectionDriver);

    if (conn.hasFeature(QSparqlConnection::AskQueries)) {

        QSparqlQuery q(query, QSparqlQuery::AskStatement);

        QSparqlQueryOptions queryOptions;
        queryOptions.setExecutionMethod((QSparqlQueryOptions::ExecutionMethod)executionMethod);

        QSparqlResult *r = conn.exec(q, queryOptions);
        CHECK_QSPARQL_RESULT(r);

        checkExecutionMethod(executionMethod, asyncObject, r);

        while (!r->isFinished())
            r->next();

        QVERIFY(r->isFinished());
        QCOMPARE(r->boolValue(), expectedResult);

        delete r;
    }
}

void tst_QSparqlAPI::ask_query_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<int>("executionMethod");
    QTest::addColumn<QString>("query");
    QTest::addColumn<bool>("expectedResult");
    QTest::addColumn<bool>("asyncObject");

    QTest::newRow("Tracker Direct Async True Query")
        << "QTRACKER_DIRECT"
        << (int)QSparqlQueryOptions::AsyncExec
        << askQueryTrue
        << true
        << false;

    QTest::newRow("Tracker Direct Async Object True Query")
        << "QTRACKER_DIRECT"
        << (int)QSparqlQueryOptions::AsyncExec
        << askQueryTrue
        << true
        << true;

    QTest::newRow("Tracker Direct Sync True Query")
        << "QTRACKER_DIRECT"
        << (int)QSparqlQueryOptions::SyncExec
        << askQueryTrue
        << true
        << false;

    QTest::newRow("Tracker Direct Async False Query")
        << "QTRACKER_DIRECT"
        << (int)QSparqlQueryOptions::AsyncExec
        << askQueryFalse
        << false
        << false;

    QTest::newRow("Tracker Direct Async Object False Query")
        << "QTRACKER_DIRECT"
        << (int)QSparqlQueryOptions::AsyncExec
        << askQueryFalse
        << false
        << true;

    QTest::newRow("Tracker Direct Sync False Query")
        << "QTRACKER_DIRECT"
        << (int)QSparqlQueryOptions::SyncExec
        << askQueryFalse
        << false
        << false;

    QTest::newRow("DBus Async True Ask Query")
        << "QTRACKER"
        << (int)QSparqlQueryOptions::AsyncExec
        << askQueryTrue
        << true
        << false;

    QTest::newRow("DBus Async True Ask Async Object Query")
        << "QTRACKER"
        << (int)QSparqlQueryOptions::AsyncExec
        << askQueryTrue
        << true
        << true;

    QTest::newRow("DBus Async False Ask Query")
        << "QTRACKER"
        << (int)QSparqlQueryOptions::AsyncExec
        << askQueryFalse
        << false
        << false;

    QTest::newRow("DBus Async False Ask Async Object Query")
        << "QTRACKER"
        << (int)QSparqlQueryOptions::AsyncExec
        << askQueryFalse
        << false
        << true;

}

void tst_QSparqlAPI::isFinished_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, query);
    QFETCH(int, executionMethod);
    QFETCH(bool, asyncObject);

    QSparqlConnection conn(connectionDriver);

    QSparqlQueryOptions queryOptions;
    queryOptions.setExecutionMethod((QSparqlQueryOptions::ExecutionMethod)executionMethod);
    QSparqlQuery q(query);

    QSparqlResult* r = conn.exec(q,queryOptions);
    CHECK_QSPARQL_RESULT(r);

    checkExecutionMethod(executionMethod, asyncObject, r);

    CHECK_QSPARQL_RESULT(r);

    // According to the documentation, isFinished() will be false for sync queries
    // until the results have been iterated
    if (executionMethod == QSparqlQueryOptions::SyncExec) {
        QVERIFY(!r->isFinished());
        validateResults(r, NUMBER_OF_INSERTS);
        QVERIFY(r->isFinished());
    } else {
        QVERIFY(r->isFinished());
        validateResults(r, NUMBER_OF_INSERTS);
    }

    delete r;
}

void tst_QSparqlAPI::isFinished_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<QString>("query");
    QTest::addColumn<int>("executionMethod");
    QTest::addColumn<bool>("asyncObject");

    QTest::newRow("DBus Async Query")
        << "QTRACKER"
        << contactSelectQuery
        << (int)QSparqlQueryOptions::AsyncExec
        << false;

   QTest::newRow("DBus Async Object Query")
        << "QTRACKER"
        << contactSelectQuery
        << (int)QSparqlQueryOptions::AsyncExec
        << true;

    QTest::newRow("Tracker Direct Async Query")
        << "QTRACKER_DIRECT"
        << contactSelectQuery
        << (int)QSparqlQueryOptions::AsyncExec
        << false;

    QTest::newRow("Tracker Direct Async Object Query")
        << "QTRACKER_DIRECT"
        << contactSelectQuery
        << (int)QSparqlQueryOptions::AsyncExec
        << true;

    QTest::newRow("Tracker Direct Sync Query")
        << "QTRACKER_DIRECT"
        << contactSelectQuery
        << (int)QSparqlQueryOptions::SyncExec
        << false;
}

void tst_QSparqlAPI::result_iteration_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, query);
    QFETCH(int, executionMethod);
    QFETCH(bool, asyncObject);

    QSparqlConnection conn(connectionDriver);

    QSparqlQueryOptions queryOptions;
    queryOptions.setExecutionMethod((QSparqlQueryOptions::ExecutionMethod)executionMethod);
    QSparqlQuery q(query);

    QSparqlResult* r = conn.exec(q,queryOptions);
    CHECK_QSPARQL_RESULT(r);

    checkExecutionMethod(executionMethod, asyncObject, r);

    CHECK_QSPARQL_RESULT(r);

    validateResults(r, NUMBER_OF_INSERTS);

    delete r;
}

void tst_QSparqlAPI::result_iteration_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<QString>("query");
    QTest::addColumn<int>("executionMethod");
    QTest::addColumn<bool>("asyncObject");

    QTest::newRow("DBus Async Query")
        << "QTRACKER"
        << contactSelectQuery
        << (int)QSparqlQueryOptions::AsyncExec
        << false;

    QTest::newRow("DBus Async Object Query")
        << "QTRACKER"
        << contactSelectQuery
        << (int)QSparqlQueryOptions::AsyncExec
        << true;

    QTest::newRow("Tracker Direct Async Query")
        << "QTRACKER_DIRECT"
        << contactSelectQuery
        << (int)QSparqlQueryOptions::AsyncExec
        << false;

    QTest::newRow("Tracker Direct Async Object Query")
        << "QTRACKER_DIRECT"
        << contactSelectQuery
        << (int)QSparqlQueryOptions::AsyncExec
        << true;

    QTest::newRow("Tracker Direct Sync Query")
        << "QTRACKER_DIRECT"
        << contactSelectQuery
        << (int)QSparqlQueryOptions::SyncExec
        << false;
}

void tst_QSparqlAPI::queryModel_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, query);

    QSparqlConnection conn(connectionDriver);
    QSparqlQueryModel model;

    model.setQuery(QSparqlQuery(query), conn);

    QSignalSpy spy(&model, SIGNAL(finished()));
    while (spy.count() == 0)
        QTest::qWait(100);

    QCOMPARE(model.rowCount(), NUMBER_OF_INSERTS);
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

    QTest::newRow("DBus Query Model")
        << "QTRACKER"
        << contactSelectQuery;

    QTest::newRow("Tracker Direct Query Model")
        << "QTRACKER_DIRECT"
        << contactSelectQuery;
}

void tst_QSparqlAPI::syncExec_waitForFinished_query_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, query);
    QFETCH(int, expectedResultsSize);

    QSparqlConnection conn(connectionDriver);
    QSparqlQuery q(query);
    QSparqlResult* r = conn.syncExec(q);
    CHECK_QSPARQL_RESULT(r);
    r->waitForFinished();
    CHECK_QSPARQL_RESULT(r);

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
        << NUMBER_OF_INSERTS;
}

void tst_QSparqlAPI::syncExec_waitForFinished_update_query_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, query);
    QFETCH(int, expectedResultsSize);

    QSparqlConnection conn(connectionDriver);

    QSparqlQuery q(query.arg(expectedResultsSize), QSparqlQuery::InsertStatement);

    QSparqlResult* r = conn.syncExec(q);
    CHECK_QSPARQL_RESULT(r);
    r->waitForFinished();

    CHECK_QSPARQL_RESULT(r);
    delete r;

    // Verify the insertion
    r = conn.syncExec(QSparqlQuery(contactSelectQuery));
    r->waitForFinished();
    validateResults(r, expectedResultsSize);
    delete r;

    r = conn.syncExec(QSparqlQuery(QString("delete { <uri00%1> a rdfs:Resource. }").arg(expectedResultsSize),
                  QSparqlQuery::DeleteStatement));

    CHECK_QSPARQL_RESULT(r);
    r->waitForFinished();
    CHECK_QSPARQL_RESULT(r);
    delete r;

    // Now verify deletion
    r = conn.syncExec(QSparqlQuery(contactSelectQuery));
    r->waitForFinished();
    validateResults(r, expectedResultsSize-1);
    delete r;
}

void tst_QSparqlAPI::syncExec_waitForFinished_update_query_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<QString>("query");
    QTest::addColumn<int>("expectedResultsSize");

    QTest::newRow("Tracker Direct Async Update Query")
        << "QTRACKER_DIRECT"
        << contactInsertQuery
        << NUMBER_OF_INSERTS + 1;

}

QTEST_MAIN( tst_QSparqlAPI )
#include "tst_qsparql_api.moc"
