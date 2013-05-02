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
#include <QtSparql>

#include <QUrl>

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
    void different_datatypes_data();
    void different_datatypes();
    void copy();
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
    QTest::addColumn<QVariantList>("replacements");
    QTest::addColumn<QString>("replacedString");

    QTest::newRow("nothing") <<
        QString("nothing to replace") <<
        (QStringList() << "foo" << "bar") <<
        (QVariantList() << "FOO" << "BAR") <<
        QString("nothing to replace");

    QTest::newRow("simple") <<
        QString("replace ?:foo ?:bar") <<
        (QStringList() << "foo" << "bar") <<
        (QVariantList() << "FOO" << "BAR") <<
        QString("replace \"FOO\" \"BAR\"");

    QTest::newRow("length_changes") <<
        QString("replace ?:foo and ?:bar also") <<
        (QStringList() << "foo" << "bar") <<
        (QVariantList() << "FOOVAL" << "BARVAL") <<
        QString("replace \"FOOVAL\" and \"BARVAL\" also");

    QTest::newRow("quoted") <<
        QString("do not replace '?:foo' or '?:bar'") <<
        (QStringList() << "foo" << "bar") <<
        (QVariantList() << "FOOVAL" << "BARVAL") <<
        QString("do not replace '?:foo' or '?:bar'");

    QTest::newRow("reallife") <<
        QString("insert { _:c a nco:Contact ; "
                "nco:fullname ?:username ; "
                "nco:hasPhoneNumber _:pn . "
                "_:pn a nco:PhoneNumber ; "
                "nco:phoneNumber ?:userphone . }") <<
        (QStringList() << "username" << "userphone") <<
        (QVariantList() << "NAME" << "PHONE") <<
        QString("insert { _:c a nco:Contact ; "
                "nco:fullname \"NAME\" ; "
                "nco:hasPhoneNumber _:pn . "
                "_:pn a nco:PhoneNumber ; "
                "nco:phoneNumber \"PHONE\" . }");

    QTest::newRow("escape_quotes") <<
        QString("the ?:value goes here") <<
        (QStringList() << "value") <<
        (QVariantList() << "some\"thing") <<
        QString("the \"some\\\"thing\" goes here");
}

void tst_QSparqlQuery::replacement()
{
    QFETCH(QString, rawString);
    QFETCH(QStringList, placeholders);
    QFETCH(QVariantList, replacements);
    QFETCH(QString, replacedString);
    QSparqlQuery q(rawString);

    for (int i = 0; i < placeholders.size(); ++i)
        q.bindValue(placeholders[i], replacements[i]);

    QCOMPARE(q.query(), rawString);
    QCOMPARE(q.preparedQueryText(), replacedString);

    // convenience accessor for bound values
    QMap<QString, QSparqlBinding> bv = q.boundValues();

    for (int i = 0; i < placeholders.size(); ++i) {
        QCOMPARE(q.boundValue(placeholders[i]), replacements[i]);
        QVERIFY(bv.contains(placeholders[i]));
        QCOMPARE(bv[placeholders[i]].value(), replacements[i]);
    }
    QCOMPARE(bv.size(), placeholders.size());
}

void tst_QSparqlQuery::unbind_and_replace_data()
{
    return replacement_data();
}

void tst_QSparqlQuery::unbind_and_replace()
{
    QFETCH(QString, rawString);
    QFETCH(QStringList, placeholders);
    QFETCH(QVariantList, replacements);
    QFETCH(QString, replacedString);
    QSparqlQuery q(rawString, QSparqlQuery::InsertStatement);
    q.unbindValues();

    for (int i = 0; i < placeholders.size(); ++i)
        q.bindValue(placeholders[i], replacements[i]);

    QCOMPARE(q.query(), rawString);
    QCOMPARE(q.preparedQueryText(), replacedString);
}

void tst_QSparqlQuery::different_datatypes_data()
{
    QTest::addColumn<QString>("rawString");
    QTest::addColumn<QStringList>("placeholders");
    QTest::addColumn<QVariantList>("replacements");
    QTest::addColumn<QString>("replacedString");

    QTest::newRow("int") <<
        QString("the ?:value goes here") <<
        (QStringList() << "value") <<
        (QVariantList() << QVariant(64)) <<
        QString("the 64 goes here");

    QTest::newRow("int") <<
        QString("the $:value goes here") <<
        (QStringList() << "value") <<
        (QVariantList() << QVariant(64)) <<
        QString("the 64 goes here");

    /*QTest::newRow("double") <<
        QString("the ?:value goes here") <<
        (QStringList() << "?:value") <<
        (QVariantList() << -3.1) <<
        QString("the -3.1 goes here");*/

    QTest::newRow("bool") <<
        QString("the ?:value goes here") <<
        (QStringList() << "value") <<
        (QVariantList() << true) <<
        QString("the true goes here");

    QTest::newRow("string") <<
        QString("the ?:value goes here") <<
        (QStringList() << "value") <<
        (QVariantList() << "cat") <<
        QString("the \"cat\" goes here");

    QTest::newRow("url") <<
        QString("the ?:value goes here") <<
        (QStringList() << "value") <<
        (QVariantList() << QUrl("http://www.test.com")) <<
        QString("the <http://www.test.com> goes here");
}

void tst_QSparqlQuery::different_datatypes()
{
    QFETCH(QString, rawString);
    QFETCH(QStringList, placeholders);
    QFETCH(QVariantList, replacements);
    QFETCH(QString, replacedString);
    QSparqlQuery q(rawString, QSparqlQuery::InsertStatement);

    for (int i = 0; i < placeholders.size(); ++i)
        q.bindValue(placeholders[i], replacements[i]);

    QCOMPARE(q.query(), rawString);
    QCOMPARE(q.preparedQueryText(), replacedString);
}

void tst_QSparqlQuery::copy()
{
    const QString query1("insert { _:c a nco:Contact ; "
                   "nco:fullname \"NAME\" ; "
                   "nco:hasPhoneNumber _:pn . "
                   "_:pn a nco:PhoneNumber ; "
                   "nco:phoneNumber \"PHONE\" . }");
    const QString query2("select ?r { ?r a nie:InformationElement ; "
                   "nie:isLogicalPartOf <qsparql-query-tests> .}");
    
    const QSparqlQuery::StatementType type1(QSparqlQuery::InsertStatement);
    const QSparqlQuery::StatementType type2(QSparqlQuery::SelectStatement);
    const QSparqlQuery q1(query1, type1);

    // Copy construct q2 from q1
    QSparqlQuery q2(q1);
    QCOMPARE(q2.query(), query1);
    QCOMPARE(q2.type(), type1);

    q2.setType(type2);
    QCOMPARE(q2.type(), type2);
    QCOMPARE(q1.type(), type1);

    q2.setQuery(query2);
    QCOMPARE(q2.query(), query2);
    QCOMPARE(q1.query(), query1);

    // Assign q1 to q2
    q2 = q1;
    QCOMPARE(q2.query(), query1);
    QCOMPARE(q2.type(), type1);

    q2.setType(type2);
    QCOMPARE(q2.type(), type2);
    QCOMPARE(q1.type(), type1);

    q2.setQuery(query2);
    QCOMPARE(q2.query(), query2);
    QCOMPARE(q1.query(), query1);
}

QTEST_MAIN( tst_QSparqlQuery )
#include "tst_qsparqlquery.moc"
