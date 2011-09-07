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

#include "querytester.h"
#include <QtTest/QtTest>
#include <QtSparql/QtSparql>
#include "resultchecker.h"

namespace {

QPair<int,int> randomRangeIn(int max)
{
    const int low = qrand() % (max-1) + 1;
    const int high = qrand() % ((max+1) - low) + low;
    return qMakePair(low, high);
}

void seedRandomRangeGenerator()
{
    static bool seeded = false;
    if (!seeded) {
        QTime time = QTime::currentTime();
        qsrand((uint)time.msec());
        seeded = true;
    }
}

}  // namespace

QueryTester::QueryTester()
    : connection(0), ownConnection(0), resultChecker(0)
{
    seedRandomRangeGenerator();
}

QueryTester::~QueryTester()
{
    cleanup();
}

void QueryTester::cleanup()
{
    delete resultChecker;
    resultChecker = 0;
    delete ownConnection;
    ownConnection = 0;
}

void QueryTester::setParameters(int numQueries, int testDataSize, bool forwardOnly)
{
    this->numQueries = numQueries;
    this->testDataSize = testDataSize;
    this->forwardOnly = forwardOnly;
}

void QueryTester::setConnection(QSparqlConnection* connection)
{
    this->connection = connection;
    delete ownConnection;
    ownConnection = 0;
}

bool QueryTester::waitForAllFinished(int silenceTimeoutMs)
{
    return resultChecker->waitForAllFinished(silenceTimeoutMs);
}

void QueryTester::startQueries()
{
    initResources();
    for (int i=0;i<numQueries;i++) {
        QPair<int, int> resultRange;
        QString filter;
        if (i % 10 == 0) {
            // Override every 10th result to read all
            resultRange = qMakePair(1, testDataSize);
            filter = ". }";
        }
        else {
            resultRange  = randomRangeIn(testDataSize);
            filter = QString("FILTER ( ?t >=%1 && ?t <=%2 ) }").arg(resultRange.first).arg(resultRange.second);
        }
        QSparqlQuery select(QString("select ?u ?t {?u a nmm:MusicPiece; "
                                    "nmm:trackNumber ?t; "
                                    "nie:isLogicalPartOf <qsparql-tracker-direct-tests-concurrency-stress> "
                                    + filter));
        QSparqlQueryOptions queryOptions;
        queryOptions.setForwardOnly(forwardOnly);
        QSparqlResult *result = connection->exec(select, queryOptions);
        resultChecker->append(result, resultRange);
    }
}

void QueryTester::startInThread(QThread* thread)
{
    this->moveToThread(thread);
    connect(thread, SIGNAL(started()), this, SLOT(runInThread()));
}

void QueryTester::runInThread()
{
    initResources();
    connect(resultChecker, SIGNAL(allFinished()), this, SLOT(quit()));
    startQueries();
}

void QueryTester::quit()
{
    cleanup();
    this->thread()->quit();
}

void QueryTester::initResources()
{
    if (!connection) {
        const int dataReadyInterval = qMax(testDataSize/100, 10);
        QSparqlConnectionOptions options;
        options.setDataReadyInterval(dataReadyInterval);
        ownConnection = new QSparqlConnection("QTRACKER_DIRECT", options);
        connection = ownConnection;
    }
    if (!resultChecker) {
        resultChecker = new ResultChecker;
    }
}
