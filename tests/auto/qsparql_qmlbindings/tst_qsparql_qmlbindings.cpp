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
#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtQml/qqml.h>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <private/qsparqlsparqlconnection-qt5_p.h>
#else
#include <QtDeclarative/qdeclarative.h>
#include <QtDeclarative>
#include <private/qsparqlsparqlconnection_p.h>
#endif
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
    void sparql_connection_ask_query_test();
    void sparql_connection_ask_query_async_test();
    void sparql_connection_update_query_test(); //insert and delete
    void sparql_connection_update_query_async_test();
    void sparql_connection_construct_query_test();
    void sparql_connection_bound_values_test();
    void sparql_connection_options_test();
    void sparql_connection_options_test_data();
    void sparql_query_model_test();
    void sparql_query_model_test_data();
    void sparql_query_model_reload_test();
    void sparql_query_model_get_test();
    void sparql_query_model_test_role_names();
    void sparql_legacy_test();

};

namespace {

// QML stuff
enum Status { Null, Ready, Loading, Error };

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
QQmlEngine *engine;
QQmlComponent *component;
QQmlContext *context;
#else
QDeclarativeEngine *engine;
QDeclarativeComponent *component;
QDeclarativeContext *context;
#endif
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

QVariant callMethod(QString methodName, QVariant parameter = QVariant())
{
    QVariant returnValue;
    if (parameter.isNull()) {
        QMetaObject::invokeMethod(qmlObject,
                                methodName.toLatin1(),
                                Qt::DirectConnection,
                                Q_RETURN_ARG(QVariant, returnValue));
    } else {
        QMetaObject::invokeMethod(qmlObject,
                                methodName.toLatin1(),
                                Qt::DirectConnection,
                                Q_RETURN_ARG(QVariant, returnValue),
                                Q_ARG(QVariant, parameter));
    }


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
    QFileInfo fileInfo(qApp->arguments()[0]);

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    engine = new QQmlEngine();
#else
    engine = new QDeclarativeEngine();
#endif
    engine->addImportPath("../../../imports");
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    component = new QQmlComponent(engine, QUrl::fromLocalFile(fileInfo.absolutePath()+"/"+fileName+"-qt5.qml"));
#else
    component = new QDeclarativeComponent(engine, QUrl::fromLocalFile(fileInfo.absolutePath()+"/"+fileName+".qml"));
#endif
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

void compareResults(QVariant qmlResults, int resultSize, QString query)
{
    QSparqlConnection *connection = new QSparqlConnection("QTRACKER_DIRECT");
    QSparqlResult *result = connection->exec(QSparqlQuery(query));
    result->waitForFinished(); // for size();

    QList<QVariant> list = qmlResults.toList();
    QCOMPARE(list.length(), resultSize);
    QHash<QString, QString> contactNamesValue;

    for (int i=0; i<list.length();i++) {
        QMap<QString, QVariant> resultMap = list[i].toMap();
        // the result map is binding->value, store the binding
        contactNamesValue[resultMap["u"].toString()] = resultMap["ng"].toString();
    }

    QCOMPARE(result->size(), resultSize);

    while (result->next()) {
        QCOMPARE(contactNamesValue[result->value(0).toString()], result->value(1).toString());
    }
    delete result;
    delete connection;
}

bool waitForSignals(QSignalSpy *signalSpy, int numSignals)
{
    QTime timer;
    int timeoutMs = 2000;
    bool timeout = false;
    timer.start();

    while (signalSpy->count() !=numSignals && !(timeout = (timer.elapsed() > timeoutMs)))
        QTest::qWait(100);

    if (signalSpy->count() != numSignals) {
        qWarning() << "Got" << signalSpy->count() << "signals, expected" << numSignals;
        return false;
    }
    return !timeout;
}

} // end namespace

tst_QSparqlQMLBindings::tst_QSparqlQMLBindings()
{
}

tst_QSparqlQMLBindings::~tst_QSparqlQMLBindings()
{
}

void tst_QSparqlQMLBindings::initTestCase()
{
    QCoreApplication::addLibraryPath("../../../plugins");
    QCoreApplication::addLibraryPath("../../../imports");

    const QString insertQueryTemplate =
        "<qml-uri00%1> a nco:PersonContact, nie:InformationElement ;"
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
}

void tst_QSparqlQMLBindings::cleanupTestCase()
{
    QSparqlConnection conn("QTRACKER");
    const QSparqlQuery q("DELETE { ?u a rdfs:Resource . } "
                         "  WHERE { ?u nie:isLogicalPartOf <qsparql-qml-tests> . }"
                         "DELETE { <qsparql-qml=tests> a rdfs:Resource . }",
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

    QVERIFY(loadQmlFile("qsparqlconnection", contextProperties));
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
    compareResults(returnValue, NUM_INSERTS, contactSelectQuery);
}

void tst_QSparqlQMLBindings::sparql_connection_select_query_async_test()
{
    sparql_connection_test_data();
    QSignalSpy resultSpy(qmlObject, SIGNAL(resultReadySignal()));

    callMethod("runSelectQueryAsync");
    // we should get two result ready signals, one from the property change
    // notification (that a new SparqlConnection.result is ready), and one
    // emitted from the javascript function that handels the resultReady() signal
    // from within javascript
    QVERIFY(waitForSignals(&resultSpy, 2));

    // the SparqlConnection.result property should be the same as the
    // result passed to the resultReady() signal slot
    QVariant returnValue = callMethod("returnResults");
    QVariant returnValueFromSlot = callMethod("returnResultsFromSlot");
    QCOMPARE(returnValue, returnValueFromSlot);
    compareResults(returnValue, NUM_INSERTS, contactSelectQuery);
    compareResults(returnValueFromSlot, NUM_INSERTS, contactSelectQuery);
}

void tst_QSparqlQMLBindings::sparql_connection_ask_query_test()
{
    sparql_connection_test_data();
    QString askQuery = "ASK { <qml-uri001> a nco:PersonContact }";
    QVariant result = callMethod("runAskQuery", askQuery);
    QVERIFY(result.canConvert(QVariant::Bool));
    QVERIFY(result.toBool());
    askQuery = "ASK { <falseask> a nco:PersonContact }";
    result = callMethod("runAskQuery", askQuery);
    QVERIFY(result.canConvert(QVariant::Bool));
    QVERIFY(!result.toBool());
}

void tst_QSparqlQMLBindings::sparql_connection_ask_query_async_test()
{
    sparql_connection_test_data();
    QSignalSpy resultSpy(qmlObject, SIGNAL(resultReadySignal()));
    QString askQuery = "ASK { <qml-uri001> a nco:PersonContact }";
    callMethod("runAskQueryAsync", askQuery);
    QVERIFY(waitForSignals(&resultSpy, 1));

    QVariant returnValue = callMethod("returnResults");
    QVERIFY(returnValue.canConvert(QVariant::Bool));
    QVERIFY(returnValue.toBool());

    askQuery = "ASK { <falseask> a nco:PersonContact }";
    resultSpy.clear();
    callMethod("runAskQueryAsync", askQuery);
    QVERIFY(waitForSignals(&resultSpy, 1));

    returnValue = callMethod("returnResults");
    QVERIFY(returnValue.canConvert(QVariant::Bool));
    QVERIFY(!returnValue.toBool());
}

void tst_QSparqlQMLBindings::sparql_connection_update_query_test()
{
    sparql_connection_test_data();

    callMethod("insertContact"); // inserts one new contact
    QVariant returnValue = callMethod("runSelectQuery");
    compareResults(returnValue, NUM_INSERTS+1, contactSelectQuery);

    // now delete the result
    callMethod("deleteContact");
    returnValue = callMethod("runSelectQuery");
    compareResults(returnValue, NUM_INSERTS, contactSelectQuery);
}

void tst_QSparqlQMLBindings::sparql_connection_update_query_async_test()
{
    sparql_connection_test_data();

    QSignalSpy resultSpy(qmlObject, SIGNAL(resultReadySignal()));
    callMethod("insertContactAsync");

    QVERIFY(waitForSignals(&resultSpy, 1));

    QVariant returnValue = callMethod("runSelectQuery");
    compareResults(returnValue, NUM_INSERTS+1, contactSelectQuery);

    callMethod("deleteContactAsync");
    // we got a signal from "runSelectQuery" once the results are done, so need to
    // wait for the signal from the delete query, hence 3
    QVERIFY(waitForSignals(&resultSpy, 3));

    returnValue = callMethod("runSelectQuery");
    compareResults(returnValue, NUM_INSERTS, contactSelectQuery);
}

void tst_QSparqlQMLBindings::sparql_connection_construct_query_test()
{
    // tracker doesn't support construct queries, so this should
    // be in error. TODO: Test with endpoint driver
    sparql_connection_test_data();
    QVariant returnValue = callMethod("runConstructQuery");
    QCOMPARE(returnValue.toInt(), -1);
    Status status = (Status)callMethod("getStatus").toInt();
    QCOMPARE(status, Error);
    returnValue = callMethod("getLastError");
    QCOMPARE(returnValue.toString(), QString("Unsupported statement type"));
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
    QVERIFY(loadQmlFile("qsparqlconnection", contextProperties));
}

void tst_QSparqlQMLBindings::sparql_connection_bound_values_test()
{
    sparql_connection_test_data();
    // insert a new contact using bound values
    callMethod("insertBoundContact");
    // ask to see if contact was inserted, using bound values
    QVariant returnValue = callMethod("askBoundContact");
    QVERIFY(returnValue.canConvert(QVariant::Bool));
    QVERIFY(returnValue.toBool());
    // check the select value
    QList<QVariant> selectValue = callMethod("selectBoundContact").toList();
    // should only be one contact
    QCOMPARE(selectValue.size(), 1);

    // Now delete the contact
    callMethod("deleteBoundContact");
    // ask the contact again, should be false
    returnValue = callMethod("askBoundContact");
    QVERIFY(!returnValue.toBool());
    selectValue = callMethod("selectBoundContact").toList();
    QCOMPARE(selectValue.size(), 0);
}

void tst_QSparqlQMLBindings::sparql_query_model_test()
{
    QSignalSpy countSpy(qmlObject, SIGNAL(modelCountChanged()));
    QSignalSpy readySpy(qmlObject, SIGNAL(modelStatusReady()));

    QSparqlConnection *connection =
        getObject<QSparqlConnection*>("sparqlConnection");
    QPointer<QSparqlQueryModel> model =
        getObject<QSparqlQueryModel *>("queryModel");
    QVERIFY(connection->isValid());

    // status of the model should be "loading" before the model finished();
    QVERIFY(model->rowCount() != NUM_INSERTS);
    Status status = (Status)callMethod("getStatus").toInt();
    QCOMPARE(status, Loading);
    QVERIFY(waitForSignals(&countSpy, NUM_INSERTS));
    QVERIFY(waitForSignals(&readySpy, 1));

    // status of the model should now be ready
    status = (Status)callMethod("getStatus").toInt();
    QCOMPARE(status, Ready);

    // get the number of rows from the list model in qml
    QVariant returnValue = callMethod("getCount");

    QCOMPARE(NUM_INSERTS, returnValue.toInt());
    QCOMPARE(returnValue.toInt(), model->rowCount());

    // verify the model is deleted when the QML object is destroyed
    qmlCleanup();
    QVERIFY(model.isNull());
}

void tst_QSparqlQMLBindings::sparql_query_model_test_data()
{
    QList<QPair<QString, QVariant> > contextProperties;
    contextProperties.append(qMakePair(QString("sparqlQueryString"),QVariant(contactSelectQuery)));
    QVERIFY(loadQmlFile("qsparqlresultlist", contextProperties));
}

void tst_QSparqlQMLBindings::sparql_query_model_reload_test()
{
    sparql_query_model_test_data();
    QSignalSpy countSpy(qmlObject, SIGNAL(modelCountChanged()));
    QSignalSpy readySpy(qmlObject, SIGNAL(modelStatusReady()));

    // wait for the model to finish
    QVERIFY(waitForSignals(&readySpy, 1));
    // we used a query with one contact, check to see the count was correct
    QVERIFY(waitForSignals(&countSpy, NUM_INSERTS));
    QVariant returnValue = callMethod("getCount");
    QCOMPARE(returnValue.toInt(), NUM_INSERTS);

    readySpy.clear();
    countSpy.clear();
    // check the status is ready
    Status status = (Status)callMethod("getStatus").toInt();
    QCOMPARE(status, Ready);
    // now reload the model, we should get one signal after clearing the model
    // model, and 10 inserts
    callMethod("reloadModel");
    // reloading the model should set the status to loading again
    status = (Status)callMethod("getStatus").toInt();
    QCOMPARE(status, Loading);
    QVERIFY(waitForSignals(&readySpy, 1));
    QVERIFY(waitForSignals(&countSpy, NUM_INSERTS+1));
    // should be ready when finished
    status = (Status)callMethod("getStatus").toInt();
    QCOMPARE(status, Ready);
}

void tst_QSparqlQMLBindings::sparql_query_model_get_test()
{
    sparql_query_model_test_data();
    QSignalSpy countSpy(qmlObject, SIGNAL(modelCountChanged()));

    QSparqlQueryModel *model =
        getObject<QSparqlQueryModel *>("queryModel");

    // wait for the model to finish
    QVERIFY(waitForSignals(&countSpy, NUM_INSERTS));

    // now check that the result from get(i) is the same
    // as the result from model.getRow()
    for (int i=0;i<model->rowCount();i++) {
        QSparqlResultRow modelRow = model->resultRow(i);
        QVariantMap row = callMethod("getRow", i).toMap();

        QString binding0 = modelRow.binding(0).name();
        QString binding1 = modelRow.binding(1).name();
        QVariant value0 = modelRow.value(0);
        QVariant value1 = modelRow.value(1);

        QCOMPARE(row[binding0], value0);
        QCOMPARE(row[binding1], value1);
    }
}

void tst_QSparqlQMLBindings::sparql_query_model_test_role_names()
{
    QString select1 = "select ?u ?ng ?nf"
                      "{ ?u a nco:PersonContact; nco:nameGiven ?ng; nco:nameFamily ?nf }";
    QString select2 = "select ?u fn:string-join( (?ng, ?nf), ' ') AS ?joinName "
                      "{ ?u a nco:PersonContact; nco:nameGiven ?ng; nco:nameFamily ?nf }";
    QList<QPair<QString, QVariant> > contextProperties;
    contextProperties.append(qMakePair(QString("sparqlQueryString"), QVariant(select1)));
    QVERIFY(loadQmlFile("qsparqlresultlist", contextProperties));

    QSparqlQueryModel *model =
        getObject<QSparqlQueryModel *>("queryModel");

    QHash<int, QByteArray> roleNames = model->roleNames();
    // wait for result
    QTest::qWait(500);
    // role names start from Qt::UserRole + 1,
    QCOMPARE(roleNames[Qt::UserRole+1], QByteArray("u"));
    QCOMPARE(roleNames[Qt::UserRole+2], QByteArray("ng"));
    QCOMPARE(roleNames[Qt::UserRole+3], QByteArray("nf"));

}

void tst_QSparqlQMLBindings::sparql_legacy_test()
{
    // This test simple checks to see if the old SparqlResultList binding still
    // works, along with its options
    QList<QPair<QString, QVariant> > contextProperties;
    // If theres a problem with finding the binding, or in setting a property (driverName())
    // this will return false
    QVERIFY(loadQmlFile("qsparqllegacy", contextProperties));
}

QTEST_MAIN(tst_QSparqlQMLBindings)
#include "tst_qsparql_qmlbindings.moc"
