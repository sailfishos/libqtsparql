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

//const QString qtest(qTableName( "qtest", __FILE__ )); // FIXME: what's this

//TESTED_FILES=

class tst_QSparqlQuery : public QObject
{
    Q_OBJECT

public:
    tst_QSparqlQuery();
    virtual ~tst_QSparqlQuery();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void replacement_data();
    void replacement();
    void unbind_and_replace_data();
    void unbind_and_replace();

};

tst_QSparqlQuery::tst_QSparqlQuery()
{
}

tst_QSparqlQuery::~tst_QSparqlQuery()
{
}

void tst_QSparqlQuery::initTestCase()
{
}

void tst_QSparqlQuery::cleanupTestCase()
{
}

void tst_QSparqlQuery::init()
{
}

void tst_QSparqlQuery::cleanup()
{
}

void tst_QSparqlQuery::replacement_data()
{
    QTest::addColumn<QString>("rawString");
    QTest::addColumn<QStringList>("placeholders");
    QTest::addColumn<QStringList>("replacements");
    QTest::addColumn<QString>("replacedString");

    QTest::newRow("nothing") <<
        QString("nothing to replace") <<
        (QStringList() << "?:foo" << "?:bar") <<
        (QStringList() << "FOO" << "BAR") <<
        QString("nothing to replace");

    QTest::newRow("simple") <<
        QString("replace ?:foo ?:bar") <<
        (QStringList() << "?:foo" << "?:bar") <<
        (QStringList() << "FOO" << "BAR") <<
        QString("replace FOO BAR");

    QTest::newRow("length_changes") <<
        QString("replace ?:foo and ?:bar also") <<
        (QStringList() << "?:foo" << "?:bar") <<
        (QStringList() << "FOOVAL" << "BARVAL") <<
        QString("replace FOOVAL and BARVAL also");

    QTest::newRow("quoted") <<
        QString("do not replace '?:foo' or '?:bar'") <<
        (QStringList() << "?:foo" << "?:bar") <<
        (QStringList() << "FOOVAL" << "BARVAL") <<
        QString("do not replace '?:foo' or '?:bar'");

    QTest::newRow("reallife") <<
        QString("insert { _:c a nco:Contact ; "
                "nco:fullname ?:username ; "
                "nco:hasPhoneNumber _:pn . "
                "_:pn a nco:PhoneNumber ; "
                "nco:phoneNumber ?:userphone . }") <<
        (QStringList() << "?:username" << "?:userphone") <<
        (QStringList() << "'NAME'" << "'PHONE'") <<
        QString("insert { _:c a nco:Contact ; "
                "nco:fullname 'NAME' ; "
                "nco:hasPhoneNumber _:pn . "
                "_:pn a nco:PhoneNumber ; "
                "nco:phoneNumber 'PHONE' . }");
}

void tst_QSparqlQuery::replacement()
{
    QFETCH(QString, rawString);
    QFETCH(QStringList, placeholders);
    QFETCH(QStringList, replacements);
    QFETCH(QString, replacedString);
    QSparqlQuery q(rawString);

    for (int i = 0; i < placeholders.size(); ++i)
        q.bindValue(placeholders[i], replacements[i]);

    QCOMPARE(q.query(), rawString);
    QCOMPARE(q.preparedQueryText(), replacedString);
}

void tst_QSparqlQuery::unbind_and_replace_data()
{
    return replacement_data();
}

void tst_QSparqlQuery::unbind_and_replace()
{
    QFETCH(QString, rawString);
    QFETCH(QStringList, placeholders);
    QFETCH(QStringList, replacements);
    QFETCH(QString, replacedString);
    QSparqlQuery q(rawString, QSparqlQuery::InsertStatement);
    q.unbindValues();

    for (int i = 0; i < placeholders.size(); ++i)
        q.bindValue(placeholders[i], replacements[i]);

    QCOMPARE(q.query(), rawString);
    QCOMPARE(q.preparedQueryText(), replacedString);
}

QTEST_MAIN( tst_QSparqlQuery )
#include "tst_qsparqlquery.moc"
