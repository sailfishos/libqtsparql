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

#include "updatetester.h"
#include <QtTest/QtTest>
#include <QtSparql>

UpdateTester::UpdateTester(int id)
    : connection(0), ownConnection(0)
    , updateFinishedMapper(0)
    , deleteFinishedMapper(0)
    , id(id), isFinished(false)
    , validateUpdateResultAttempts(0)
    , validateDeleteResultAttempts(0)
{
}

void UpdateTester::setParameters(int numInserts, int numDeletes)
{
    QVERIFY(numDeletes <= numInserts);
    this->numInserts = numInserts;
    this->numDeletes = numDeletes;
}

void UpdateTester::setConnection(QSparqlConnection *connection)
{
    this->connection = connection;
    delete ownConnection;
    ownConnection = 0;
}

void UpdateTester::waitForFinished()
{
    while (!isFinished)
    {
        QTest::qWait(100);
    }
}

void UpdateTester::startInThread(QThread* thread)
{
    this->moveToThread(thread);
    connect(thread, SIGNAL(started()), this, SLOT(runInThread()));
}

void UpdateTester::start()
{
    connect(this, SIGNAL(updatesComplete()), this, SLOT(startValidateUpdate()));
    connect(this, SIGNAL(validateUpdateComplete()), this, SLOT(startDeletions()));
    connect(this, SIGNAL(deletionsComplete()), this, SLOT(startValidateDeletion()));
    connect(this, SIGNAL(validateDeletionComplete()), this, SLOT(finish()));
    initResources();
    startUpdates();
}

void UpdateTester::run()
{
    start();
    waitForFinished();
    cleanup();
}

void UpdateTester::runInThread()
{
    run();
    this->thread()->quit();
}

void UpdateTester::cleanup()
{
    if (connection && numInserts-numDeletes > 0) {
        QString deleteTemplate = "DELETE { ?u a nco:PersonContact }"
                              " WHERE { ?u a nco:PersonContact; nie:isLogicalPartOf <qsparql-tracker-direct-concurrency-thread%1> . }";
        QSparqlQuery deleteQuery(deleteTemplate.arg(id), QSparqlQuery::DeleteStatement);
        QSparqlResult* result = connection->syncExec(deleteQuery);
        delete result;
    }

    qDeleteAll(resultList);
    resultList.clear();

    if (ownConnection) {
        delete ownConnection;
        ownConnection = connection = 0;
    }
}

void UpdateTester::initResources()
{
    if (!connection) {
        ownConnection = new QSparqlConnection("QTRACKER_DIRECT");
        connection = ownConnection;
    }
    if (!updateFinishedMapper) {
        updateFinishedMapper = new QSignalMapper(this);
        connect(updateFinishedMapper, SIGNAL(mapped(QObject*)), this, SLOT(onUpdateFinished(QObject*)));
    }
    if (!deleteFinishedMapper) {
        deleteFinishedMapper = new QSignalMapper(this);
        connect(deleteFinishedMapper, SIGNAL(mapped(QObject*)), this, SLOT(onDeleteFinished(QObject*)));
    }
}

void UpdateTester::startUpdates()
{
    validateUpdateResultAttempts = 0;
    const QString insertTemplate = "insert { <addeduri00%1-%2> a nco:PersonContact; nie:isLogicalPartOf <qsparql-tracker-direct-concurrency-thread%2>;"
                                    "nie:isLogicalPartOf <qsparql-tracker-direct-tests-concurrency-stress>;"
                                    "nco:nameGiven \"addedname00%1\"; nco:nameFamily \"addedFamily00%1\" . }";
    for (int i=0;i<numInserts;i++) {
        QSparqlQuery insertQuery(insertTemplate.arg(i).arg(id), QSparqlQuery::InsertStatement);
        QSparqlResult *result = connection->exec(insertQuery);
        QVERIFY( result );
        QVERIFY( !result->hasError() );
        appendPendingResult(result, updateFinishedMapper);
    }
}

void UpdateTester::startValidateUpdate()
{
    QSparqlResult* result = connection->exec(QSparqlQuery(selectTemplate().arg(id)));
    connect(result, SIGNAL(finished()), this, SLOT(validateUpdateResult()));
    QVERIFY( result );
    QVERIFY( !result->hasError() );
    resultList.append(result);
}

void UpdateTester::validateUpdateResult()
{
    bool retry = false;
    doValidateUpdateResult(&retry);
    if (retry) {
        QMetaObject::invokeMethod(this, "startValidateUpdate", Qt::QueuedConnection);
    }
    else {
        Q_EMIT validateUpdateComplete();
    }
}

void UpdateTester::startDeletions()
{
    validateDeleteResultAttempts = 0;
    const QString deleteTemplate = "delete { <addeduri00%1-%2> a nco:PersonContact }"
                                   " WHERE { <addeduri00%1-%2> a nco:PersonContact; nie:isLogicalPartOf <qsparql-tracker-direct-concurrency-thread%2> . }";
    for (int i=0;i<numDeletes;i++) {
        QSparqlQuery deleteQuery(deleteTemplate.arg(i).arg(id), QSparqlQuery::DeleteStatement);
        QSparqlResult* result = connection->exec(deleteQuery);
        QVERIFY( result );
        QVERIFY( !result->hasError() );
        appendPendingResult(result, deleteFinishedMapper);
    }
}

void UpdateTester::startValidateDeletion()
{
    QSparqlResult* result = connection->exec(QSparqlQuery(selectTemplate().arg(id)));
    QVERIFY( result );
    QVERIFY( !result->hasError() );
    connect(result, SIGNAL(finished()), this, SLOT(validateDeleteResult()));
    resultList.append(result);
}

void UpdateTester::validateDeleteResult()
{
    bool retry = false;
    doValidateDeleteResult(&retry);
    if (retry) {
        QMetaObject::invokeMethod(this, "startValidateDeletion", Qt::QueuedConnection);
    }
    else {
        Q_EMIT validateDeletionComplete();
    }
}

void UpdateTester::onUpdateFinished(QObject* mappedResult)
{
    checkIsPendingResult(mappedResult);
    if (removePendingResultWasLast(mappedResult))
        Q_EMIT updatesComplete();
}

void UpdateTester::onDeleteFinished(QObject* mappedResult)
{
    checkIsPendingResult(mappedResult);
    if (removePendingResultWasLast(mappedResult))
        Q_EMIT deletionsComplete();
}

void UpdateTester::finish()
{
    isFinished = true;
    if (validateUpdateResultAttempts > 1) {
        qDebug() << "Update result validation attempts" << validateUpdateResultAttempts;
    }
    if (validateDeleteResultAttempts > 1) {
        qDebug() << "Delete result validation attempts" << validateDeleteResultAttempts;
    }

    Q_EMIT finished();
}

QString UpdateTester::selectTemplate()
{
    return QString("select ?u ?ng ?nf { ?u a nco:PersonContact; nie:isLogicalPartOf <qsparql-tracker-direct-concurrency-thread%1>;"
                        "nco:nameGiven ?ng; nco:nameFamily ?nf . }");
}

void UpdateTester::appendPendingResult(QSparqlResult* result, QSignalMapper* signalMapper)
{
    QVERIFY( !pendingResults.contains(result) );
    resultList.append(result);
    pendingResults.insert(result);
    signalMapper->setMapping(result,result);
    connect(result, SIGNAL(finished()), signalMapper, SLOT(map()));
}

void UpdateTester::checkIsPendingResult(QObject* mappedResult) const
{
    QSparqlResult* result = qobject_cast<QSparqlResult*>(mappedResult);
    QVERIFY( result );
    QVERIFY( pendingResults.contains(result) );
}

bool UpdateTester::removePendingResultWasLast(QObject* mappedResult)
{
    QSparqlResult* result = qobject_cast<QSparqlResult*>(mappedResult);
    pendingResults.remove(result);
    return pendingResults.isEmpty();
}

void UpdateTester::doValidateUpdateResult(bool* retry)
{
    QVERIFY( ++validateUpdateResultAttempts < 10 );

    QSparqlResult* result = resultList.back();
    QVERIFY( !result->hasError() );

    // Verify that we get most numInserts results
    // The result size may temporarily be smaller with tracker direct access
    // due to its transaction management
    QVERIFY( result->size() <= numInserts );

    QHash<QString, QString> contactNameValues;
    while (result->next()) {
        contactNameValues[result->value(0).toString()] = result->value(1).toString();
    }
    // Verify that all resource uris were unique
    QVERIFY( contactNameValues.count() == result->size() );

    // Verify that all succesfully inserted data was present in the result
    int missingInsertsCount = 0;
    for(int i=0; i<numInserts; i++) {
        const QString resourceUri = QString("addeduri00%1-%2").arg(i).arg(id);
        if (contactNameValues.contains(resourceUri)) {
            QCOMPARE(contactNameValues[resourceUri], QString("addedname00%1").arg(i));
        }
        else {
            ++missingInsertsCount;
        }
    }
    QCOMPARE( numInserts - result->size(), missingInsertsCount );

    *retry = (missingInsertsCount > 0);
}

void UpdateTester::doValidateDeleteResult(bool* retry)
{
    QVERIFY( ++validateDeleteResultAttempts < 10 );

    QSparqlResult* result = resultList.back();
    QVERIFY( !result->hasError() );
    if (result->size() == numInserts-numDeletes) {
        QHash<QString, QString> contactNameValues;
        while (result->next()) {
            contactNameValues[result->value(0).toString()] = result->value(1).toString();
        }
        QCOMPARE(contactNameValues.size(), numInserts-numDeletes);
        // number of deletes might be less than the number of inserts, we delete from 0 -> numDeletes-1, so
        int startFrom = numInserts - numDeletes;
        if (startFrom != 0) {
            for (int i=startFrom;i<numInserts;i++) {
                QCOMPARE(contactNameValues[QString("addeduri00%1-%2").arg(i).arg(id)], QString("addedname00%1").arg(i));
            }
        }
    }
    else {
        *retry = true;
    }
}
