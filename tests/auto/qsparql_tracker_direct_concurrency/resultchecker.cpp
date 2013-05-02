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

#include "resultchecker.h"
#include <QtTest/QtTest>
#include <QtSparql>

ResultChecker::ResultChecker()
    : dataReadyMapper(0), finishedMapper(0)
{
}

ResultChecker::~ResultChecker()
{
    pendingResults.clear();
    qDeleteAll(allResults);
}

void ResultChecker::append(QSparqlResult *r, const QPair<int, int>& range)
{
    QVERIFY( r );
    QVERIFY( !r->hasError() );
    initResources();
    dataReadyMapper->setMapping(r, r);
    connect(r, SIGNAL(dataReady(int)), dataReadyMapper, SLOT(map()));
    finishedMapper->setMapping(r, r);
    connect(r, SIGNAL(finished()), finishedMapper, SLOT(map()));
    allResults.append(r);
    pendingResults.insert(r, range);
}

bool ResultChecker::waitForAllFinished(int silenceTimeoutMs)
{
    QTime timeoutTimer;
    timeoutTimer.start();
    bool timeout = false;
    while (!pendingResults.empty() && !timeout) {
        const int pendingResultsCountBefore = pendingResults.count();
        QTest::qWait(silenceTimeoutMs / 10);
        if (pendingResults.count() < pendingResultsCountBefore) {
            timeoutTimer.restart();
        }
        else if (timeoutTimer.elapsed() > silenceTimeoutMs) {
            timeout = true;
        }
    }
    return !timeout;
}

void ResultChecker::onDataReady(QObject* mappedResult)
{
    QSparqlResult *result = qobject_cast<QSparqlResult*>(mappedResult);
    while (result->next()) {
        // just do something pointless with the result
        result->value(1).toInt();
        result->value(0).toString();
    }
}

void ResultChecker::onFinished(QObject* mappedResult)
{
    QSparqlResult *result = qobject_cast<QSparqlResult*>(mappedResult);
    QVERIFY( result );
    QVERIFY( !result->hasError() );
    QVERIFY( pendingResults.contains(result) );

    const QPair<int, int> resultRange = pendingResults.value(result);
    const int expectedResultSize = (resultRange.second - resultRange.first) + 1;
    int resultSize = result->size();
    if (result->hasFeature(QSparqlResult::ForwardOnly)) {
        resultSize = 0;
        while (result->next()) {
            resultSize++;
            // Also verify the results here
            QVERIFY(result->value(1).toInt() >= resultRange.first);
            QVERIFY(result->value(1).toInt() <= resultRange.second);
        }
    }
    QCOMPARE(resultSize, expectedResultSize);

    // the results should have been fully nexted in the data ready function
    QCOMPARE(result->pos(), int(QSparql::AfterLastRow));

    if (!result->hasFeature(QSparqlResult::ForwardOnly)) {
        // go back through the results and validate that they are in range
        int resultCount = 0;
        while (result->previous()) {
            //we don't know the order, so just ensure the result is within range
            QVERIFY(result->value(1).toInt() >= resultRange.first);
            QVERIFY(result->value(1).toInt() <= resultRange.second);
            resultCount++;
        }
        // now make sure the results counted match the size
        QCOMPARE(resultCount, expectedResultSize);
    }

    pendingResults.remove(result);
    if (pendingResults.empty())
        Q_EMIT allFinished();
}

void ResultChecker::initResources()
{
    if (!dataReadyMapper) {
        dataReadyMapper = new QSignalMapper(this);
        connect(dataReadyMapper, SIGNAL(mapped(QObject*)), this, SLOT(onDataReady(QObject*)));
    }
    if (!finishedMapper) {
        finishedMapper = new QSignalMapper(this);
        connect(finishedMapper, SIGNAL(mapped(QObject*)), this, SLOT(onFinished(QObject*)));
    }
}
