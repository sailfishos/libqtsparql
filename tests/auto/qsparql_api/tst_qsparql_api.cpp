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

//const QString qtest(qTableName( "qtest", __FILE__ )); // FIXME: what's this

//TESTED_FILES=

class tst_QSparqlAPI : public QObject
{
    Q_OBJECT

public:
    tst_QSparqlAPI();
    virtual ~tst_QSparqlAPI();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void query_test();
    void query_test_data();

    void query_test_asyncObject();
    void query_test_asyncObject_data();

    void ask_query();
    void ask_query_data();

    void isFinished_test();
    void isFinished_test_data();

    void isFinished_asyncObject_test();
    void isFinished_asyncObject_test_data();
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
const QString contactSelectQuery =
    "select ?u ?ng {?u a nco:PersonContact; "
    "nie:isLogicalPartOf <qsparql-api-tests> ;"
    "nco:nameGiven ?ng .}";
const QString askQueryTrue =
    "ask { <uri001> a nco:PersonContact, nie:InformationElement ;"
    "nie:isLogicalPartOf <qsparql-api-tests> ;"
    "nco:nameGiven \"name001\" .}";

const QString askQueryFalse =
    "ask { <uri001> a nco:PersonContact, nie:InformationElement ;"
    "nie:isLogicalPartOf <qsparql-api-tests> ;"
    "nco:nameGiven \"name002\" .}";

    class MySignalObject : public QObject
    {
        Q_OBJECT
        public slots:
            void onFinished() { emit finished(); }
        signals:
            void finished();
    };

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
    qInstallMsgHandler(myMessageOutput);

    // clean any remainings
    cleanupTestCase();

    const QString insertQueryTemplate =
        "<uri00%1> a nco:PersonContact, nie:InformationElement ;"
        "nie:isLogicalPartOf <qsparql-api-tests> ;"
        "nco:nameGiven \"name00%1\" .";
    QString insertQuery = "insert { <qsparql-api-tests> a nie:InformationElement .";
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
    testLogLevel = QtDebugMsg;
}

void tst_QSparqlAPI::cleanup()
{
}

void tst_QSparqlAPI::query_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, query);
    QFETCH(int, expectedResultsSize);
    QFETCH(int, executionMethod);

    QSparqlConnection conn(connectionDriver);

    QSparqlQueryOptions queryOptions;
    queryOptions.setExecutionMethod((QSparqlQueryOptions::ExecutionMethod)executionMethod);
    QSparqlQuery q(query);

    QSparqlResult* r = conn.exec(q,queryOptions);
    CHECK_QSPARQL_RESULT(r);

    if(executionMethod == QSparqlQueryOptions::AsyncExec)
        r->waitForFinished();

    CHECK_QSPARQL_RESULT(r);

    if(r->hasFeature(QSparqlResult::QuerySize))
        QCOMPARE(r->size(), expectedResultsSize);

    int resultCount = 0;
    while (r->next()) {
        QVERIFY(r->isValid());
        resultCount++;
    }
    QCOMPARE(resultCount, expectedResultsSize);
    delete r;
}

void tst_QSparqlAPI::query_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<QString>("query");
    QTest::addColumn<int>("expectedResultsSize");
    QTest::addColumn<int>("executionMethod");

    QTest::newRow("DBus Async Query")
        << "QTRACKER"
        << contactSelectQuery
        << 3
        << (int)QSparqlQueryOptions::AsyncExec;

    QTest::newRow("DBus Sync Query")
        << "QTRACKER"
        << contactSelectQuery
        << 3
        << (int)QSparqlQueryOptions::SyncExec;

    QTest::newRow("Tracker Direct Async Query")
        << "QTRACKER_DIRECT"
        << contactSelectQuery
        << 3
        << (int)QSparqlQueryOptions::AsyncExec;

    QTest::newRow("Tracker Direct Sync Query")
        << "QTRACKER_DIRECT"
        << contactSelectQuery
        << 3
        << (int)QSparqlQueryOptions::SyncExec;
}

void tst_QSparqlAPI::query_test_asyncObject()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, query);
    QFETCH(int, expectedResultsSize);

    QSparqlConnection conn(connectionDriver);
    QSparqlQuery q(query);

    MySignalObject signalObject;

    QSparqlResult *r = conn.exec(q);
    CHECK_QSPARQL_RESULT(r);
    connect(r, SIGNAL(finished()), &signalObject, SLOT(onFinished()));

    QSignalSpy spy(&signalObject, SIGNAL(finished()));
    while (spy.count() == 0) {
       QTest::qWait(100);
    }

    CHECK_QSPARQL_RESULT(r);

    if(r->hasFeature(QSparqlResult::QuerySize))
        QCOMPARE(r->size(), expectedResultsSize);

    int resultCount = 0;
    while (r->next()) {
        QVERIFY(r->isValid());
        resultCount++;
    }
    QCOMPARE(resultCount, expectedResultsSize);
    delete r;
}

void tst_QSparqlAPI::query_test_asyncObject_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<QString>("query");
    QTest::addColumn<int>("expectedResultsSize");

    QTest::newRow("DBus Async Query")
        << "QTRACKER"
        << contactSelectQuery
        << 3;

    QTest::newRow("Tracker Direct Async Query")
        << "QTRACKER_DIRECT"
        << contactSelectQuery
        << 3;
}

void tst_QSparqlAPI::ask_query()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, query);
    QFETCH(int, executionMethod);
    QFETCH(bool, expectedResult);

    QSparqlConnection conn(connectionDriver);

    if (conn.hasFeature(QSparqlConnection::AskQueries)) {

        QSparqlQuery q(query, QSparqlQuery::AskStatement);

        QSparqlQueryOptions queryOptions;
        queryOptions.setExecutionMethod((QSparqlQueryOptions::ExecutionMethod)executionMethod);

        QSparqlResult *r = conn.exec(q, queryOptions);
        CHECK_QSPARQL_RESULT(r);

        if (executionMethod == QSparqlQueryOptions::AsyncExec) {
            QEventLoop loop;
            connect(r, SIGNAL(finished()), &loop, SLOT(quit()));
            loop.exec();
        }

        while(!r->isFinished())
            r->next();

        QVERIFY(r->isFinished());
        QCOMPARE(r->boolValue(), expectedResult);

        delete r;
    }
}

void tst_QSparqlAPI::ask_query_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<int>("executionMethod");
    QTest::addColumn<QString>("query");
    QTest::addColumn<bool>("expectedResult");

    QTest::newRow("Tracker Direct Async True Query")
        << "QTRACKER_DIRECT"
        << (int)QSparqlQueryOptions::AsyncExec
        << askQueryTrue
        << true;

    QTest::newRow("Tracker Direct Sync True Query")
        << "QTRACKER_DIRECT"
        << (int)QSparqlQueryOptions::SyncExec
        << askQueryTrue
        << true;

    QTest::newRow("Tracker Direct Async False Query")
        << "QTRACKER_DIRECT"
        << (int)QSparqlQueryOptions::AsyncExec
        << askQueryFalse
        << false;

    QTest::newRow("Tracker Direct Sync False Query")
        << "QTRACKER_DIRECT"
        << (int)QSparqlQueryOptions::AsyncExec
        << askQueryFalse
        << false;

    QTest::newRow("DBus Async True Ask Query")
        << "QTRACKER"
        << (int)QSparqlQueryOptions::AsyncExec
        << askQueryTrue
        << true;

    QTest::newRow("DBus Async False Ask Query")
        << "QTRACKER"
        << (int)QSparqlQueryOptions::AsyncExec
        << askQueryFalse
        << false;
}

void tst_QSparqlAPI::isFinished_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, query);
    QFETCH(int, expectedResultsSize);
    QFETCH(int, executionMethod);

    QSparqlConnection conn(connectionDriver);

    QSparqlQueryOptions queryOptions;
    queryOptions.setExecutionMethod((QSparqlQueryOptions::ExecutionMethod)executionMethod);
    QSparqlQuery q(query);

    QSparqlResult* r = conn.exec(q,queryOptions);
    CHECK_QSPARQL_RESULT(r);

    if (executionMethod == QSparqlQueryOptions::AsyncExec)
        r->waitForFinished();

    CHECK_QSPARQL_RESULT(r);

    // According to the documentation, isFinished() will be false for sync queries
    // until the results have been iterated
    if (executionMethod == QSparqlQueryOptions::SyncExec) {
        QVERIFY(!r->isFinished());
        int resultCount = 0;

        while (r->next())
            resultCount++;

        QVERIFY(r->isFinished());
        QCOMPARE(resultCount, expectedResultsSize);
    }
    else {
        QVERIFY(r->isFinished());
        // Async results should support size()
        QCOMPARE(r->size(), expectedResultsSize);
    }

    delete r;
}

void tst_QSparqlAPI::isFinished_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<QString>("query");
    QTest::addColumn<int>("expectedResultsSize");
    QTest::addColumn<int>("executionMethod");

    QTest::newRow("DBus Async Query")
        << "QTRACKER"
        << contactSelectQuery
        << 3
        << (int)QSparqlQueryOptions::AsyncExec;

    QTest::newRow("Tracker Direct Async Query")
        << "QTRACKER_DIRECT"
        << contactSelectQuery
        << 3
        << (int)QSparqlQueryOptions::AsyncExec;

    QTest::newRow("Tracker Direct Sync Query")
        << "QTRACKER_DIRECT"
        << contactSelectQuery
        << 3
        << (int)QSparqlQueryOptions::SyncExec;
}

void tst_QSparqlAPI::isFinished_asyncObject_test()
{
    QFETCH(QString, connectionDriver);
    QFETCH(QString, query);
    QFETCH(int, expectedResultsSize);

    QSparqlConnection conn(connectionDriver);
    QSparqlQuery q(query);

    MySignalObject signalObject;

    QSparqlResult* r = conn.exec(q);
    CHECK_QSPARQL_RESULT(r);

    connect(r, SIGNAL(finished()), &signalObject, SLOT(onFinished()));

    QSignalSpy spy(&signalObject, SIGNAL(finished()));

    while (spy.count() == 0) {
       QTest::qWait(100);
    }

    CHECK_QSPARQL_RESULT(r);

    QVERIFY(r->isFinished());
    QCOMPARE(r->size(), expectedResultsSize);

    delete r;
}

void tst_QSparqlAPI::isFinished_asyncObject_test_data()
{
    QTest::addColumn<QString>("connectionDriver");
    QTest::addColumn<QString>("query");
    QTest::addColumn<int>("expectedResultsSize");

    QTest::newRow("DBus Async Query")
        << "QTRACKER"
        << contactSelectQuery
        << 3;

    QTest::newRow("Tracker Direct Async Query")
        << "QTRACKER_DIRECT"
        << contactSelectQuery
        << 3;

}

QTEST_MAIN( tst_QSparqlAPI )
#include "tst_qsparql_api.moc"
