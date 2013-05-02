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

#include <QtCore/QtCore>
Q_DECLARE_METATYPE(QVariant)

#include <QtTest/QtTest>
#include <QtSparql>
#include <qsparqlresultrow.h>
Q_DECLARE_METATYPE(QSparqlResultRow)

#include <QUrl>

class tst_QSparqlResultRow : public QObject
{
    Q_OBJECT

public:
    tst_QSparqlResultRow();
    virtual ~tst_QSparqlResultRow();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void equality_operator_data();
    void equality_operator();
    void assignment_operator();
    void clear();
    void clearValues();
    void indexOf();
    void contains();
    void variableName();
    void binding();
    void value();

private:
};

tst_QSparqlResultRow::tst_QSparqlResultRow()
{
}

tst_QSparqlResultRow::~tst_QSparqlResultRow()
{
}

void tst_QSparqlResultRow::initTestCase()
{
}

void tst_QSparqlResultRow::cleanupTestCase()
{
}

void tst_QSparqlResultRow::init()
{
}

void tst_QSparqlResultRow::cleanup()
{
}

void tst_QSparqlResultRow::equality_operator_data()
{
    QTest::addColumn<QSparqlResultRow>("value1");
    QTest::addColumn<QSparqlResultRow>("value2");
    QTest::addColumn<bool>("isEqual");

    QSparqlResultRow r1;
    QSparqlResultRow r2;
    QSparqlResultRow r3;
    QSparqlBinding b1("testBinding1", QVariant(-67));
    r3.append(b1);
    QSparqlResultRow r4;
    QSparqlBinding b2("testBinding1", QString("string_literal"));
    r4.append(b2);
    QSparqlResultRow r5;
    r5.append(b1);
    r5.append(b2);
    QSparqlResultRow r6;
    r6.append(b1);
    r6.append(b2);
    QSparqlResultRow r7;
    QSparqlBinding b3("testBinding2", QString("string_literal"));
    r6.append(b1);
    r6.append(b3);

    QTest::newRow("empty1") <<
        r1 << r1 << true;

    QTest::newRow("empty2") <<
        r1 << r2 << true;

    QTest::newRow("one_value1") <<
        r3 << r3 << true;

    QTest::newRow("one_value2") <<
        r3 << r4 << false;

    QTest::newRow("multiple_values1") <<
        r5 << r5 << true;

    QTest::newRow("multiple_values2") <<
        r5 << r6 << false;
}

void tst_QSparqlResultRow::equality_operator()
{
    QFETCH(QSparqlResultRow, value1);
    QFETCH(QSparqlResultRow, value2);
    QFETCH(bool, isEqual);

    QCOMPARE(value1 == value2, isEqual);
    QCOMPARE(value1 != value2, !isEqual);
}

void tst_QSparqlResultRow::assignment_operator()
{
    QSparqlResultRow r1;
    QSparqlBinding b1("testBinding1", QVariant(-67));
    r1.append(b1);
    QSparqlResultRow r2 = r1;
    QSparqlResultRow r3(r1);

    QCOMPARE(r1 == r2, true);
    QCOMPARE(r1 == r3, true);
}

void tst_QSparqlResultRow::clear()
{
    QSparqlResultRow r1;
    QSparqlBinding b1("testBinding1", QVariant(-67));
    r1.append(b1);
    QCOMPARE(r1.count(), 1);
    QCOMPARE(r1.isEmpty(), false);
    QSparqlResultRow r2;
    r2.append(b1);
    QCOMPARE(r1 == r2, true);
    r1.clear();
    QCOMPARE(r1.isEmpty(), true);
    QCOMPARE(r1.count(), 0);
    QCOMPARE(r1 == r2, false);
}

void tst_QSparqlResultRow::clearValues()
{
    QSparqlResultRow r1;
    QSparqlBinding b1("testBinding1", QVariant(-67));
    r1.append(b1);
    QCOMPARE(r1.count(), 1);
    QCOMPARE(r1.isEmpty(), false);
    QSparqlResultRow r2;
    r2.append(b1);
    QCOMPARE(r1 == r2, true);
    r1.clearValues();
    QCOMPARE(r1.isEmpty(), false);
    QCOMPARE(r1.count(), 1);
    QCOMPARE(r1 == r2, false);
    QCOMPARE(r1.binding("testBinding1") == b1, false);
}

void tst_QSparqlResultRow::indexOf()
{
    QSparqlResultRow r1;
    QSparqlBinding b1("testBinding1", QVariant(-67));
    QSparqlBinding b2("testBinding2", QString("string_literal"));
    r1.append(b1);
    r1.append(b2);
    QCOMPARE(r1.indexOf("foo"), -1);
    QCOMPARE(r1.indexOf(""), -1);
    QCOMPARE(r1.indexOf("testBinding1"), 0);
    QCOMPARE(r1.indexOf("testBinding2"), 1);
}

void tst_QSparqlResultRow::contains()
{
    QSparqlResultRow r1;
    QSparqlBinding b1("testBinding1", QVariant(-67));
    QSparqlBinding b2("testBinding2", QString("string_literal"));
    r1.append(b1);
    r1.append(b2);
    QCOMPARE(r1.contains("foo"), false);
    QCOMPARE(r1.contains(""), false);
    QCOMPARE(r1.contains("testBinding1"), true);
    QCOMPARE(r1.contains("testBinding2"), true);
}

void tst_QSparqlResultRow::variableName()
{
    QSparqlResultRow r1;
    QSparqlBinding b1("testBinding1", QVariant(-67));
    QSparqlBinding b2("testBinding2", QString("string_literal"));
    r1.append(b1);
    r1.append(b2);
    QCOMPARE(r1.variableName(-1), QString());
    QCOMPARE(r1.variableName(0), QLatin1String("testBinding1"));
    QCOMPARE(r1.variableName(1), QLatin1String("testBinding2"));
    QCOMPARE(r1.variableName(2), QString());
}

void tst_QSparqlResultRow::binding()
{
    QSparqlResultRow r1;
    QSparqlBinding b1("testBinding1", QVariant(-67));
    QSparqlBinding b2("testBinding2", QString("string_literal"));
    r1.append(b1);
    r1.append(b2);
    QCOMPARE(r1.binding(-1), QSparqlBinding());
    QCOMPARE(r1.binding(0), b1);
    QCOMPARE(r1.binding(1), b2);
    QCOMPARE(r1.binding(2), QSparqlBinding());
    QCOMPARE(r1.binding(""), QSparqlBinding());
    QCOMPARE(r1.binding("foo"), QSparqlBinding());
    QCOMPARE(r1.binding("testBinding1"), b1);
    QCOMPARE(r1.binding("testBinding2"), b2);
}

void tst_QSparqlResultRow::value()
{
    QSparqlResultRow r1;
    QVariant v1(-67);
    QVariant v2(QLatin1String("string_literal"));
    QSparqlBinding b1("testBinding1", v1);
    QSparqlBinding b2("testBinding2", v2);
    r1.append(b1);
    r1.append(b2);
    QCOMPARE(r1.value(-1), QVariant());
    QCOMPARE(r1.value(0), v1);
    QCOMPARE(r1.value(1), v2);
    QCOMPARE(r1.value(2), QVariant());
    QCOMPARE(r1.value(""), QVariant());
    QCOMPARE(r1.value("foo"), QVariant());
    QCOMPARE(r1.value("testBinding1"), v1);
    QCOMPARE(r1.value("testBinding2"), v2);
}

QTEST_MAIN( tst_QSparqlResultRow )
#include "tst_qsparqlresultrow.moc"
