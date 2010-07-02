/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the test suite of the Qt Toolkit.
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
** Nokia at qt-info@nokia.com.
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

#include "private/qsparqlconnection_p.h"
//const QString qtest(qTableName( "qtest", __FILE__ )); // FIXME: what's this

//TESTED_FILES=

class MockDriver;

class MockResult : public QSparqlResult
{
    Q_OBJECT
    public:
    MockResult(const MockDriver* d);
    int size()
    {
        return 0;
    }
    bool fetch(int)
    {
        return false;
    }
    bool fetchFirst()
    {
        return false;
    }
    bool fetchLast()
    {
        return false;
    }
    QVariant data(int) const
    {
        return QVariant();
    }
    bool isNull(int) const
    {
        return true;
    }
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
    bool hasFeature(QSparqlDriver::DriverFeature) const
    {
        return false;
    }
    MockResult* createResult() const
    {
        return new MockResult(this);
    }
    static int openCount;
    static int closeCount;
    static bool openRetVal;
};

int MockDriver::openCount = 0;
int MockDriver::closeCount = 0;
bool MockDriver::openRetVal = true;

MockResult::MockResult(const MockDriver* d)
    : QSparqlResult(d)
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
    void open_fails();
    void connection_scope();
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
}

void tst_QSparql::cleanupTestCase()
{
}

void tst_QSparql::init()
{
    MockDriver::openCount = 0;
    MockDriver::closeCount = 0;
    MockDriver::openRetVal = true;
}

void tst_QSparql::cleanup()
{
}

void tst_QSparql::mock_creation()
{
    QSparqlConnection conn("MOCK");
    QCOMPARE(MockDriver::openCount, 1);
    QVERIFY(conn.isOpen());
}

void tst_QSparql::wrong_creation()
{
    QSparqlConnection conn("TOTALLYNOTTHERE");
    QVERIFY(!conn.isOpen());
    QCOMPARE((void*)conn.exec(QSparqlQuery("foo")), (void*)0);
}

void tst_QSparql::open_fails()
{
    MockDriver::openRetVal = false;
    QSparqlConnection conn("MOCK");
    QVERIFY(!conn.isOpen());
    QCOMPARE((void*)conn.exec(QSparqlQuery("foo")), (void*)0);
}

void tst_QSparql::connection_scope()
{
    {
        QSparqlConnection conn("MOCK");
    }
    QCOMPARE(MockDriver::openCount, 1);
    QCOMPARE(MockDriver::closeCount, 1);
}


QTEST_MAIN(tst_QSparql)
#include "tst_qsparql.moc"

