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
#include <QtSparql/QtSparql>
#include <QtDeclarative/qdeclarative.h>
#include <QtDeclarative>
#include <QtSparql/private/qsparqlsparqlconnection_p.h>
#include <QtSparql>

#define NUM_INSERTS 10

class tst_QSparqlQMLBindings : public QObject
{
    Q_OBJECT

public:
    tst_QSparqlQMLBindings();
    virtual ~tst_QSparqlQMLBindings();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void sparql_connection_test();
    void sparql_connection_test_data();
    void sparql_connection_select_query_test();
    void sparql_connection_select_query_async_test();
    void sparql_connection_options_test();
    void sparql_connection_options_test_data();
    void sparql_query_model_test();
    void sparql_query_model_test_data();
    void sparql_query_model_test_role_names();
};

namespace {

// QML stuff
enum Status { Null, Ready, Loading, Error };

QDeclarativeEngine *engine;
QDeclarativeComponent *component;
QDeclarativeContext *context;
QObject* qmlObject;

const QString contactSelectQuery =
    "select ?u ?ng {"
    "    ?u a nco:PersonContact; "
    "    nie:isLogicalPartOf <qsparql-qml-tests> ;"
    "    nco:nameGiven ?ng .}";

template <class T>
T getObject(QString objectName)
{
    T object;
    object = qobject_cast<T>(qmlObject->findChild<QObject*>(objectName));
    return object;
}

QVariant callMethod(QString methodName)
{
    QVariant returnValue;
    QMetaObject::invokeMethod(qmlObject,
                              methodName.toAscii(),
                              Qt::DirectConnection,
                              Q_RETURN_ARG(QVariant, returnValue));
    return returnValue;
}

void qmlCleanup()
{
    // may want to do this as part of a test
    // so set pointers to null as well
    delete qmlObject;
    qmlObject = 0;
    delete component;
    component = 0;
    delete engine;
    engine = 0;
}

bool loadQmlFile(QString fileName, QList<QPair<QString, QVariant> > contextProperties)
{
    engine = new QDeclarativeEngine();
    component = new QDeclarativeComponent(engine, QUrl::fromLocalFile(fileName));
    context = engine->rootContext();

    for(int i=0;i<contextProperties.size();i++) {
        QPair<QString, QVariant> contextPair = contextProperties.at(i);
        context->setContextProperty(contextPair.first, contextPair.second);
    }
    qmlObject = component->create();
    if (component->isError()) {
        qWarning() << component->errors();
        return false;
    }
    return true;
}

}

tst_QSparqlQMLBindings::tst_QSparqlQMLBindings()
{
}

tst_QSparqlQMLBindings::~tst_QSparqlQMLBindings()
{
}

void tst_QSparqlQMLBindings::initTestCase()
{
    const QString insertQueryTemplate =
        "<uri00%1> a nco:PersonContact, nie:InformationElement ;"
        "nie:isLogicalPartOf <qsparql-qml-tests> ;"
        "nco:nameGiven \"name00%1\" .";
    QString insertQuery = "insert { <qsparql-qml-tests> a nie:InformationElement .";
    for (int item = 1; item <= NUM_INSERTS; item++) {
        insertQuery.append( insertQueryTemplate.arg(item) );
    }
    insertQuery.append(" }");

    QSparqlConnection conn("QTRACKER_DIRECT");
    const QSparqlQuery q(insertQuery,QSparqlQuery::InsertStatement);
    QSparqlResult* r = conn.exec(q);
    QVERIFY(!r->hasError());
    r->waitForFinished();
    QVERIFY(!r->hasError());
    delete r;

    QCoreApplication::addLibraryPath("../../../plugins");
    QCoreApplication::addLibraryPath("../../../imports");
}

void tst_QSparqlQMLBindings::cleanupTestCase()
{
    QSparqlConnection conn("QTRACKER");
    const QSparqlQuery q("DELETE { ?u a rdfs:Resource . } "
                         "  WHERE { ?u nie:isLogicalPartOf <qsparql-api-tests> . }"
                         "DELETE { <qsparql-api-tests> a rdfs:Resource . }",
                    QSparqlQuery::DeleteStatement);
    QSparqlResult* r = conn.exec(q);
    QVERIFY(!r->hasError());
    r->waitForFinished();
    QVERIFY(!r->hasError());
    delete r;
}

void tst_QSparqlQMLBindings::init()
{
}

void tst_QSparqlQMLBindings::cleanup()
{
    qmlCleanup();
}

void tst_QSparqlQMLBindings::sparql_connection_test()
{
    QPointer<QSparqlConnection> connPointer =
        getObject<QSparqlConnection*>("connectionWithOptions");

    QVERIFY(connPointer->isValid());
    // now make sure the connection is "Ready"
    Status status = (Status)callMethod("getStatus").toInt();
    QCOMPARE(status, Ready);
    QCOMPARE(connPointer->driverName(), QString("QTRACKER_DIRECT"));
    // now make sure deleting the root object (i.e closing the qml viewer) deletes the connection object
    qmlCleanup();
    QVERIFY(connPointer.isNull());
}

void tst_QSparqlQMLBindings::sparql_connection_test_data()
{
    QList<QPair<QString, QVariant> > contextProperties;
    contextProperties.append(qMakePair(QString("sparqlQueryString"),QVariant(contactSelectQuery)));
    contextProperties.append(qMakePair(QString("setPortNumber"),QVariant(1234)));
    contextProperties.append(qMakePair(QString("setHost"), QVariant("null")));

    QVERIFY(loadQmlFile("qsparqlconnection.qml", contextProperties));
}

void tst_QSparqlQMLBindings::sparql_connection_select_query_test()
{
    // shares data with connection test
    sparql_connection_test_data();

    QSparqlConnection *connection =
        getObject<QSparqlConnection*>("connectionWithOptions");

    QVERIFY(connection->isValid());
    Status status = (Status)callMethod("getStatus").toInt();
    QCOMPARE(status, Ready);
    // get the return value from qml
    QVariant returnValue = callMethod("runSelectQuery");

    QList<QVariant> list = returnValue.toList();
    QCOMPARE(list.length(), NUM_INSERTS);

    QHash<QString, QString> contactNamesValue;

    for (int i=0; i<list.length();i++) {
        QMap<QString, QVariant> resultMap = list[i].toMap();
        // the result map is binding->value, store the binding
        contactNamesValue[resultMap["u"].toString()] = resultMap["ng"].toString();
    }

    //now validate against a QSparqlResult
    QSparqlResult *r = connection->exec(QSparqlQuery(contactSelectQuery));
    QVERIFY(!r->hasError());
    r->waitForFinished();
    QCOMPARE(r->size(), NUM_INSERTS);

    while (r->next()) {
        QCOMPARE(contactNamesValue[r->value(0).toString()], r->value(1).toString());
    }
    delete r;
}

void tst_QSparqlQMLBindings::sparql_connection_select_query_async_test()
{
    sparql_connection_test_data();
    QTime timer;
    int timeoutMs = 2000;
    bool timeout = false;

    QSignalSpy resultSpy(qmlObject, SIGNAL(resultReadySignal()));

    QSparqlConnection connection("QTRACKER_DIRECT");
    QSparqlResult *result = connection.exec(QSparqlQuery(contactSelectQuery));
    result->waitForFinished(); // we want to use size() later

    callMethod("runSelectQueryAsync");
    timer.start();
    // we should get two result ready signals, one from the property change
    // notification (that a new SparqlConnection.result is ready), and one
    // emitted from the javascript function that handels the resultReady() signal
    // from within javascript
    while (resultSpy.count() != 2 && !(timeout = (timer.elapsed() > timeoutMs)))
        QTest::qWait(100);
    QVERIFY(!timeout);
    QCOMPARE(resultSpy.count(), 2);

    // the SparqlConnection.result property should be the same as the
    // result passed to the resultReady() signal slot
    QVariant returnValue = callMethod("returnResults");
    QVariant returnValueFromSlot = callMethod("returnResultsFromSlot");
    QCOMPARE(returnValue, returnValueFromSlot);

    // now compare the qml result with what was returned from the connection
    QList<QVariant> list = returnValue.toList();
    QCOMPARE(list.length(), NUM_INSERTS);
    QHash<QString, QString> contactNamesValue;

    for (int i=0; i<list.length();i++) {
        QMap<QString, QVariant> resultMap = list[i].toMap();
        // the result map is binding->value, store the binding
        contactNamesValue[resultMap["u"].toString()] = resultMap["ng"].toString();
    }

    QCOMPARE(result->size(), NUM_INSERTS);
    while (result->next()) {
        QCOMPARE(contactNamesValue[result->value(0).toString()], result->value(1).toString());
    }

    delete result;
}

void tst_QSparqlQMLBindings::sparql_connection_options_test()
{
    QFETCH(int, portNumber);
    QFETCH(QString, hostName);

    QVariant returnValue = callMethod("returnConnectionOptions");

    // compare the values we set from what was returned
    QMap<QString, QVariant> map = returnValue.toMap();
    QMap<QString, QVariant>::const_iterator i = map.constBegin();
    while (i != map.constEnd()) {
        if (i.key() == "port")
            QCOMPARE(i.value().toInt(), portNumber);
        else if (i.key() == "hostName")
            QCOMPARE(i.value().toString(), hostName);
        else
            QCOMPARE(i.value().toString(), QString(""));
        ++i;
    }
}

void tst_QSparqlQMLBindings::sparql_connection_options_test_data()
{
    int portNumber = 1234;
    QString hostName = "localhost";

    QTest::addColumn<int>("portNumber");
    QTest::addColumn<QString>("hostName");
    QTest::newRow("connection_options_test")
        << portNumber
        << hostName;

    QList<QPair<QString, QVariant> > contextProperties;
    contextProperties.append(qMakePair(QString("sparqlQueryString"),QVariant(contactSelectQuery)));
    contextProperties.append(qMakePair(QString("setPortNumber"),QVariant(portNumber)));
    contextProperties.append(qMakePair(QString("setHost"), QVariant(hostName)));
    QVERIFY(loadQmlFile("qsparqlconnection.qml", contextProperties));
}

void tst_QSparqlQMLBindings::sparql_query_model_test()
{
    QSignalSpy countSpy(qmlObject, SIGNAL(modelCountChanged()));
    QSignalSpy readySpy(qmlObject, SIGNAL(modelStatusReady()));
    QTime timer;
    int timeoutMs = 2000;
    bool timeout = false;

    QSparqlConnection *connection =
        getObject<QSparqlConnection*>("sparqlConnection");
    QPointer<QSparqlQueryModel> model =
        getObject<QSparqlQueryModel *>("queryModel");
    QVERIFY(connection->isValid());

    // status of the model should be "loading" before the model finished();
    QVERIFY(model->rowCount() != NUM_INSERTS);
    Status status = (Status)callMethod("getStatus").toInt();
    QCOMPARE(status, Loading);

    timer.start();

    while ((countSpy.count() != NUM_INSERTS || readySpy.count() != 1) && !(timeout = (timer.elapsed() > timeoutMs)))
        QTest::qWait(100);
    // signal spy should have received NUM_INSERTS count changes
    QVERIFY(!timeout);
    QCOMPARE(countSpy.count(), NUM_INSERTS);
    QCOMPARE(readySpy.count(), 1);
    // status of the model should now be ready
    status = (Status)callMethod("getStatus").toInt();
    QCOMPARE(status, Ready);

    // get the number of rows from the list model in qml
    QVariant returnValue = callMethod("getCount");

    QCOMPARE(NUM_INSERTS, returnValue.toInt());
    QCOMPARE(returnValue.toInt(), model->rowCount());
    QString selectOneContact = "select <uri001> as ?u ?ng {"
                               "<uri001> a nco:PersonContact; nco:nameGiven ?ng; "
                               "nie:isLogicalPartOf <qsparql-qml-tests> }";

    // call set query and change the query model object, this should also update
    // the qml model, which should now go back to loading state
    model->setQueryQML(QSparqlQuery(selectOneContact), *connection);
    status = (Status)callMethod("getStatus").toInt();
    QCOMPARE(status, Loading);
    // Signal spy should now emit twice more, one for the clearing of the model
    // and another for the one contact the query adds
    timer.restart();
    while ((countSpy.count() != NUM_INSERTS+2 ||readySpy.count() != 2) && !(timeout = (timer.elapsed() > timeoutMs)))
        QTest::qWait(100);
    QVERIFY(!timeout);
    QCOMPARE(countSpy.count(), NUM_INSERTS+2);
    QCOMPARE(readySpy.count(), 2);

    status = (Status)callMethod("getStatus").toInt();
    QCOMPARE(status, Ready);
    // now check the count again
    returnValue = callMethod("getCount");

    QCOMPARE(model->rowCount(), 1);
    QCOMPARE(returnValue.toInt(), 1);

    // verify the model is deleted when the QML object is destroyed
    qmlCleanup();
    QVERIFY(model.isNull());
}

void tst_QSparqlQMLBindings::sparql_query_model_test_data()
{
    QList<QPair<QString, QVariant> > contextProperties;
    contextProperties.append(qMakePair(QString("sparqlQueryString"),QVariant(contactSelectQuery)));
    QVERIFY(loadQmlFile("qsparqlresultlist.qml", contextProperties));
}

void tst_QSparqlQMLBindings::sparql_query_model_test_role_names()
{
    QString select1 = "select ?u ?ng ?nf"
                      "{ ?u a nco:PersonContact; nco:nameGiven ?ng; nco:nameFamily ?nf }";
    QString select2 = "select ?u fn:string-join( (?ng, ?nf), ' ') AS ?joinName "
                      "{ ?u a nco:PersonContact; nco:nameGiven ?ng; nco:nameFamily ?nf }";
    QList<QPair<QString, QVariant> > contextProperties;
    contextProperties.append(qMakePair(QString("sparqlQueryString"), QVariant(select1)));
    QVERIFY(loadQmlFile("qsparqlresultlist.qml", contextProperties));

    QSparqlQueryModel *model =
        getObject<QSparqlQueryModel *>("queryModel");
    QSparqlConnection *connection =
        getObject<QSparqlConnection *>("sparqlConnection");

    QHash<int, QByteArray> roleNames = model->roleNames();

    // wait for result
    QTest::qWait(500);
    // role names start from Qt::UserRole + 1,
    QCOMPARE(roleNames[Qt::UserRole+1], QByteArray("u"));
    QCOMPARE(roleNames[Qt::UserRole+2], QByteArray("ng"));
    QCOMPARE(roleNames[Qt::UserRole+3], QByteArray("nf"));

    model->setQueryQML(QSparqlQuery(select2), *connection);
    roleNames = model->roleNames();
    QTest::qWait(500);
    QCOMPARE(roleNames[Qt::UserRole+1], QByteArray("u"));
    QCOMPARE(roleNames[Qt::UserRole+2], QByteArray("joinName"));
}

QTEST_MAIN(tst_QSparqlQMLBindings)
#include "tst_qsparql_qmlbindings.moc"
