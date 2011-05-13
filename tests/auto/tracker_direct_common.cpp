#include "tracker_direct_common.h"
#include "testhelpers.h"
#include <QtTest/QtTest>
#include <QtSparql/QtSparql>

namespace {

    int testLogLevel = QtWarningMsg;

    void myMessageOutput(QtMsgType type, const char *msg)
    {
        switch (type) {
            case QtDebugMsg:
                if (testLogLevel <= 0)
                    fprintf(stderr, "QDEBUG : %s\n", msg);
                break;
            case QtWarningMsg:
                if (testLogLevel <= 1)
                    fprintf(stderr, "QWARN  : %s\n", msg);
                break;
            case QtCriticalMsg:
                if (testLogLevel <= 2)
                    fprintf(stderr, "QCRITICAL: %s\n", msg);
                break;
            case QtFatalMsg:
                if (testLogLevel <= 3)
                    fprintf(stderr, "QFATAL : %s\n", msg);
                abort();
        }
    }

    const QSparqlQuery iterateResultsQuery(
            "select ?u ?ng { ?u a nco:PersonContact; "
                "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                "nco:nameGiven ?ng .}",
            QSparqlQuery::SelectStatement);
    const int iterateResultsExpectedSize = 3;

} // end unnamed namespace


TrackerDirectCommon::TrackerDirectCommon()
    : origMsgHandler(0)
{
}

TrackerDirectCommon::~TrackerDirectCommon()
{
}

QSparqlResult* TrackerDirectCommon::runQuery(QSparqlConnection &conn, const QSparqlQuery &q)
{
    QSparqlResult* r = execQuery(conn, q);
    if (!r) {
        testError("execQuery() returned empty result");
        return 0;
    }
    waitForQueryFinished(r);
    if (r->hasError()) {
        testError(QString("QSparqlResult resulted with error: ") + r->lastError().message());
        return 0;
    }
    return r;
}

void TrackerDirectCommon::installMsgHandler()
{
    QtMsgHandler prevHandler = qInstallMsgHandler(myMessageOutput);
    if (!origMsgHandler)
        origMsgHandler = prevHandler;
}

void TrackerDirectCommon::setMsgLogLevel(int logLevel)
{
    testLogLevel = logLevel;
}

bool TrackerDirectCommon::setupData()
{
    const QString insertQueryTemplate =
        "<uri00%1> a nco:PersonContact, nie:InformationElement ;"
        "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
        "nco:nameGiven \"name00%1\" .";
    QString insertQuery = "insert { <qsparql-tracker-direct-tests> a nie:InformationElement .";
    for (int item = 1; item <= 3; item++) {
        insertQuery.append( insertQueryTemplate.arg(item) );
    }
    insertQuery.append("<uri004> a nie:DataObject , nie:InformationElement ;"
                       "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                       "tracker:available true ."
                       "<uri005> a nie:DataObject , nie:InformationElement ;"
                       "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                       "tracker:available false .");
    insertQuery.append(" }");

    QSparqlConnection conn("QTRACKER");
    QSparqlQuery q(insertQuery,
                   QSparqlQuery::InsertStatement);
    QSparqlResult* r = conn.exec(q);
    if (r == 0)
    {
        testError("Error when inserting setup data for the test case - conn.exec() returned 0");
        return false;
    }
    r->waitForFinished();
    if (r->hasError()) {
        testError(QString("Error when inserting setup data for the test case: ") + r->lastError().message());
        return false;
    }
    delete r;
    return true;
}

bool TrackerDirectCommon::cleanData()
{
    QSparqlConnection conn("QTRACKER");
    QSparqlQuery q("DELETE { ?u a rdfs:Resource . } "
                    "WHERE { ?u nie:isLogicalPartOf <qsparql-tracker-direct-tests> . }"
                    "DELETE { <qsparql-tracker-direct-tests> a rdfs:Resource . }",
                    QSparqlQuery::DeleteStatement);
    QSparqlResult* r = conn.exec(q);
    if (r == 0)
    {
        testError("Error when deleting test data from tracker - conn.exec() returned 0");
        return false;
    }
    r->waitForFinished();
    if (r->hasError()) {
        testError(QString("Error when deleting test data from tracker: ") + r->lastError().message());
        return false;
    }
    delete r;
    return true;
}

void TrackerDirectCommon::testError(const QString& msg)
{
    QtMsgHandler currMsgHandler = 0;
    if (origMsgHandler != 0)
        currMsgHandler = qInstallMsgHandler(origMsgHandler);

    qWarning() << "Test error:" << msg;

    if (currMsgHandler != 0)
        qInstallMsgHandler(currMsgHandler);
}

void TrackerDirectCommon::query_contacts()
{
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlQuery q("select ?u ?ng {?u a nco:PersonContact; "
                   "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
                   "nco:nameGiven ?ng .}");
    QSparqlResult* r = runQuery(conn, q);
    QVERIFY(r != 0);
    QVERIFY(checkResultSize(r, 3));
    QHash<QString, QString> contactNames;
    while (r->next()) {
        QCOMPARE(r->current().count(), 2);
        QCOMPARE(r->stringValue(0), r->value(0).toString());
        QCOMPARE(r->stringValue(1), r->value(1).toString());

        contactNames[r->value(0).toString()] = r->value(1).toString();
    }
    QVERIFY(r->isFinished());
    QCOMPARE(contactNames.size(), 3);
    QCOMPARE(contactNames["uri001"], QString("name001"));
    QCOMPARE(contactNames["uri002"], QString("name002"));
    QCOMPARE(contactNames["uri003"], QString("name003"));

    CHECK_ERROR(r);
    delete r;
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
    // This test will print out warnings
    setMsgLogLevel(QtCriticalMsg);
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
    setMsgLogLevel(QtCriticalMsg);
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlResult* r = runQuery(conn, iterateResultsQuery);
    QVERIFY(r);
    QVERIFY(checkResultSize(r, iterateResultsExpectedSize));

    QVERIFY(r->pos() == QSparql::BeforeFirstRow);

    for (int i=0; i < iterateResultsExpectedSize; ++i) {
        QVERIFY(r->next());
        QCOMPARE(r->pos(), i);
    }
    QVERIFY(!r->next());
    QVERIFY(r->pos() == QSparql::AfterLastRow);
    QVERIFY(!r->next());
    QVERIFY(r->pos() == QSparql::AfterLastRow);

    delete r;
}

void TrackerDirectCommon::iterate_result_rows()
{
    // This test will print out warnings
    setMsgLogLevel(QtCriticalMsg);
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlResult* r = runQuery(conn, iterateResultsQuery);
    QVERIFY(r);

    QCOMPARE(r->current(), QSparqlResultRow());

    while (r->next()) {
        QVERIFY(!r->current().isEmpty());
    }

    QCOMPARE(r->current(), QSparqlResultRow());

    delete r;
}

void TrackerDirectCommon::iterate_result_bindings()
{
    // This test will print out warnings
    setMsgLogLevel(QtCriticalMsg);
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlResult* r = runQuery(conn, iterateResultsQuery);
    QVERIFY(r);

    // This is not a valid position
    for (int i=-1; i <= 2; ++i) {
        QCOMPARE(r->binding(i), QSparqlBinding());
    }

    while (r->next()) {
        QCOMPARE(r->binding(-1), QSparqlBinding());
        QVERIFY(r->binding(0).value().isValid());
        QVERIFY(r->binding(1).value().isValid());
        QCOMPARE(r->binding(2), QSparqlBinding());
    }

    // This is not a valid position
    for (int i=-1; i <= 2; ++i) {
        QCOMPARE(r->binding(i), QSparqlBinding());
    }

    delete r;
}

void TrackerDirectCommon::iterate_result_values()
{
    // This test will print out warnings
    setMsgLogLevel(QtCriticalMsg);
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlResult* r = runQuery(conn, iterateResultsQuery);
    QVERIFY(r);

    // This is not a valid position
    for (int i=-1; i <= 2; ++i) {
        QCOMPARE(r->value(i), QVariant());
    }

    while (r->next()) {
        QCOMPARE(r->value(-1), QVariant());
        QVERIFY(r->value(0).isValid());
        QVERIFY(r->value(1).isValid());
        QCOMPARE(r->value(2), QVariant());
    }

    // This is not a valid position
    for (int i=-1; i <= 2; ++i) {
        QCOMPARE(r->value(i), QVariant());
    }

    delete r;
}

void TrackerDirectCommon::iterate_result_stringValues()
{
    // This test will print out warnings
    setMsgLogLevel(QtCriticalMsg);
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlResult* r = runQuery(conn, iterateResultsQuery);
    QVERIFY(r);

    // This is not a valid position
    for (int i=-1; i <= 2; ++i) {
        QVERIFY(r->stringValue(i).isEmpty());
    }

    while (r->next()) {
        QVERIFY(r->stringValue(-1).isEmpty());
        QVERIFY(!r->stringValue(0).isEmpty());
        QVERIFY(!r->stringValue(1).isEmpty());
        QVERIFY(r->stringValue(2).isEmpty());
    }

    // This is not a valid position
    for (int i=-1; i <= 2; ++i) {
        QVERIFY(r->stringValue(i).isEmpty());
    }

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

void TrackerDirectCommon::explicit_data_types()
{
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlQuery explicitTypes("select "
                               "<qsparql-tracker-direct-tests> "
                               "\"80\"^^xsd:integer "
                               "\"-7\"^^xsd:int "
                               "\"23.4\"^^xsd:double "
                               "\"true\"^^xsd:boolean "
                               "\"false\"^^xsd:boolean "
                               "\"a string\"^^xsd:string "
                               "\"2011-03-28T09:36:00+02:00\"^^xsd:dateTime "
                               "{ }");
    QSparqlResult* r = runQuery(conn, explicitTypes);
    QVERIFY(r!=0);

    QVERIFY(r->next());
    QCOMPARE(r->value(0),
             QVariant(QUrl::fromEncoded("qsparql-tracker-direct-tests")));
    QCOMPARE(r->value(1), QVariant(80));
    QCOMPARE(r->value(2), QVariant(-7));
    QCOMPARE(r->value(3), QVariant(23.4));
    QCOMPARE(r->value(4), QVariant(true));
    QCOMPARE(r->value(5), QVariant(false));
    QCOMPARE(r->value(6), QVariant(QString::fromLatin1("a string")));

    // Tracker seems to return the datetime as a string
    QCOMPARE(r->value(7),
             QVariant(QDateTime::fromString("2011-03-28T09:36:00+02:00", Qt::ISODate)));

    // urls don't have data type uris
    QCOMPARE(r->binding(0).dataTypeUri(),
            QUrl::fromEncoded(""));

    QCOMPARE(r->binding(1).dataTypeUri(),
             QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#integer"));
    QCOMPARE(r->binding(2).dataTypeUri(),
             QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#integer"));

    QCOMPARE(r->binding(3).dataTypeUri(),
             QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#double"));

    QCOMPARE(r->binding(4).dataTypeUri(),
             QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#boolean"));
    QCOMPARE(r->binding(5).dataTypeUri(),
             QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#boolean"));

    QCOMPARE(r->binding(6).dataTypeUri(),
             QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#string"));

    QCOMPARE(r->binding(7).dataTypeUri(),
             QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#dateTime"));

    delete r;
}

void TrackerDirectCommon::large_integer()
{
    QSparqlQuery insert("insert {<mydataobject> a nie:DataObject, nie:InformationElement ; "
                        "nie:isLogicalPartOf <qsparql-tracker-live-tests> ;"
                        "nie:byteSize 5000000000 .}", QSparqlQuery::InsertStatement);
    QSparqlConnection conn("QTRACKER_DIRECT");
    QSparqlResult* r = runQuery(conn, insert);
    QVERIFY(r!=0);
    delete r;
    QSparqlQuery select("select ?do ?bs "
                        "{ ?do a nie:DataObject ; "
                        "nie:isLogicalPartOf <qsparql-tracker-live-tests> ;"
                        "nie:byteSize ?bs .}");
    r = runQuery(conn, select);
    QVERIFY(r!=0);

    QVERIFY(r->next());
    QCOMPARE(r->value(1), QVariant(Q_UINT64_C(5000000000)));
    delete r;

    QSparqlQuery clean("delete {<mydataobject> a rdfs:Resource . }",
                       QSparqlQuery::DeleteStatement);
    r = runQuery(conn, clean);
    QVERIFY(r!=0);
    delete r;
}


void TrackerDirectCommon::datatype_string_data()
{
    QString property = "nie:mimeType";
    QString classes = "";
    QString dataType = "http://www.w3.org/2001/XMLSchema#string";

    QTest::newRow("emptyString") <<
        property <<
        classes <<
        "\"\"" <<
        QVariant("") <<
        dataType;
    QTest::newRow("nonemptyString") <<
        property <<
        classes <<
        "\"nonempty\"" <<
        QVariant("nonempty") <<
        dataType;
}

void TrackerDirectCommon::datatype_int_data()
{
    QString property = "nie:byteSize";
    QString classes = ", nie:DataObject";
    QString dataType = "http://www.w3.org/2001/XMLSchema#integer";

    QTest::newRow("typicalNegativeInteger") <<
        property <<
        classes <<
        "-600" <<
        QVariant(qlonglong(-600)) <<
        dataType;
    QTest::newRow("typicalPositiveInteger") <<
        property <<
        classes <<
        "800" <<
        QVariant(qlonglong(800)) <<
        dataType;
    QTest::newRow("zeroInterger") <<
        property <<
        classes <<
        "0" <<
        QVariant(qlonglong(0)) <<
        dataType;
    QTest::newRow("bigInterger") <<
        property <<
        classes <<
        "9223372036854775807" <<
        QVariant(qlonglong(Q_UINT64_C(9223372036854775807))) <<
        dataType;
    QTest::newRow("smallInterger") <<
        property <<
        classes <<
        "-9223372036854775808" <<
        QVariant(qlonglong(Q_UINT64_C(-9223372036854775808))) <<
        dataType;
}

void TrackerDirectCommon::datatype_double_data()
{
    QString property = "slo:latitude";
    QString classes = ", slo:GeoLocation";
    QString dataType = "http://www.w3.org/2001/XMLSchema#double";

    QTest::newRow("typicalNegativeDouble") <<
        property <<
        classes <<
        "-300.123456789" <<
        QVariant(-300.123456789) <<
        dataType;
    QTest::newRow("typicalPositiveDouble") <<
        property <<
        classes <<
        "987.09877" <<
        QVariant(987.09877) <<
        dataType;
    QTest::newRow("zeroDouble") <<
        property <<
        classes <<
        "0" <<
        QVariant(0.0) <<
        dataType;
    QTest::newRow("maxSignificantDigitsDouble") <<
        property <<
        classes <<
        "-0.12345678901234" <<
        QVariant(-0.12345678901234) <<
        dataType;
    QTest::newRow("maxSignificantDigitsDouble2") <<
        property <<
        classes <<
        "0.12345678901234" <<
        QVariant(0.12345678901234) <<
        dataType;
    QTest::newRow("withExponentDouble") <<
        property <<
        classes <<
        "0.12345678901234e+19" <<
        QVariant(0.12345678901234e+19) <<
        dataType;
    QTest::newRow("withExponentDouble3") <<
        property <<
        classes <<
        "-0.12345678901234e+19" <<
        QVariant(-0.12345678901234e+19) <<
        dataType;
    QTest::newRow("withExponentDouble4") <<
        property <<
        classes <<
        "-0.12345678901234e-19" <<
        QVariant(-0.12345678901234e-19) <<
        dataType;
}

void TrackerDirectCommon::datatype_datetime_data()
{
    QString property = "nco:birthDate";
    QString classes = ", nco:PersonContact";
    QString dataType = "http://www.w3.org/2001/XMLSchema#dateTime";
    QTest::newRow("typicalDateTime") <<
        property <<
        classes <<
        "\"1953-03-16T03:20:12Z\"" <<
        QVariant(QDateTime::fromString("1953-03-16T03:20:12Z", Qt::ISODate)) <<
        dataType;
}

void TrackerDirectCommon::datatype_boolean_data()
{
    QString property = "tracker:available";
    QString classes = ", nie:DataObject";
    QString dataType = "http://www.w3.org/2001/XMLSchema#boolean";

    QTest::newRow("trueBool") <<
        property <<
        classes <<
        "true" <<
        QVariant(true) <<
        dataType;
    QTest::newRow("falseBool") <<
        property <<
        classes <<
        "false" <<
        QVariant(false) <<
        dataType;
}

void TrackerDirectCommon::datatypes_as_properties_data()
{
    // which property in the ontology can be used for inserting this type of
    // data
    QTest::addColumn<QString>("property");
    // what classes the test resource needs to belong to (in addition to
    // nie:InformationElement)
    QTest::addColumn<QString>("classes");
    // string representation of the value, inserted into the query
    QTest::addColumn<QString>("string");
    // Expected value: what QVariant we should get back
    QTest::addColumn<QVariant>("expectedValue");
    // Expected value: data type uri of the constructed QSparqlBinding
    QTest::addColumn<QString>("dataTypeUri");

    // different data sets
    datatype_string_data();
    datatype_int_data();
    datatype_double_data();
    datatype_datetime_data();
    datatype_boolean_data();
}

void TrackerDirectCommon::datatypes_as_properties()
{
    QFETCH(QString, property);
    QFETCH(QString, classes);
    QFETCH(QString, string);
    QFETCH(QVariant, expectedValue);
    QFETCH(QString, dataTypeUri);

    QSparqlConnection conn("QTRACKER_DIRECT");

    // Insert the test data.
    QSparqlQuery insertQuery(
        QString("insert { "
        "<testresource> a nie:InformationElement %1 ; "
        "nie:isLogicalPartOf <qsparql-tracker-direct-tests> ;"
        "%2 %3 . }").arg(classes).arg(property).arg(string),
        QSparqlQuery::InsertStatement);

    QSparqlResult* r = runQuery(conn, insertQuery);
    QVERIFY(r!=0);
    CHECK_ERROR(r);
    delete r;

    QSparqlQuery query(QString("select ?value { "
                                "<testresource> %1 ?value . "
                                "}").arg(property));

    r = runQuery(conn, query);
    QVERIFY(r!=0);
    CHECK_ERROR(r);

    QVERIFY(r->next());
    QVariant value = r->value(0);
    QSparqlBinding binding = r->binding(0);
    QVERIFY(!r->next());

    QSparqlQuery cleanString("delete { <testresource> a rdfs:Resource . }",
                             QSparqlQuery::DeleteStatement);
    QSparqlResult* deleteResult = runQuery(conn, cleanString);
    QVERIFY(deleteResult!=0);
    CHECK_ERROR(deleteResult);
    delete deleteResult;

    QCOMPARE(value.type(), expectedValue.type());
    QCOMPARE(binding.value().type(), expectedValue.type());
    QCOMPARE(binding.name(), QString("value"));
    QCOMPARE(binding.dataTypeUri().toString(), dataTypeUri);

    // Compare the actual values fuzzily, QCOMPARE with QVariant doesn't do that
    // (though QCOMPARE with doubles does).
    if (expectedValue.type() == QVariant::Double) {
        QCOMPARE(value.toDouble(), expectedValue.toDouble());
        QCOMPARE(binding.value().toDouble(), expectedValue.toDouble());
    }
    else {
        QCOMPARE(value, expectedValue);
        QCOMPARE(binding.value(), expectedValue);
    }

    delete r;
}

class TestDataImpl : public TestData {

public:
    TestDataImpl(const QSparqlQuery& cleanupQuery, const QSparqlQuery& selectQuery);
    ~TestDataImpl();
    void setOK();
    bool isOK() const;
    QSparqlQuery selectQuery() const;
private:
    QSparqlQuery cleanupQuery;
    QSparqlQuery selectQuery_;
    bool ok;
};

TestData* createTestData(int testDataAmount, const QString& testSuiteTag, const QString& testCaseTag)
{
    const int insertBatchSize = 200;
    QSparqlConnection conn("QTRACKER");
    const QString insertQueryTemplate =
            "<urn:music:%1> a nmm:MusicPiece, nfo:FileDataObject, nfo:Audio;"
                "nie:isLogicalPartOf %3 ;"
                "nie:isLogicalPartOf %4 ;"
                "tracker:available          true ;"
                "nie:byteSize               \"0\" ;"
                "nie:url                    \"file://music/Song_%1.mp3\" ;"
                "nfo:belongsToContainer     <file://music/> ;"
                "nie:title                  \"Song %1\" ;"
                "nie:mimeType               \"audio/mpeg\" ;"
                "nie:contentCreated         \"2000-01-01T01:01:01Z\" ;"
                "nie:isLogicalPartOf        <urn:album:%2> ;"
                "nco:contributor            <urn:artist:%2> ;"
                "nfo:fileLastAccessed       \"2000-01-01T01:01:01Z\" ;"
                "nfo:fileSize               \"0\" ;"
                "nfo:fileName               \"Song_%1.mp3\" ;"
                "nfo:fileLastModified       \"2000-01-01T01:01:01Z\" ;"
                "nfo:codec                  \"MPEG\" ;"
                "nfo:averageBitrate         \"16\" ;"
                "nfo:genre                  \"Genre %2\" ;"
                "nfo:channels               \"2\" ;"
                "nfo:sampleRate             \"44100.0\" ;"
                "nmm:musicAlbum             <urn:album:%2> ;"
                "nmm:musicAlbumDisc         <urn:album-disk:%2> ;"
                "nmm:performer              <urn:artist:%2> ;"
                "nfo:duration               \"1\" ;"
                "nmm:trackNumber            \"%1\" .";

    const QSparqlQuery cleanupQuery(
        QString("delete { "
            "?u a rdfs:Resource . } "
            "where { "
            "?u nie:isLogicalPartOf %1 . "
            "?u nie:isLogicalPartOf %2 . "
            "} "
            "delete {"
            "%2 a rdfs:Resource . "
            "}").arg(testSuiteTag).arg(testCaseTag),
        QSparqlQuery::DeleteStatement);

    const QSparqlQuery selectQuery(
        QString("select tracker:id(?musicPiece) ?title ?performer ?album ?duration ?created "
            "{ "
            "?musicPiece a nmm:MusicPiece; "
            "nie:isLogicalPartOf %1; "
            "nie:isLogicalPartOf %2; "
            "nie:title ?title; "
            "nmm:performer ?performer; "
            "nmm:musicAlbum ?album; "
            "nfo:duration ?duration; "
            "nie:contentCreated ?created. "
            "} order by ?title ?created")
                .arg(testSuiteTag)
                .arg(testCaseTag));

    TestDataImpl* testData = new TestDataImpl(cleanupQuery, selectQuery);

    QString insertTagsQuery = QString("insert { %1 a nie:InformationElement. "
                                               "%2 a nie:InformationElement. "
                                               "%2 nie:isLogicalPartOf %1. }")
                                        .arg(testSuiteTag).arg(testCaseTag);
    QScopedPointer<QSparqlResult> r(conn.syncExec(QSparqlQuery(insertTagsQuery, QSparqlQuery::InsertStatement)));
    if (r.isNull() || r->hasError())
        return testData;
    r.reset();

    for (int item = 1; item <= testDataAmount; ) {
        QString insertQuery = "insert { ";
        int itemDozen = 10;
        const int batchEnd = item + insertBatchSize;
        for (; item < batchEnd && item <= testDataAmount; ++item) {
            insertQuery.append( insertQueryTemplate
                                    .arg(item)
                                    .arg(itemDozen)
                                    .arg(testSuiteTag)
                                    .arg(testCaseTag) );
            if (item % 10 == 0) itemDozen += 10;
        }
        insertQuery.append(" }");
        QScopedPointer<QSparqlResult> r(conn.syncExec(QSparqlQuery(insertQuery, QSparqlQuery::InsertStatement)));
        if (r.isNull() || r->hasError())
            return testData;
    }

    testData->setOK();
    return testData;
}

TestDataImpl::TestDataImpl(const QSparqlQuery& cleanupQuery, const QSparqlQuery& selectQuery)
    : cleanupQuery(cleanupQuery), selectQuery_(selectQuery), ok(false)
{
}

TestDataImpl::~TestDataImpl()
{
    QSparqlConnection conn("QTRACKER");
    conn.syncExec(cleanupQuery);
}

void TestDataImpl::setOK()
{
    ok = true;
}

bool TestDataImpl::isOK() const
{
    return ok;
}

QSparqlQuery TestDataImpl::selectQuery() const
{
    return selectQuery_;
}

