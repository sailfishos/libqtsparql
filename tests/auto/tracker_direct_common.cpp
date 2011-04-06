#include "tracker_direct_common.h"
#include "testhelpers.h"
#include <QtTest/QtTest>
#include <QtSparql/QtSparql>

TrackerDirectCommon::TrackerDirectCommon()
{
}

TrackerDirectCommon::~TrackerDirectCommon()
{
}

QSparqlResult* TrackerDirectCommon::runQuery(QSparqlConnection &conn, const QSparqlQuery &q)
{
    QSparqlResult* r = execQuery(conn, q);
    if (!r) {
        qWarning() << "execQuery() returned empty result";
        return 0;
    }
    waitForQueryFinished(r);
    if (r->hasError()) {
        qWarning() << "QSparqlResult resulted with error: " << r->lastError().message();
        return 0;
    }
    return r;
}

void TrackerDirectCommon::insert_and_delete_contact()
{

    // This test will leave unclean test data in tracker if it crashes.
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlQuery add("insert { <addeduri001> a nco:PersonContact; "
                     "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                     "nco:nameGiven \"addedname001\" .}",
                     QSparqlQuery::InsertStatement);

    QSparqlResult* r = runQuery(conn, add);
    QVERIFY(r!=0);
    QVERIFY(r->isFinished());
    delete r;

    // Verify that the insertion succeeded
    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");
    QHash<QString, QString> contactNames;
    r = runQuery(conn, q);
    QVERIFY(r!=0);

    QVERIFY(checkResultSize(r, 4));
    while (r->next()) {
        contactNames[r->value(0).toString()] =
            r->value(1).toString();
    }
    QVERIFY(r->isFinished());
    CHECK_ERROR(r);
    QCOMPARE(contactNames.size(), 4);
    QCOMPARE(contactNames["addeduri001"], QString("addedname001"));
    delete r;

    // Delete the uri
    QSparqlQuery del("delete { <addeduri001> a rdfs:Resource. }",
                     QSparqlQuery::DeleteStatement);

    r = runQuery(conn, del);
    QVERIFY(r!=0);
    QVERIFY(r->isFinished());
    delete r;

    // Verify that it got deleted
    contactNames.clear();
    r = runQuery(conn, q);
    QVERIFY(r!=0);
    QVERIFY(checkResultSize(r, 3));
    while (r->next()) {
        // A different way for retrieving the results
        contactNames[r->binding(0).value().toString()] =
            r->binding(1).value().toString();
    }
    QVERIFY(r->isFinished());
    QCOMPARE(contactNames.size(), 3);
    delete r;
}

void TrackerDirectCommon::query_with_error()
{
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlQuery q("this is not a valid query");
    QSparqlResult* r = execQuery(conn, q);
    QVERIFY(r!=0);
    waitForQueryFinished(r);
    QVERIFY(r->isFinished());
    QVERIFY(r->hasError());
    QCOMPARE(r->lastError().type(), QSparqlError::StatementError);
    delete r;
}

void TrackerDirectCommon::iterate_result()
{
    // This test will print out warnings
    testLogLevel = QtCriticalMsg;
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                    "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                    "nco:nameGiven ?ng .}");
    QSparqlResult* r = runQuery(conn, q);
    QVERIFY(r!=0);
    QVERIFY(checkResultSize(r, 3));

    QVERIFY(r->pos() == QSparql::BeforeFirstRow);
    // This is not a valid position
    for (int i=-1; i <= 2; ++i) {
        QCOMPARE(r->binding(i), QSparqlBinding());
        QVERIFY(r->value(i).isNull());
    }
    QCOMPARE(r->current(), QSparqlResultRow());

    for (int i=0; i<3; ++i) {
        QVERIFY(r->next());
        QCOMPARE(r->pos(), i);

        QVERIFY(r->binding(-1).value().isNull());
        QVERIFY(r->binding(0).value().isNull() == false);
        QVERIFY(r->binding(1).value().isNull() == false);
        QVERIFY(r->binding(2).value().isNull());

        QVERIFY(r->value(-1).isNull());
        QVERIFY(r->value(0).isNull() == false);
        QVERIFY(r->value(1).isNull() == false);
        QVERIFY(r->value(2).isNull());
    }
    QVERIFY(!r->next());
    QVERIFY(r->pos() == QSparql::AfterLastRow);
    // This is not a valid position
    for (int i=-1; i <= 2; ++i) {
        QCOMPARE(r->binding(i), QSparqlBinding());
        QVERIFY(r->value(i).isNull());
    }
    QCOMPARE(r->current(), QSparqlResultRow());
    QVERIFY(r->isFinished());

    delete r;
}

void TrackerDirectCommon::special_chars()
{
    // This test will leave unclean test data in tracker if it crashes.
    QSparqlConnection conn("QTRACKER_DIRECT");
    QString withSpecialChars("foo\u2780\u2781\u2782");
    QSparqlQuery add(QString("insert { <addeduri002> a nco:PersonContact; "
                             "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                             "nco:nameGiven \"%1\" .}").arg(withSpecialChars),
                             QSparqlQuery::InsertStatement);

    QSparqlResult* r = runQuery(conn, add);
    QVERIFY(r!=0);
    QVERIFY(r->isFinished());
    delete r;

    // Verify that the insertion succeeded
    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");
    QHash<QString, QString> contactNames;
    r = runQuery(conn, q);
    QVERIFY(r!=0);
    QVERIFY(checkResultSize(r, 4));
    while (r->next()) {
        contactNames[r->value(0).toString()] =
            r->value(1).toString();
    }
    CHECK_ERROR(r);
    QVERIFY(r->isFinished());
    QCOMPARE(contactNames.size(), 4);
    QCOMPARE(contactNames["addeduri002"], withSpecialChars);
    delete r;

    // Delete the uri
    QSparqlQuery del("delete { <addeduri002> a rdfs:Resource. }",
                     QSparqlQuery::DeleteStatement);

    r = runQuery(conn, del);
    QVERIFY(r!=0);
    delete r;
}

void TrackerDirectCommon::data_types()
{
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlQuery dataTypes("select "
                           "<qsparql-tracker-direct-tests> "
                           "80 "
                           "23.4 "
                           "true "
                           "false "
                           "\"a string\" "
                           "{ }");
    QSparqlResult* r = runQuery(conn, dataTypes);
    QVERIFY(r!=0);

    QVERIFY(r->next());
    QCOMPARE(r->value(0),
             QVariant(QUrl::fromEncoded("qsparql-tracker-direct-tests")));
    QCOMPARE(r->value(1), QVariant(80));
    QCOMPARE(r->value(2), QVariant(23.4));
    QCOMPARE(r->value(3), QVariant(true));
    QCOMPARE(r->value(4), QVariant(false));
    QCOMPARE(r->value(5), QVariant(QString::fromLatin1("a string")));

    // urls don't have data type uris
    QCOMPARE(r->binding(0).dataTypeUri(),
            QUrl::fromEncoded(""));

    QCOMPARE(r->binding(1).dataTypeUri(),
             QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#integer"));

    QCOMPARE(r->binding(2).dataTypeUri(),
             QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#double"));

    QCOMPARE(r->binding(3).dataTypeUri(),
             QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#boolean"));
    QCOMPARE(r->binding(4).dataTypeUri(),
             QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#boolean"));

    QCOMPARE(r->binding(5).dataTypeUri(),
             QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#string"));

    delete r;
}