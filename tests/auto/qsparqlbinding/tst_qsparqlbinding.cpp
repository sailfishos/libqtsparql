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

    QTest::newRow("int") <<
        QVariant(54) <<
        QVariant() <<
        QVariant() <<
        QString("54");

    QTest::newRow("int_with_datatype") <<
        QVariant(54) <<
        QVariant() <<
        QVariant("mydatatype") <<
        QString("\"54\"^^<mydatatype>");

    // FIXME: add test for doubles

    QTest::newRow("string") <<
        QVariant("foo") <<
        QVariant() <<
        QVariant() <<
        QString("\"foo\"");

    QTest::newRow("string_escaped") <<
        QVariant("foo\tbar\nbazwith \"quotes\' and \\ is escaped") <<
        QVariant() <<
        QVariant() <<
        QString("\"foo\\tbar\\nbazwith \\\"quotes\\' and \\\\ is escaped\"");

    QTest::newRow("string_with_datatype") <<
        QVariant("cat") <<
        QVariant() <<
        QVariant("mydatatype") <<
        QString("\"cat\"^^<mydatatype>");

    QTest::newRow("url") <<
        QVariant(QUrl("urn:uri:123")) <<
        QVariant() <<
        QVariant() <<
        QString("<urn:uri:123>");
}

void tst_QSparqlBinding::toString()
{
    QFETCH(QVariant, value);
    QFETCH(QVariant, lang);
    QFETCH(QVariant, datatype);
    QFETCH(QString, toString);

    QSparqlBinding b("testBinding", value);
    if (!lang.isNull())
        b.setLanguageTag(lang.toString());
    if (!datatype.isNull())
        b.setDataTypeUri(datatype.toUrl());

    QCOMPARE(b.toString(), toString);
    QCOMPARE(b.value(), value);
}

void tst_QSparqlBinding::types_data()
{
    QTest::addColumn<QVariant>("value");
    QTest::addColumn<bool>("isResource");
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
    QFETCH(bool, isResource);
    QFETCH(bool, isLiteral);
    QFETCH(bool, isBlank);

    QSparqlBinding b("testBinding", value);

    QCOMPARE(b.isResource(), isResource);
    QCOMPARE(b.isLiteral(), isLiteral);
    QCOMPARE(b.isBlank(), isBlank);
}


QTEST_MAIN( tst_QSparqlBinding )
#include "tst_qsparqlbinding.moc"
