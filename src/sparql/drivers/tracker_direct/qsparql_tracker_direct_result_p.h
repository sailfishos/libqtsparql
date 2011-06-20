/****************************************************************************
**
** Copyright (C) 2010-2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (ivan.frade@nokia.com)
**
** This file is part of the QtSparql module (not yet part of the Qt Toolkit).
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

#ifndef QSPARQL_TRACKER_DIRECT_RESULT_P_H
#define QSPARQL_TRACKER_DIRECT_RESULT_P_H

#include <QtSparql/qsparqlresult.h>
#include <QtSparql/qsparqlquery.h>
#include <QtSparql/qsparqlerror.h>

#include <QRunnable>
#include <QThreadPool>
#include <QtCore/qsemaphore.h>
#include <QtCore/qdebug.h>

#include <tracker-sparql.h>

#include "qsparql_tracker_direct_driver_p.h"
#include "qsparql_tracker_direct.h"

#ifdef QT_PLUGIN
#define Q_EXPORT_SPARQLDRIVER_TRACKER_DIRECT
#else
#define Q_EXPORT_SPARQLDRIVER_TRACKER_DIRECT Q_SPARQL_EXPORT
#endif

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QTrackerDirectDriverPrivate;
class QTrackerDirectQueryRunner;
class QTrackerDirectDriverPrivate;
class Q_EXPORT_SPARQLDRIVER_TRACKER_DIRECT QTrackerDirectResult : public QSparqlResult
{
    Q_OBJECT
    friend class QTrackerDirectQueryRunner;
public:
    QTrackerDirectResult();
    ~QTrackerDirectResult();

    virtual bool isFinished() const;

private:
    // Will be called by the query runner to execute the query, results that don't
    // need a thread to run in (sync) do not need to implement this
    virtual void run() {}
    virtual void stopAndWait() = 0;

protected:
    QTrackerDirectDriverPrivate *driverPrivate;
    QAtomicInt resultFinished;
    QTrackerDirectQueryRunner *queryRunner;

public Q_SLOTS:
    virtual void exec() = 0;
    void driverClosing();

};

class QTrackerDirectQueryRunner : public QRunnable
{
public:
    QTrackerDirectResult *result;
    QAtomicInt runFinished;
    QSemaphore runSemaphore;
    bool started;

    QTrackerDirectQueryRunner(QTrackerDirectResult *result) : result(result), runFinished(0), runSemaphore(1), started(false)
    {
        setAutoDelete(false);
    }

    void runOrWait()
    {
        if(acquireRunSemaphore()) {
            if (!runFinished)
                run();
            else
                runSemaphore.release(1);
        } else {
            wait();
        }
    }

    void queue(QThreadPool& threadPool)
    {
        if(acquireRunSemaphore())
            threadPool.start(this);
    }

    void wait()
    {
        //if something has has acquired the semaphore (eg the fetcher thread)
        //this will block until it is released in run
        runSemaphore.acquire(1);
        runSemaphore.release(1);
    }

private:
    void run()
    {
        if (!runFinished) {
            result->run();
        }
        runFinished=1;
        runSemaphore.release(1);
    }

    bool acquireRunSemaphore()
    {
        return runSemaphore.tryAcquire(1);
    }
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSPARQL_TRACKER_DIRECT_RESULT_P_H
