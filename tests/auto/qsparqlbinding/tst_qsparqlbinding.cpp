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
Q_DECLARE_METATYPE(QSparqlBinding)

#include <QUrl>

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
    void equality_operator_data();
    void equality_operator();
    void assignment_operator();
    void clear();

private:
    void add_toString_data_rows(const char* dataTag,
                                const QVariant& value,
                                const QString& as_string,
                                const QUrl& datatype);
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

void tst_QSparqlBinding::add_toString_data_rows(const char* dataTag,
                                                const QVariant& value,
                                                const QString& as_string,
                                                const QUrl& datatype)
{
    QTest::newRow(dataTag) <<
        value <<
        QVariant() <<
        QVariant() <<
        as_string <<
        as_string <<
        datatype;

    QTest::newRow((QByteArray(dataTag) + "_with_datatype").data()) <<
        value <<
        QVariant() <<
        QVariant(datatype) <<
        (QString("\"") + as_string + "\"^^<" + datatype.toString() + ">") <<
        as_string <<
        datatype;
}

void tst_QSparqlBinding::toString_data()
{
    QTest::addColumn<QVariant>("value");
    QTest::addColumn<QVariant>("lang");
    QTest::addColumn<QVariant>("datatype");
    QTest::addColumn<QString>("toString");
    QTest::addColumn<QString>("as_string");
    QTest::addColumn<QUrl>("get_datatype");

    const QUrl xsd_int("http://www.w3.org/2001/XMLSchema#int");
    add_toString_data_rows("int_null",    QVariant(int(            0)),           "0", xsd_int);
    add_toString_data_rows("int_typical", QVariant(int(           54)),          "54", xsd_int);
    add_toString_data_rows("int_max",     QVariant(int(   2147483647)),  "2147483647", xsd_int);
    add_toString_data_rows("int_min",     QVariant(int(-2147483647-1)), "-2147483648", xsd_int);

    const QUrl xsd_uint("http://www.w3.org/2001/XMLSchema#unsignedInt");
    add_toString_data_rows("uint_null",    QVariant(uint(         0u)),           "0", xsd_uint);
    add_toString_data_rows("uint_typical", QVariant(uint(        54u)),          "54", xsd_uint);
    add_toString_data_rows("uint_max",     QVariant(uint(4294967295u)),  "4294967295", xsd_uint);

    const QUrl xsd_long("http://www.w3.org/2001/XMLSchema#long");
    add_toString_data_rows("long_long_null",    QVariant(Q_INT64_C(                      0)),                    "0", xsd_long);
    add_toString_data_rows("long_long_typical", QVariant(Q_INT64_C(                     54)),                   "54", xsd_long);
    add_toString_data_rows("long_long_max",     QVariant(Q_INT64_C(    9223372036854775807)),  "9223372036854775807", xsd_long);
    add_toString_data_rows("long_long_min",     QVariant(Q_INT64_C(-9223372036854775807)-1), "-9223372036854775808", xsd_long);

    const QUrl xsd_ulong("http://www.w3.org/2001/XMLSchema#unsignedLong");
    add_toString_data_rows("ulong_long_null",    QVariant(Q_UINT64_C(                   0)),                     "0", xsd_ulong);
    add_toString_data_rows("ulong_long_typical", QVariant(Q_UINT64_C(                  54)),                    "54", xsd_ulong);
    add_toString_data_rows("ulong_long_max",     QVariant(Q_UINT64_C(18446744073709551615)),  "18446744073709551615", xsd_ulong);

    const QUrl xsd_double("http://www.w3.org/2001/XMLSchema#double");
    add_toString_data_rows("double_null",        QVariant(double(    0)),  "0.0000000000e+00", xsd_double);
    add_toString_data_rows("double_typical_pos", QVariant(double( 54.0)),  "5.4000000000e+01", xsd_double);
    add_toString_data_rows("double_typical_neg", QVariant(double(-54.0)), "-5.4000000000e+01", xsd_double);
    // double min and max are not fixed and thus hard to test

    QTest::newRow("boolean(true)") <<
        QVariant(true) <<
        QVariant() <<
        QVariant() <<
        QString("true") <<
        QString("true") <<
        QUrl("http://www.w3.org/2001/XMLSchema#boolean");

    QTest::newRow("boolean(false") <<
        QVariant(false) <<
        QVariant() <<
        QVariant() <<
        QString("false") <<
        QString("false") <<
        QUrl("http://www.w3.org/2001/XMLSchema#boolean");

    QTest::newRow("boolean_with_datatype(true)") <<
        QVariant(true) <<
        QVariant() <<
        QVariant(QUrl("http://www.w3.org/2001/XMLSchema#boolean")) <<
        QString("\"true\"^^<http://www.w3.org/2001/XMLSchema#boolean>") <<
        QString("true") <<
        QUrl("http://www.w3.org/2001/XMLSchema#boolean");

    QTest::newRow("boolean_with_datatype(false)") <<
        QVariant(false) <<
        QVariant() <<
        QVariant(QUrl("http://www.w3.org/2001/XMLSchema#boolean")) <<
        QString("\"false\"^^<http://www.w3.org/2001/XMLSchema#boolean>") <<
        QString("false") <<
        QUrl("http://www.w3.org/2001/XMLSchema#boolean");

    QTest::newRow("date") <<
        QVariant(QDate(2000, 1, 30)) <<
        QVariant() <<
        QVariant() <<
        QString("\"2000-01-30\"") <<
        QString("2000-01-30") <<
        QUrl("http://www.w3.org/2001/XMLSchema#date");

    QTest::newRow("date_with_datatype") <<
        QVariant(QDate(2000, 1, 30)) <<
        QVariant() <<
        QVariant(QUrl("http://www.w3.org/2001/XMLSchema#date")) <<
        QString("\"2000-01-30\"^^<http://www.w3.org/2001/XMLSchema#date>") <<
        QString("2000-01-30") <<
        QUrl("http://www.w3.org/2001/XMLSchema#date");

    QTest::newRow("time") <<
        QVariant(QTime(12, 5, 59)) <<
        QVariant() <<
        QVariant() <<
        QString("\"12:05:59\"") <<
        QString("12:05:59") <<
        QUrl("http://www.w3.org/2001/XMLSchema#time");

    QTest::newRow("time_with_datatype") <<
        QVariant(QTime(12, 5, 59)) <<
        QVariant() <<
        QVariant(QUrl("http://www.w3.org/2001/XMLSchema#time")) <<
        QString("\"12:05:59\"^^<http://www.w3.org/2001/XMLSchema#time>") <<
        QString("12:05:59") <<
        QUrl("http://www.w3.org/2001/XMLSchema#time");

    QTest::newRow("datetime") <<
        QVariant(QDateTime(QDate(2000, 1, 30), QTime(12, 5, 59))) <<
        QVariant() <<
        QVariant() <<
        QString("\"2000-01-30T12:05:59+00:00\"") <<
        QString("2000-01-30T12:05:59") <<
        QUrl("http://www.w3.org/2001/XMLSchema#dateTime");

    QTest::newRow("datetime_with_datatype") <<
        QVariant(QDateTime(QDate(2000, 1, 30), QTime(12, 5, 59))) <<
        QVariant() <<
        QVariant(QUrl("http://www.w3.org/2001/XMLSchema#dateTime")) <<
        QString("\"2000-01-30T12:05:59+00:00\"^^<http://www.w3.org/2001/XMLSchema#dateTime>") <<
        QString("2000-01-30T12:05:59") <<
        QUrl("http://www.w3.org/2001/XMLSchema#dateTime");

    QTest::newRow("empty_string") <<
        QVariant("") <<
        QVariant() <<
        QVariant() <<
        QString("\"\"") <<
        QString("") <<
        QUrl("http://www.w3.org/2001/XMLSchema#string");

    QTest::newRow("empty_string_with_datatype") <<
        QVariant("") <<
        QVariant() <<
        QVariant(QUrl("http://www.w3.org/2001/XMLSchema#string")) <<
        QString("\"\"^^<http://www.w3.org/2001/XMLSchema#string>") <<
        QString("") <<
        QUrl("http://www.w3.org/2001/XMLSchema#string");

    QTest::newRow("string") <<
        QVariant("foo") <<
        QVariant() <<
        QVariant() <<
        QString("\"foo\"") <<
        QString("foo") <<
        QUrl("http://www.w3.org/2001/XMLSchema#string");

    QTest::newRow("string_escaped") <<
        QVariant("foo\tbar\nbazwith \"quotes\' and \\ is escaped") <<
        QVariant() <<
        QVariant() <<
        QString("\"foo\\tbar\\nbazwith \\\"quotes\\' and \\\\ is escaped\"") <<
        QString("\"foo\\tbar\\nbazwith \\\"quotes\\' and \\\\ is escaped\"") <<
        QUrl("http://www.w3.org/2001/XMLSchema#string");

    QTest::newRow("string_with_datatype") <<
        QVariant("cat") <<
        QVariant() <<
        QVariant(QUrl("http://www.w3.org/2001/XMLSchema#string")) <<
        QString("\"cat\"^^<http://www.w3.org/2001/XMLSchema#string>") <<
        QString("cat") <<
        QUrl("http://www.w3.org/2001/XMLSchema#string");

    QTest::newRow("url") <<
        QVariant(QUrl("urn:uri:123")) <<
        QVariant() <<
        QVariant() <<
        QString("<urn:uri:123>") <<
        QString("urn:uri:123") <<
        QUrl();
}

void tst_QSparqlBinding::toString()
{
    QFETCH(QVariant, value);
    QFETCH(QVariant, lang);
    QFETCH(QVariant, datatype);
    QFETCH(QString, toString);
    QFETCH(QString, as_string);
    QFETCH(QUrl, get_datatype);

    QSparqlBinding b1("testBinding1", value);
    if (!lang.isNull())
        b1.setLanguageTag(lang.toString());
    if (!datatype.isNull())
        b1.setDataTypeUri(datatype.toUrl());

    QCOMPARE(b1.toString(), toString);
    QCOMPARE(b1.value(), value);
    QCOMPARE(b1.dataTypeUri(), get_datatype);

    if (!datatype.isNull()) {
        QSparqlBinding b2("testBinding2");
        b2.setValue(as_string, datatype.toUrl());
        QCOMPARE(b2.toString(), toString);
        QCOMPARE(b2.value(), value);
    }
}

void tst_QSparqlBinding::types_data()
{
    QTest::addColumn<QSparqlBinding>("value");
    QTest::addColumn<bool>("isUri");
    QTest::addColumn<bool>("isLiteral");
    QTest::addColumn<bool>("isBlank");

    QSparqlBinding b1("testBinding1", QVariant(-67));
    QSparqlBinding b2("testBinding2", QUrl("urn:uri:123"));
    QSparqlBinding b3("testBinding3");
    b3.setBlankNodeLabel("b3");

    QTest::newRow("int") <<
        b1 <<
        false << true << false;

    QTest::newRow("url") <<
        b2 <<
        true << false << false;

    QTest::newRow("blank") <<
        b3 <<
        false << false << true;
}

void tst_QSparqlBinding::types()
{
    QFETCH(QSparqlBinding, value);
    QFETCH(bool, isUri);
    QFETCH(bool, isLiteral);
    QFETCH(bool, isBlank);

    QCOMPARE(value.isUri(), isUri);
    QCOMPARE(value.isLiteral(), isLiteral);
    QCOMPARE(value.isBlank(), isBlank);
}

void tst_QSparqlBinding::equality_operator_data()
{
    QTest::addColumn<QSparqlBinding>("value1");
    QTest::addColumn<QSparqlBinding>("value2");
    QTest::addColumn<bool>("isEqual");

    QSparqlBinding b1("testBinding1", QVariant(-67));
    QSparqlBinding b2("testBinding2", QVariant(-67));
    QSparqlBinding b3("testBinding3", QVariant(-88));
    QSparqlBinding b4("testBinding4", QUrl("urn:uri:123"));
    QSparqlBinding b5("testBinding5", QUrl("urn:uri:123"));
    QSparqlBinding b6("testBinding6", QUrl("urn:uri:456"));
    QSparqlBinding b7("testBinding7");
    b7.setBlankNodeLabel("b3");
    QSparqlBinding b8("testBinding8");
    b8.setBlankNodeLabel("b3");
    QSparqlBinding b9("testBinding9");
    b9.setBlankNodeLabel("b6");

    QTest::newRow("int1") <<
        b1 << b1 << true;

    QTest::newRow("int2") <<
        b1 << b2 << true;

    QTest::newRow("int3") <<
        b1 << b3 << false;

    QTest::newRow("int4") <<
        b1 << b4 << false;

    QTest::newRow("int5") <<
        b1 << b5 << false;

    QTest::newRow("int6") <<
        b1 << b6 << false;

    QTest::newRow("int7") <<
        b1 << b7 << false;

    QTest::newRow("int8") <<
        b1 << b8 << false;

    QTest::newRow("int9") <<
        b1 << b9 << false;

    QTest::newRow("url1") <<
        b4 << b1 << false;

    QTest::newRow("url2") <<
        b4 << b2 << false;

    QTest::newRow("url3") <<
        b4 << b3 << false;

    QTest::newRow("url4") <<
        b4 << b4 << true;

    QTest::newRow("url5") <<
        b4 << b5 << true;

    QTest::newRow("url6") <<
        b4 << b6 << false;

    QTest::newRow("url7") <<
        b4 << b7 << false;

    QTest::newRow("url8") <<
        b4 << b8 << false;

    QTest::newRow("url9") <<
        b4 << b9 << false;

    QTest::newRow("blank1") <<
        b7 << b1 << false;

    QTest::newRow("blank2") <<
        b7 << b2 << false;

    QTest::newRow("blank3") <<
        b7 << b3 << false;

    QTest::newRow("blank4") <<
        b7 << b4 << false;

    QTest::newRow("blank5") <<
        b7 << b5 << false;

    QTest::newRow("blank6") <<
        b7 << b6 << false;

    QTest::newRow("blank7") <<
        b7 << b7 << true;

    QTest::newRow("blank8") <<
        b7 << b8 << true;

    QTest::newRow("blank9") <<
        b7 << b9 << false;
}

void tst_QSparqlBinding::equality_operator()
{
    QFETCH(QSparqlBinding, value1);
    QFETCH(QSparqlBinding, value2);
    QFETCH(bool, isEqual);

    QCOMPARE(value1 == value2, isEqual);
    QCOMPARE(value1 != value2, !isEqual);
}

void tst_QSparqlBinding::assignment_operator()
{
    QSparqlBinding b1("testBinding1", QVariant(-67));
    QSparqlBinding b2 = b1;
    QSparqlBinding b3(b1);

    QCOMPARE(b1 == b2, true);
    QCOMPARE(b1 == b3, true);
}

void tst_QSparqlBinding::clear()
{
    QSparqlBinding b1;
    b1.setName("testBinding1");
    b1.setValue(QVariant(-67));
    QCOMPARE(b1.name(), QString::fromLatin1("testBinding1"));
    QCOMPARE(b1.isLiteral(), true);
    QCOMPARE(b1.value(), QVariant(-67));
    QCOMPARE(b1.dataTypeUri(), QUrl("http://www.w3.org/2001/XMLSchema#int"));
    b1.clear();
    QCOMPARE(b1.name(), QString::fromLatin1("testBinding1"));
    QCOMPARE(b1.isLiteral(), false);
    QCOMPARE(b1.dataTypeUri(), QUrl());
    QCOMPARE(b1.value(), QVariant());

    QSparqlBinding b2("testBinding2", QVariant("Here is some text"));
    b2.setLanguageTag("en");
    QCOMPARE(b2.name(), QString::fromLatin1("testBinding2"));
    QCOMPARE(b2.isLiteral(), true);
    QCOMPARE(b2.dataTypeUri(), QUrl("http://www.w3.org/2001/XMLSchema#string"));
    b2.clear();
    QCOMPARE(b2.name(), QString::fromLatin1("testBinding2"));
    QCOMPARE(b2.languageTag(), QString());
    QCOMPARE(b2.isLiteral(), false);
    QCOMPARE(b2.dataTypeUri(), QUrl());

    QSparqlBinding b3("testBinding3");
    b3.setBlankNodeLabel("b3");
    QCOMPARE(b3.name(), QString::fromLatin1("testBinding3"));
    QCOMPARE(b3.isBlank(), true);
    b3.clear();
    QCOMPARE(b3.name(), QString::fromLatin1("testBinding3"));
    QCOMPARE(b3.isBlank(), false);

    QSparqlBinding b4("testBinding4", QUrl("urn:uri:123"));
    QCOMPARE(b4.name(), QString::fromLatin1("testBinding4"));
    QCOMPARE(b4.isUri(), true);
    QCOMPARE(b4.value(), QVariant(QUrl("urn:uri:123")));
    b4.clear();
    QCOMPARE(b4.name(), QString::fromLatin1("testBinding4"));
    QCOMPARE(b4.isUri(), false);
    QCOMPARE(b4.value(), QVariant());
}

QTEST_MAIN( tst_QSparqlBinding )
#include "tst_qsparqlbinding.moc"
