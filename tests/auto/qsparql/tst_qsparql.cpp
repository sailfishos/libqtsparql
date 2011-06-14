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

#include "private/qsparqlconnection_p.h"
#include "private/qsparqldriver_p.h"

class MockDriver;

class MockResult : public QSparqlResult
{
    Q_OBJECT
    public:
    MockResult(const MockDriver* d);
    int size() const
    {
        return size_;
    }

    QSparqlResultRow current() const
    {
        return QSparqlResultRow();
    }

    QSparqlBinding binding(int) const
    {
        return QSparqlBinding();
    }

    QVariant value(int) const
    {
        return QVariant();
    }
public:
    static int size_;
};

class MockSyncFwOnlyResult : public QSparqlResult
{
    Q_OBJECT
    public:
    MockSyncFwOnlyResult()
        : pos(-1) // first row is row 0, we start BeforeFirstRow
    {
    }
    // Only this is needed for iterating the result
    bool next()
    {
        // Do some work to fetch the next row
        if (++pos < size_) { // determine if the row was the last or not
            updatePos(pos);
            return true;
        }
        updatePos(QSparql::AfterLastRow);
        return false;
    }
    bool hasFeature(QSparqlResult::Feature f) const
    {
        if (f == QSparqlResult::ForwardOnly || f == QSparqlResult::Sync)
            return true;
        return false;
    }

    QSparqlResultRow current() const
    {
        return QSparqlResultRow();
    }

    QSparqlBinding binding(int) const
    {
        return QSparqlBinding();
    }

    QVariant value(int) const
    {
        return QVariant();
    }
    int pos;
    static int size_;
};

class MockDriver : public QSparqlDriver
{
    Q_OBJECT
    public:
    MockDriver()
    {
    }
    ~MockDriver()
    {
    }
    bool open(const QSparqlConnectionOptions&)
    {
        ++openCount;
        setOpen(openRetVal);
        return openRetVal;
    }

    void close()
    {
        ++closeCount;
    }
    bool hasFeature(QSparqlConnection::Feature f) const
    {
        if (f == QSparqlConnection::SyncExec || f == QSparqlConnection::AsyncExec)
            return true;
        return false;
    }
    QSparqlResult* exec(const QString&, QSparqlQuery::StatementType, const QSparqlQueryOptions& options)
    {
        switch(options.executionMethod()) {
        case QSparqlQueryOptions::AsyncExec:
            return new MockResult(this);
        case QSparqlQueryOptions::SyncExec:
            return new MockSyncFwOnlyResult();
        default:
            return 0;
        }
    }

    static int openCount;
    static int closeCount;
    static bool openRetVal;
};

int MockResult::size_ = 0;
int MockSyncFwOnlyResult::size_ = 0;

int MockDriver::openCount = 0;
int MockDriver::closeCount = 0;
bool MockDriver::openRetVal = true;

MockResult::MockResult(const MockDriver*)
    : QSparqlResult()
{
}

class MockDriverCreator : public QSparqlDriverCreatorBase
{
    QSparqlDriver* createObject() const
    {
        return new MockDriver();
    }
};

class tst_QSparql : public QObject
{
    Q_OBJECT

public:
    tst_QSparql();
    virtual ~tst_QSparql();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void mock_creation();
    void wrong_creation();
    void iterate_error_result();
    void open_fails();
    void connection_scope();
    void drivers_list();

    void iterate_empty_result();
    void iterate_nonempty_result();
    void iterate_nonempty_result_backwards();

    void iterate_empty_fwonly_result();
    void iterate_nonempty_fwonly_result();
    void iterate_nonempty_fwonly_result_first();

    void default_QSparqlQueryOptions();
    void copies_of_QSparqlQueryOptions_are_equal_and_independent();
    void assignment_of_QSparqlQueryOptions_creates_equal_and_independent_copy();
};

tst_QSparql::tst_QSparql()
{
}

tst_QSparql::~tst_QSparql()
{
}

void tst_QSparql::initTestCase()
{
    qSparqlRegisterConnectionCreator("MOCK", new MockDriverCreator());

    // For running the test without installing the plugins. Should work in
    // normal and vpath builds.
    QCoreApplication::addLibraryPath("../../../plugins");
}

void tst_QSparql::cleanupTestCase()
{
}

void tst_QSparql::init()
{
    MockDriver::openCount = 0;
    MockDriver::closeCount = 0;
    MockDriver::openRetVal = true;
    MockResult::size_ = 0;
    MockSyncFwOnlyResult::size_ = 0;
}

void tst_QSparql::cleanup()
{
}

void tst_QSparql::mock_creation()
{
    QSparqlConnection conn("MOCK");
    QCOMPARE(MockDriver::openCount, 1);
}

void tst_QSparql::wrong_creation()
{
    QSparqlConnection conn("TOTALLYNOTTHERE");
    QSparqlResult* res = conn.exec(QSparqlQuery("foo"));
    QVERIFY(res->hasError());
    QCOMPARE(res->lastError().type(), QSparqlError::ConnectionError);
    delete res;
}

void tst_QSparql::iterate_error_result()
{
    QSparqlConnection conn("TOTALLYNOTTHERE");
    QSparqlResult* res = conn.exec(QSparqlQuery("foo"));
    QVERIFY(res->hasError());
    QVERIFY(!res->next());
    delete res;
}

void tst_QSparql::open_fails()
{
    MockDriver::openRetVal = false;
    QSparqlConnection conn("MOCK");
    QSparqlResult* res = conn.exec(QSparqlQuery("foo"));
    QVERIFY(res->hasError());
    QCOMPARE(res->lastError().type(), QSparqlError::ConnectionError);
}

void tst_QSparql::connection_scope()
{
    {
        QSparqlConnection conn("MOCK");
    }
    QCOMPARE(MockDriver::openCount, 1);
    QCOMPARE(MockDriver::closeCount, 1);
}

void tst_QSparql::drivers_list()
{
    QStringList expectedDrivers;
    expectedDrivers << "QSPARQL_ENDPOINT" << "QTRACKER" << "QTRACKER_DIRECT" << "QVIRTUOSO" << "MOCK";

    QStringList drivers = QSparqlConnection::drivers();
    foreach (const QString& driver, drivers) {
        QVERIFY(expectedDrivers.contains(driver));
        // qDebug() << driver;
    }
    QVERIFY(drivers.size() >= 1);
    QVERIFY(drivers.contains("MOCK"));
}

void tst_QSparql::iterate_empty_result()
{
    QSparqlConnection conn("MOCK");
    QSparqlResult* res = conn.exec(QSparqlQuery("foo"));
    QVERIFY(!res->hasError());
    QVERIFY(res->pos() == QSparql::BeforeFirstRow);
    QVERIFY(!res->next());
    QVERIFY(res->pos() == QSparql::AfterLastRow);
    QVERIFY(!res->previous());
    QVERIFY(res->pos() == QSparql::BeforeFirstRow);
    delete res;
}

void tst_QSparql::iterate_nonempty_result()
{
    QSparqlConnection conn("MOCK");
    QSparqlResult* res = conn.exec(QSparqlQuery("foo"));
    QVERIFY(!res->hasError());
    MockResult::size_ = 2;
    QVERIFY(res->pos() == QSparql::BeforeFirstRow);
    QVERIFY(res->next());
    QCOMPARE(res->pos(), 0);
    QVERIFY(res->next());
    QCOMPARE(res->pos(), 1);
    QVERIFY(!res->next());
    QVERIFY(res->pos() == QSparql::AfterLastRow);
    QVERIFY(!res->next());
    QVERIFY(res->pos() == QSparql::AfterLastRow);
    res->first();
    QVERIFY(res->pos() == 0);
    res->last();
    QVERIFY(res->pos() == 1);
    delete res;
}

void tst_QSparql::iterate_nonempty_result_backwards()
{
    QSparqlConnection conn("MOCK");
    QSparqlResult* res = conn.exec(QSparqlQuery("foo"));
    QVERIFY(!res->hasError());
    MockResult::size_ = 2;
    // Get the result to the last position
    QVERIFY(res->last());
    QVERIFY(!res->next());
    // And then iterate
    QVERIFY(res->pos() == QSparql::AfterLastRow);
    QVERIFY(res->previous());
    QCOMPARE(res->pos(), 1);
    QVERIFY(res->previous());
    QCOMPARE(res->pos(), 0);
    QVERIFY(!res->previous());
    QVERIFY(res->pos() == QSparql::BeforeFirstRow);
    QVERIFY(!res->previous());
    QVERIFY(res->pos() == QSparql::BeforeFirstRow);
    delete res;
}

void tst_QSparql::iterate_empty_fwonly_result()
{
    QSparqlConnection conn("MOCK");
    QSparqlResult* res = conn.syncExec(QSparqlQuery("foo"));
    QVERIFY(!res->hasError());

    MockSyncFwOnlyResult::size_ = 0;
    QVERIFY(res->pos() == QSparql::BeforeFirstRow);
    QVERIFY(!res->next());
    QVERIFY(res->pos() == QSparql::AfterLastRow);
    // previous() always fails, and it doesn't change the position
    QVERIFY(!res->previous());
    QVERIFY(res->pos() == QSparql::AfterLastRow);
    delete res;
}

void tst_QSparql::iterate_nonempty_fwonly_result()
{
    QSparqlConnection conn("MOCK");
    QSparqlResult* res = conn.syncExec(QSparqlQuery("foo"));
    QVERIFY(!res->hasError());
    MockSyncFwOnlyResult::size_ = 2;
    QVERIFY(res->pos() == QSparql::BeforeFirstRow);
    QVERIFY(res->next());
    QCOMPARE(res->pos(), 0);

    // previous() always fails, and it doesn't change the position
    QVERIFY(!res->previous());
    QCOMPARE(res->pos(), 0);

    QVERIFY(res->next());
    QCOMPARE(res->pos(), 1);

    // previous() always fails, and it doesn't change the position
    QVERIFY(!res->previous());
    QCOMPARE(res->pos(), 1);

    // first and last fail, and they don't chane the position
    QVERIFY(!res->first());
    QVERIFY(res->pos() == 1);
    QVERIFY(!res->last());
    QVERIFY(res->pos() == 1);

    // also setPos fails (even if we try setPos(pos() + 1))
    QVERIFY(!res->setPos(0));
    QVERIFY(res->pos() == 1);
    QVERIFY(!res->setPos(2));
    QVERIFY(res->pos() == 1);

    QVERIFY(!res->next());
    QVERIFY(res->pos() == QSparql::AfterLastRow);
    QVERIFY(!res->next());
    QVERIFY(res->pos() == QSparql::AfterLastRow);

    // previous() always fails, and it doesn't change the position
    QVERIFY(!res->previous());
    QVERIFY(res->pos() == QSparql::AfterLastRow);

    delete res;
}

void tst_QSparql::iterate_nonempty_fwonly_result_first()
{
    // A corner case where the user calls first() legally for a forward only
    // result
    QSparqlConnection conn("MOCK");
    QSparqlResult* res = conn.syncExec(QSparqlQuery("foo"));
    QVERIFY(!res->hasError());
    MockSyncFwOnlyResult::size_ = 2;
    QVERIFY(res->pos() == QSparql::BeforeFirstRow);
    // legal first(): just call next() once
    QVERIFY(res->first());
    QCOMPARE(res->pos(), 0);

    // another legal first(), we're at the first row already
    QVERIFY(res->first());
    QCOMPARE(res->pos(), 0);
}

void tst_QSparql::default_QSparqlQueryOptions()
{
    QSparqlQueryOptions opt;
    QCOMPARE( opt.executionMethod(), QSparqlQueryOptions::AsyncExec );
    QCOMPARE( opt.priority(), QSparqlQueryOptions::NormalPriority );
}

void tst_QSparql::copies_of_QSparqlQueryOptions_are_equal_and_independent()
{
    QSparqlQueryOptions opt1;
    opt1.setExecutionMethod(QSparqlQueryOptions::SyncExec);
    QSparqlQueryOptions opt2(opt1);
    QCOMPARE( opt2.executionMethod(), QSparqlQueryOptions::SyncExec );
    QVERIFY( opt2 == opt1 );

    opt2.setPriority(QSparqlQueryOptions::LowPriority);
    QCOMPARE( opt2.priority(), QSparqlQueryOptions::LowPriority );
    QCOMPARE( opt1.priority(), QSparqlQueryOptions::NormalPriority );
    QVERIFY( !(opt2 == opt1) );

    opt2.setPriority(QSparqlQueryOptions::NormalPriority);
    QVERIFY( opt2 == opt1 );
}

void tst_QSparql::assignment_of_QSparqlQueryOptions_creates_equal_and_independent_copy()
{
    QSparqlQueryOptions opt1;
    opt1.setExecutionMethod(QSparqlQueryOptions::SyncExec);
    QSparqlQueryOptions opt2;
    QVERIFY( !(opt2 == opt1) );
    opt2 = opt1;
    QCOMPARE( opt2.executionMethod(), QSparqlQueryOptions::SyncExec );
    QVERIFY( opt2 == opt1 );

    opt2.setPriority(QSparqlQueryOptions::LowPriority);
    QCOMPARE( opt2.priority(), QSparqlQueryOptions::LowPriority );
    QCOMPARE( opt1.priority(), QSparqlQueryOptions::NormalPriority );
    QVERIFY( !(opt2 == opt1) );

    opt2.setPriority(QSparqlQueryOptions::NormalPriority);
    QVERIFY( opt2 == opt1 );
}

QTEST_MAIN(tst_QSparql)
#include "tst_qsparql.moc"

