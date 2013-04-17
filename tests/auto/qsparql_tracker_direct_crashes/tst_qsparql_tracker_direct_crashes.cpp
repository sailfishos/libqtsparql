/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
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

class tst_QSparqlTrackerDirectCrashes : public QObject
{
    Q_OBJECT

public:
    tst_QSparqlTrackerDirectCrashes();
    virtual ~tst_QSparqlTrackerDirectCrashes();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void waitForFinished_crashes_when_connection_opening_fails();
    void syncExec_crashes_when_connection_opening_fails();
    void syncExec_update_crashes_when_connection_opening_fails();
    void syncExec_failed_connection();
    void asyncExec_failed_connection();

    void two_failing_connections();
};

tst_QSparqlTrackerDirectCrashes::tst_QSparqlTrackerDirectCrashes()
{
}

tst_QSparqlTrackerDirectCrashes::~tst_QSparqlTrackerDirectCrashes()
{
}

void tst_QSparqlTrackerDirectCrashes::initTestCase()
{
    // For running the test without installing the plugins. Should work in
    // normal and vpath builds.
    QCoreApplication::addLibraryPath("../../../plugins");
}

void tst_QSparqlTrackerDirectCrashes::cleanupTestCase()
{
}

void tst_QSparqlTrackerDirectCrashes::init()
{
}

void tst_QSparqlTrackerDirectCrashes::cleanup()
{
}

void tst_QSparqlTrackerDirectCrashes::waitForFinished_crashes_when_connection_opening_fails()
{
    // This is a regression test. Run this in a setup where the
    // libtracker-sparql connection opening fails to reproduce the problem.
    QSparqlConnection conn ("QTRACKER_DIRECT");
    QSparqlQuery query("SELECT ?u {?u a rdfs:Resource .}");

    QSparqlResult* r = conn.exec(query);
    r->waitForFinished();
    QVERIFY2(r->hasError(), "This test needs to run in a setup where connection opening fails");
    QVERIFY(r->lastError().type() == QSparqlError::ConnectionError);
    delete r;
}

void tst_QSparqlTrackerDirectCrashes::syncExec_crashes_when_connection_opening_fails()
{
    // This is a regression test. Run this in a setup where the
    // libtracker-sparql connection opening fails to reproduce the problem.
    QSparqlConnection conn ("QTRACKER_DIRECT");
    QSparqlQuery query("SELECT ?u {?u a rdfs:Resource .}");

    QSparqlResult* r = conn.syncExec(query);
    QVERIFY2(r->hasError(), "This test needs to run in a setup where connection opening fails");
    QVERIFY(r->lastError().type() == QSparqlError::ConnectionError);
    delete r;
}

void tst_QSparqlTrackerDirectCrashes::syncExec_update_crashes_when_connection_opening_fails()
{
    // This is a regression test. Run this in a setup where the
    // libtracker-sparql connection opening fails to reproduce the problem.
    QSparqlConnection conn ("QTRACKER_DIRECT");
    QSparqlQuery insertQuery("insert { "
                             "<testcontact> a nco:PersonContact ; "
                             "nie:isLogicalPartOf <qsparql-tracker-direct-tests> . }",
                             QSparqlQuery::InsertStatement);

    QSparqlResult* r = conn.syncExec(insertQuery);
    delete r;

    QSparqlQuery deleteQuery("delete { "
                             "<testcontact> a rdfs:Resource . }",
                             QSparqlQuery::DeleteStatement);

    r = conn.syncExec(deleteQuery);
    QVERIFY2(r->hasError(), "This test needs to run in a setup where connection opening fails");
    QVERIFY(r->lastError().type() == QSparqlError::ConnectionError);
    delete r;
}

void tst_QSparqlTrackerDirectCrashes::syncExec_failed_connection()
{
    // Run this in a setup where the libtracker-sparql connection opening fails
    QSparqlConnection conn ("QTRACKER_DIRECT");
    QCOMPARE(conn.isValid(), true);
    //Connection opening in direct driver is asynchronous so we have to wait until it's finished
    QTest::qWait(1000);
    QCOMPARE(conn.hasError(), true);
    QCOMPARE(conn.lastError().type(), QSparqlError::ConnectionError);

    QSparqlQuery insertQuery("insert { "
                             "<testcontact> a nco:PersonContact ; "
                             "nie:isLogicalPartOf <qsparql-tracker-direct-tests> . }",
                             QSparqlQuery::InsertStatement);

    QSparqlResult* r = conn.syncExec(insertQuery);
    QCOMPARE(r->hasError(), true);
    QCOMPARE(r->lastError().type(), QSparqlError::ConnectionError);
    delete r;
}

void tst_QSparqlTrackerDirectCrashes::asyncExec_failed_connection()
{
    // Run this in a setup where the libtracker-sparql connection opening fails
    QSparqlConnection conn ("QTRACKER_DIRECT");
    QCOMPARE(conn.isValid(), true);
    //Connection opening in direct driver is asynchronous so we have to wait until it's finished
    QTest::qWait(1000);
    QCOMPARE(conn.hasError(), true);
    QCOMPARE(conn.lastError().type(), QSparqlError::ConnectionError);

    QSparqlQuery insertQuery("insert { "
                             "<testcontact> a nco:PersonContact ; "
                             "nie:isLogicalPartOf <qsparql-tracker-direct-tests> . }",
                             QSparqlQuery::InsertStatement);

    QSparqlResult* r = conn.exec(insertQuery);
    r->waitForFinished();
    QCOMPARE(r->hasError(), true);
    QCOMPARE(r->lastError().type(), QSparqlError::ConnectionError);
    delete r;
}

void tst_QSparqlTrackerDirectCrashes::two_failing_connections()
{
    // This is a regression test. Run this in a setup where the
    // libtracker-sparql connection opening fails to reproduce the problem.
    {
        QSparqlConnection conn ("QTRACKER_DIRECT");
        QSparqlQuery query("SELECT ?u {?u a rdfs:Resource .}");

        QSparqlResult* r = conn.exec(query);
        r->waitForFinished();
        QVERIFY2(r->hasError(), "This test needs to run in a setup where connection opening fails");
        QVERIFY(r->lastError().type() == QSparqlError::ConnectionError);
        delete r;
    }
    {
        QSparqlConnection conn ("QTRACKER_DIRECT");
        QSparqlQuery query("SELECT ?u {?u a rdfs:Resource .}");
        QSparqlResult* r = conn.syncExec(query);
        QVERIFY2(r->hasError(), "This test needs to run in a setup where connection opening fails");
        QVERIFY(r->lastError().type() == QSparqlError::ConnectionError);
        delete r;
    }
}

QTEST_MAIN( tst_QSparqlTrackerDirectCrashes )
#include "tst_qsparql_tracker_direct_crashes.moc"
