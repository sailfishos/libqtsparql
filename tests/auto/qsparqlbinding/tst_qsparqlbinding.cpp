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

#include <QtCore/QtCore>
Q_DECLARE_METATYPE(QVariant)

#include <QtTest/QtTest>
#include <QtSparql/QtSparql>

#include <QUrl>

//const QString qtest(qTableName( "qtest", __FILE__ )); // FIXME: what's this

//TESTED_FILES=

class tst_QSparqlBinding : public QObject
{
    Q_OBJECT

public:
    tst_QSparqlBinding();
    virtual ~tst_QSparqlBinding();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void toString_data();
    void toString();
    void types_data();
    void types();
};

tst_QSparqlBinding::tst_QSparqlBinding()
{
}

tst_QSparqlBinding::~tst_QSparqlBinding()
{
}

void tst_QSparqlBinding::initTestCase()
{
}

void tst_QSparqlBinding::cleanupTestCase()
{
}

void tst_QSparqlBinding::init()
{
}

void tst_QSparqlBinding::cleanup()
{
}

void tst_QSparqlBinding::toString_data()
{
    QTest::addColumn<QVariant>("value");
    QTest::addColumn<QVariant>("lang");
    QTest::addColumn<QVariant>("datatype");
    QTest::addColumn<QString>("toString");
    QTest::addColumn<QString>("as_string");

    QTest::newRow("int") <<
        QVariant(static_cast<int>(54)) <<
        QVariant() <<
        QVariant() <<
        QString("54") <<
        QString("54");

    QTest::newRow("int_with_datatype") <<
        QVariant(static_cast<int>(54)) <<
        QVariant() <<
        QVariant(QUrl("http://www.w3.org/2001/XMLSchema#int")) <<
        QString("\"54\"^^<http://www.w3.org/2001/XMLSchema#int>") <<
        QString("54");

    QTest::newRow("unsigned_int") <<
        QVariant(static_cast<unsigned int>(54)) <<
        QVariant() <<
        QVariant() <<
        QString("54") <<
        QString("54");

    QTest::newRow("unsigned_int_with_datatype") <<
        QVariant(static_cast<unsigned int>(54)) <<
        QVariant() <<
        QVariant(QUrl("http://www.w3.org/2001/XMLSchema#unsignedInt")) <<
        QString("\"54\"^^<http://www.w3.org/2001/XMLSchema#unsignedInt>") <<
        QString("54");

    // Note that a QVariant can't be constructed with a long
    QTest::newRow("long") <<
        QVariant(static_cast<int>(54)) <<
        QVariant() <<
        QVariant() <<
        QString("54") <<
        QString("54");

    QTest::newRow("long_with_datatype") <<
        QVariant(static_cast<int>(54)) <<
        QVariant() <<
        QVariant(QUrl("http://www.w3.org/2001/XMLSchema#long")) <<
        QString("\"54\"^^<http://www.w3.org/2001/XMLSchema#long>") <<
        QString("54");

    QTest::newRow("unsigned_long") <<
        QVariant(static_cast<int>(54)) <<
        QVariant() <<
        QVariant() <<
        QString("54") <<
        QString("54");

    QTest::newRow("unsigned_long_with_datatype") <<
        QVariant(static_cast<int>(54)) <<
        QVariant() <<
        QVariant(QUrl("http://www.w3.org/2001/XMLSchema#unsignedLong")) <<
        QString("\"54\"^^<http://www.w3.org/2001/XMLSchema#unsignedLong>") <<
        QString("54");

    QTest::newRow("double") <<
        QVariant(static_cast<double>(54.0)) <<
        QVariant() <<
        QVariant() <<
        QString("5.4000000000e+01") <<
        QString("5.4000000000e+01");

    QTest::newRow("double_with_datatype") <<
        QVariant(static_cast<double>(54.0)) <<
        QVariant() <<
        QVariant(QUrl("http://www.w3.org/2001/XMLSchema#double")) <<
        QString("\"5.4000000000e+01\"^^<http://www.w3.org/2001/XMLSchema#double>") <<
        QString("5.4000000000e+01");

    QTest::newRow("boolean") <<
        QVariant(true) <<
        QVariant() <<
        QVariant() <<
        QString("true") <<
        QString("true");

    QTest::newRow("boolean_with_datatype") <<
        QVariant(true) <<
        QVariant() <<
        QVariant(QUrl("http://www.w3.org/2001/XMLSchema#boolean")) <<
        QString("\"true\"^^<http://www.w3.org/2001/XMLSchema#boolean>") <<
        QString("true");

    QTest::newRow("date") <<
        QVariant(QDate(2000, 1, 30)) <<
        QVariant() <<
        QVariant() <<
        QString("2000-01-30") <<
        QString("2000-01-30");

    QTest::newRow("date_with_datatype") <<
        QVariant(QDate(2000, 1, 30)) <<
        QVariant() <<
        QVariant(QUrl("http://www.w3.org/2001/XMLSchema#date")) <<
        QString("\"2000-01-30\"^^<http://www.w3.org/2001/XMLSchema#date>") <<
        QString("2000-01-30");

    QTest::newRow("time") <<
        QVariant(QTime(12, 5, 59)) <<
        QVariant() <<
        QVariant() <<
        QString("12:05:59") <<
        QString("12:05:59");

    QTest::newRow("time_with_datatype") <<
        QVariant(QTime(12, 5, 59)) <<
        QVariant() <<
        QVariant(QUrl("http://www.w3.org/2001/XMLSchema#time")) <<
        QString("\"12:05:59\"^^<http://www.w3.org/2001/XMLSchema#time>") <<
        QString("12:05:59");

    QTest::newRow("datetime") <<
        QVariant(QDateTime(QDate(2000, 1, 30), QTime(12, 5, 59))) <<
        QVariant() <<
        QVariant() <<
        QString("2000-01-30T12:05:59") <<
        QString("2000-01-30T12:05:59");

    QTest::newRow("datetime_with_datatype") <<
        QVariant(QDateTime(QDate(2000, 1, 30), QTime(12, 5, 59))) <<
        QVariant() <<
        QVariant(QUrl("http://www.w3.org/2001/XMLSchema#dateTime")) <<
        QString("\"2000-01-30T12:05:59\"^^<http://www.w3.org/2001/XMLSchema#dateTime>") <<
        QString("2000-01-30T12:05:59");

    QTest::newRow("string") <<
        QVariant("foo") <<
        QVariant() <<
        QVariant() <<
        QString("\"foo\"") <<
        QString("foo");

    QTest::newRow("string_escaped") <<
        QVariant("foo\tbar\nbazwith \"quotes\' and \\ is escaped") <<
        QVariant() <<
        QVariant() <<
        QString("\"foo\\tbar\\nbazwith \\\"quotes\\' and \\\\ is escaped\"") <<
        QString("\"foo\\tbar\\nbazwith \\\"quotes\\' and \\\\ is escaped\"");

    QTest::newRow("string_with_datatype") <<
        QVariant("cat") <<
        QVariant() <<
        QVariant("http://www.w3.org/2001/XMLSchema#string") <<
        QString("\"cat\"^^<http://www.w3.org/2001/XMLSchema#string>") <<
        QString("cat");

    QTest::newRow("url") <<
        QVariant(QUrl("urn:uri:123")) <<
        QVariant() <<
        QVariant() <<
        QString("<urn:uri:123>") <<
        QString("urn:uri:123");
}

void tst_QSparqlBinding::toString()
{
    QFETCH(QVariant, value);
    QFETCH(QVariant, lang);
    QFETCH(QVariant, datatype);
    QFETCH(QString, toString);
    QFETCH(QString, as_string);

    QSparqlBinding b1("testBinding1", value);
    if (!lang.isNull())
        b1.setLanguageTag(lang.toString());
    if (!datatype.isNull())
        b1.setDataTypeUri(datatype.toUrl());

    QCOMPARE(b1.toString(), toString);
    QCOMPARE(b1.value(), value);

    if (!datatype.isNull()) {
        QSparqlBinding b2("testBinding2");
        b2.setValue(as_string, datatype.toUrl());
        QCOMPARE(b2.toString(), toString);
    }
}

void tst_QSparqlBinding::types_data()
{
    QTest::addColumn<QVariant>("value");
    QTest::addColumn<bool>("isUri");
    QTest::addColumn<bool>("isLiteral");
    QTest::addColumn<bool>("isBlank");

    QTest::newRow("int") <<
        QVariant(-67) <<
        false << true << false;

    QTest::newRow("url") <<
        QVariant(QUrl("urn:uri:123")) <<
        true << false << false;
}

void tst_QSparqlBinding::types()
{
    QFETCH(QVariant, value);
    QFETCH(bool, isUri);
    QFETCH(bool, isLiteral);
    QFETCH(bool, isBlank);

    QSparqlBinding b("testBinding", value);

    QCOMPARE(b.isUri(), isUri);
    QCOMPARE(b.isLiteral(), isLiteral);
    QCOMPARE(b.isBlank(), isBlank);
}


QTEST_MAIN( tst_QSparqlBinding )
#include "tst_qsparqlbinding.moc"
