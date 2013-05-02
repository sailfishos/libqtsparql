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

#define TEST_PORT 1234
// #define TEST_PORT 1111

class tst_QSparqlVirtuoso : public QObject
{
    Q_OBJECT

public:
    tst_QSparqlVirtuoso();
    virtual ~tst_QSparqlVirtuoso();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    void iterateResults(int);

private slots:
    void query_contacts();
    void construct_contacts();
    void ask_contact();
    void insert_and_delete_contact();
    void query_with_error();
    void select_datatypes();
    void select_blanknode();
    void iterate_on_dataready();

private:
    int previousTotalResults;
	QString driverPath;
	int portNumber;
};

tst_QSparqlVirtuoso::tst_QSparqlVirtuoso()
{
}

tst_QSparqlVirtuoso::~tst_QSparqlVirtuoso()
{
}

void tst_QSparqlVirtuoso::initTestCase()
{
    // For running the test without installing the plugins. Should work in
    // normal and vpath builds.
    QCoreApplication::addLibraryPath("../../../plugins");

    QSettings settings("settings", QSettings::IniFormat);
    QString driver;
    QString driverArg("DRIVER=%1");

    settings.beginGroup("Settings");
    portNumber = settings.value("portNumber").toInt();
    driver = settings.value("driver").toString();
    settings.endGroup();

    if(!portNumber)
        portNumber = 1111;
    if(driver.isNull())
        driver = "/usr/local/virtuoso-opensource/lib/virtodbc_r.so";

    driverPath.append(driverArg.arg(driver));
}

void tst_QSparqlVirtuoso::cleanupTestCase()
{
}

void tst_QSparqlVirtuoso::init()
{
}

void tst_QSparqlVirtuoso::cleanup()
{
}

void tst_QSparqlVirtuoso::query_contacts()
{
    QSparqlConnectionOptions options;
    options.setDatabaseName(driverPath);
    options.setPort(portNumber);
    QSparqlConnection conn("QVIRTUOSO", options);

    QSparqlQuery q("select ?u ?ng "
                   "from <http://virtuoso/testgraph> "
                   " {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-virtuoso-tests> ;"
                   "nco:nameGiven ?ng .}");

    QSparqlResult* r = conn.exec(q);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished(); // this test is synchronous only
    // No prefixes were added and so the query will give an error
    QCOMPARE(r->hasError(), true);
    delete r;

    conn.addPrefix("nco", QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#"));
    conn.addPrefix("nie", QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#"));

    r = conn.exec(q);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished(); // this test is syncronous only
    QCOMPARE(r->hasError(), false);
    QCOMPARE(r->size(), 3);
    QHash<QString, QString> contactNames;
    while (r->next()) {
        QCOMPARE(r->current().count(), 2);
        contactNames[r->value(0).toString()] = r->value(1).toString();
    }
    QCOMPARE(contactNames.size(), 3);
    QCOMPARE(contactNames["uri001"], QString("name001"));
    QCOMPARE(contactNames["uri002"], QString("name002"));
    QCOMPARE(contactNames["uri003"], QString("name003"));
    delete r;
}

void tst_QSparqlVirtuoso::construct_contacts()
{
    // Note that to run this test you will need a patched version of Virtuoso with
    // support for NTriples via a 'define output:format "NT"' option in the query
    QSparqlConnectionOptions options;
    options.setDatabaseName(driverPath);
    options.setPort(portNumber);
    QSparqlConnection conn("QVIRTUOSO", options);
    conn.addPrefix("nco", QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#"));
    conn.addPrefix("nie", QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#"));

    QSparqlQuery q("construct { ?u <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#nameGiven> ?ng } where {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-virtuoso-tests> ;"
                   "nco:nameGiven ?ng .}", QSparqlQuery::ConstructStatement);
    QSparqlResult* r = conn.exec(q);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished(); // this test is syncronous only
    qDebug() << r->lastError();
    QCOMPARE(r->hasError(), false);
    QCOMPARE(r->size(), 3);
    QHash<QString, QString> contactNames;
    while (r->next()) {
        QCOMPARE(r->current().count(), 3);
        contactNames[r->value(0).toString()] = r->value(2).toString();
    }
    QCOMPARE(contactNames.size(), 3);
    QCOMPARE(contactNames["uri001"], QString("name001"));
    QCOMPARE(contactNames["uri002"], QString("name002"));
    QCOMPARE(contactNames["uri003"], QString("name003"));
    delete r;
}

void tst_QSparqlVirtuoso::ask_contact()
{
    QSparqlConnectionOptions options;
    options.setDatabaseName(driverPath);
    options.setPort(portNumber);
    QSparqlConnection conn("QVIRTUOSO", options);
    conn.addPrefix("nco", QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#"));
    conn.addPrefix("nie", QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#"));

    QSparqlQuery q1(" ask from <http://virtuoso/testgraph> { "
                   " ?u a nco:PersonContact; "
                   " nie:isLogicalPartOf <qsparql-virtuoso-tests> ; "
                   "nco:nameGiven \"name001\" . }", QSparqlQuery::AskStatement);
    QSparqlResult* r = conn.exec(q1);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished(); // this test is syncronous only
    QCOMPARE(r->hasError(), false);
    QCOMPARE(r->isBool(), true);
    QCOMPARE(r->boolValue(), true);
    delete r;

    QSparqlQuery q2("ask from <http://virtuoso/testgraph> { "
                   " ?u a nco:PersonContact; "
                   " nie:isLogicalPartOf <qsparql-virtuoso-tests> ; "
                   "nco:nameGiven \"name005\" . }", QSparqlQuery::AskStatement);
    r = conn.exec(q2);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished(); // this test is syncronous only
    QCOMPARE(r->hasError(), false);
    QCOMPARE(r->isBool(), true);
    QCOMPARE(r->boolValue(), false);
    delete r;
}

void tst_QSparqlVirtuoso::insert_and_delete_contact()
{
    // This test will leave unclean test data into virtuoso if it crashes.
    QSparqlConnectionOptions options;
    options.setDatabaseName(driverPath);
    options.setPort(portNumber);
    QSparqlConnection conn("QVIRTUOSO", options);
    conn.addPrefix("nco", QUrl("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#"));
    conn.addPrefix("nie", QUrl("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#"));

    QSparqlQuery add("insert into <http://virtuoso/testgraph> "
                     "{ <addeduri001> a nco:PersonContact; "
                     "nie:isLogicalPartOf <qsparql-virtuoso-tests> ;"
                     "nco:nameGiven \"addedname001\" . }",
                     QSparqlQuery::InsertStatement);

    QSparqlResult* r = conn.exec(add);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished(); // this test is syncronous only
    QCOMPARE(r->hasError(), false);
    delete r;

    // Verify that the insertion succeeded
    QSparqlQuery q("select ?u ?ng from <http://virtuoso/testgraph> { "
                   "?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-virtuoso-tests> ;"
                   "nco:nameGiven ?ng .}");
    QHash<QString, QString> contactNames;
    r = conn.exec(q);
    QVERIFY(r != 0);
    r->waitForFinished();
    QCOMPARE(r->size(), 4);
    while (r->next()) {
        contactNames[r->binding(0).value().toString()] = r->binding(1).value().toString();
    }
    QCOMPARE(contactNames.size(), 4);
    QCOMPARE(contactNames["addeduri001"], QString("addedname001"));
    delete r;

    // Delete the uri
    QSparqlQuery del("DELETE FROM GRAPH <http://virtuoso/testgraph> "
                     "{ <addeduri001> ?p ?o . } "
                     "FROM <http://virtuoso/testgraph> "
                     "WHERE { <addeduri001> ?p ?o . }",
                     QSparqlQuery::DeleteStatement);

    r = conn.exec(del);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished(); // this test is syncronous only
    QCOMPARE(r->hasError(), false);
    delete r;

    // Verify that it got deleted
    contactNames.clear();
    r = conn.exec(q);
    QVERIFY(r != 0);
    r->waitForFinished();
    QCOMPARE(r->size(), 3);
    while (r->next()) {
        contactNames[r->binding(0).value().toString()] = r->binding(1).value().toString();
    }
    QCOMPARE(contactNames.size(), 3);
    delete r;
}

void tst_QSparqlVirtuoso::query_with_error()
{
    QSparqlConnectionOptions options;
    options.setDatabaseName(driverPath);
    options.setPort(portNumber);
    QSparqlConnection conn("QVIRTUOSO", options);

    QSparqlQuery q("this is not a valid query");
    QSparqlResult* r = conn.exec(q);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished(); // this test is syncronous only
    qDebug() << r->lastError();
    QCOMPARE(r->hasError(), true);
    QCOMPARE(r->lastError().type(), QSparqlError::StatementError);
    delete r;
}

void tst_QSparqlVirtuoso::select_datatypes()
{
    QSparqlConnectionOptions options;
    options.setDatabaseName(driverPath);
    options.setPort(portNumber);
    QSparqlConnection conn("QVIRTUOSO", options);

    QSparqlQuery q("select * from <http://virtuoso/testgraph> where { <thing001> ?p ?o . }");
    QSparqlResult* r = conn.exec(q);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished(); // this test is syncronous only
    QCOMPARE(r->hasError(), false);
    QCOMPARE(r->size(), 29);
    QHash<QString, QSparqlBinding> results;

    while (r->next()) {
        QSparqlResultRow resultRow = r->current();
        results[resultRow.binding(0).toString()] = resultRow.binding(1);
    }

    QCOMPARE(results["<string_property>"].toString(), QString("\"A string\"^^<http://www.w3.org/2001/XMLSchema#string>"));
    QCOMPARE(results["<string_language_tag_property>"].toString(), QString("\"Una cadena\"@es"));
    QCOMPARE(results["<string_tab_property>"].toString(), QString("\"A string \\\\t with tab\"^^<http://www.w3.org/2001/XMLSchema#string>"));
    QCOMPARE(results["<string_newline_property>"].toString(), QString("\"A string \\\\n with newline\"^^<http://www.w3.org/2001/XMLSchema#string>"));
    QCOMPARE(results["<string_carriage_return_property>"].toString(), QString("\"A string \\\\r with carriage return\"^^<http://www.w3.org/2001/XMLSchema#string>"));
    QCOMPARE(results["<string_backspace_property>"].toString(), QString("\"A string \\\\b with backspace\"^^<http://www.w3.org/2001/XMLSchema#string>"));
    QCOMPARE(results["<string_single_quote_property>"].toString(), QString("\"A string \\\' with single quote\""));
    QCOMPARE(results["<string_double_quote_property>"].toString(), QString("\"A string \\\" with double quote\""));
    QCOMPARE(results["<string_backslash_property>"].toString(), QString("\"A string \\\\\\\\ with backslash\"^^<http://www.w3.org/2001/XMLSchema#string>"));

    QCOMPARE(results["<integer_property>"].toString(), QString("-1234"));
    QCOMPARE(results["<int_property>"].toString(), QString("\"5678\"^^<http://www.w3.org/2001/XMLSchema#int>"));
    QCOMPARE(results["<nonNegativeInteger_property>"].toString(), QString("9012"));

    // Note that the date and time tests for timezones don't work as Virtuoso doesn't return the timezone
    QCOMPARE(results["<date_property>"].toString(), QString("\"2010-11-30\"^^<http://www.w3.org/2001/XMLSchema#date>"));
    QCOMPARE(results["<date_negative_timezone_property>"].toString(), QString("\"2010-11-30\"^^<http://www.w3.org/2001/XMLSchema#date>"));
    QCOMPARE(results["<date_positive_timezone_property>"].toString(), QString("\"2010-11-30\"^^<http://www.w3.org/2001/XMLSchema#date>"));
    QCOMPARE(results["<time_property>"].toString(), QString("\"12:30:59\"^^<http://www.w3.org/2001/XMLSchema#time>"));
    QCOMPARE(results["<time_negative_timezone_property>"].toString(), QString("\"12:30:59\"^^<http://www.w3.org/2001/XMLSchema#time>"));
    QCOMPARE(results["<time_positive_timezone_property>"].toString(), QString("\"12:30:59\"^^<http://www.w3.org/2001/XMLSchema#time>"));
    QCOMPARE(results["<dateTime_property>"].toString(), QString("\"2010-11-30T12:30:59\"^^<http://www.w3.org/2001/XMLSchema#dateTime>"));
    QCOMPARE(results["<dateTime_negative_timezone_property>"].toString(), QString("\"2010-11-30T12:30:59\"^^<http://www.w3.org/2001/XMLSchema#dateTime>"));
    QCOMPARE(results["<dateTime_positive_timezone_property>"].toString(), QString("\"2010-11-30T12:30:59\"^^<http://www.w3.org/2001/XMLSchema#dateTime>"));

    QCOMPARE(results["<decimal_property>"].toString(), QString("\"1234.56\"^^<http://www.w3.org/2001/XMLSchema#decimal>"));
    QCOMPARE(results["<short_property>"].toString(), QString("\"4567\"^^<http://www.w3.org/2001/XMLSchema#short>"));
    QCOMPARE(results["<long_property>"].toString(), QString("\"123456789\"^^<http://www.w3.org/2001/XMLSchema#long>"));

    // Booleans are just changed to ints of value 0 or 1 by Virtuoso
    QCOMPARE(results["<boolean_property>"].toString(), QString("1"));

    // Originally 4567.123
    QCOMPARE(results["<double_property>"].toString(), QString("4.5671200000e+03"));
    // QCOMPARE(results["<double_property>"].value().toDouble(), 4567.123);

    // Originally 123.45
    QCOMPARE(results["<float_property>"].toString(), QString("1.2344999695e+02"));
    // QCOMPARE(results["<float_property>"].value().toDouble(), 123.45);

    QCOMPARE(results["<base64Binary_property>"].toString(), QString("\"qouh3908t38hohfr\"^^<http://www.w3.org/2001/XMLSchema#base64Binary>"));

}

void tst_QSparqlVirtuoso::select_blanknode()
{
    QSparqlConnectionOptions options;
    options.setDatabaseName(driverPath);
    options.setPort(portNumber);
    QSparqlConnection conn("QVIRTUOSO", options);
    conn.addPrefix("foaf", QUrl("http://xmlns.com/foaf/0.1/"));

    // Example from section 2.10.1 of the SPARQL spec
    QSparqlQuery q("SELECT ?a FROM <http://virtuoso/testgraph> WHERE {"
                   "?a    foaf:givenname   \"Alice\" ."
                   "?a    foaf:family_name \"Hacker\" . }");
    QSparqlResult* r = conn.exec(q);
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished(); // this test is synchronous only
    QCOMPARE(r->hasError(), false);
    QCOMPARE(r->size(), 1);
    r->next();
    QSparqlResultRow resultRow = r->current();
    qDebug() << resultRow.binding(0).name() << resultRow.binding(0).toString();
    QCOMPARE(resultRow.binding(0).isBlank(), true);
}

void tst_QSparqlVirtuoso::iterateResults(int totalResults)
{
    QSparqlResult *r = qobject_cast<QSparqlResult *>(sender());
    r->setPos(previousTotalResults - 1);
    int resultsRead = 0;
    while (r->next() && r->pos() < totalResults) {
        QSparqlResultRow resultRow = r->current();
        // qDebug() << resultRow.binding(0).toString() << resultRow.binding(1).toString() << resultRow.binding(2).toString();
        resultsRead++;
    }

    QCOMPARE(totalResults, previousTotalResults + resultsRead);
    previousTotalResults = totalResults;
}

void tst_QSparqlVirtuoso::iterate_on_dataready()
{
    QSparqlConnectionOptions options;
    options.setDatabaseName(driverPath);
    options.setPort(portNumber);
    QSparqlConnection conn("QVIRTUOSO", options);

    QSparqlQuery q("SELECT ?s ?p ?o WHERE { ?s ?p ?o . }");
    QSparqlResult* r = conn.exec(q);
    previousTotalResults = 0;
    connect(r, SIGNAL(dataReady(int)), SLOT(iterateResults(int)));
    QVERIFY(r != 0);
    QCOMPARE(r->hasError(), false);
    r->waitForFinished(); // this test is synchronous only
    QCOMPARE(r->hasError(), false);
}

QTEST_MAIN( tst_QSparqlVirtuoso )
#include "tst_qsparql_virtuoso.moc"
