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

#include "qsparql_tracker_direct_result_p.h"
#include <QtSparql/qsparqlerror.h>
#include <QtCore/qdebug.h>

// Query Runner Implementation
QTrackerDirectQueryRunner::QTrackerDirectQueryRunner(QTrackerDirectResult *result)
  : result(result), runFinished(0), runSemaphore(1), started(false)
{
    setAutoDelete(false);
}

void QTrackerDirectQueryRunner::runOrWait()
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

void QTrackerDirectQueryRunner::queue(QThreadPool& threadPool)
{
    if(acquireRunSemaphore())
        threadPool.start(this);
}

void QTrackerDirectQueryRunner::wait()
{
    //if something has has acquired the semaphore (eg the fetcher thread)
    //this will block until it is released in run
    runSemaphore.acquire(1);
    runSemaphore.release(1);
}

void QTrackerDirectQueryRunner::run()
{
    if (!runFinished) {
        result->run();
    }
    runFinished=1;
    runSemaphore.release(1);
}

bool QTrackerDirectQueryRunner::acquireRunSemaphore()
{
    return runSemaphore.tryAcquire(1);
}

////////////////////////////////////////////////////////////////////////////

QTrackerDirectResult::QTrackerDirectResult()
  : queryRunner(0), resultFinished(0)
{
}

QTrackerDirectResult::~QTrackerDirectResult()
{
}

void QTrackerDirectResult::driverClosing()
{
    qWarning() << "QSparqlConnection closed before QSparqlResult with query:" <<
                    query();
    if (!isFinished())
    {
        setLastError(QSparqlError(QString::fromUtf8("QSparqlConnection closed before QSparqlResult"),
                        QSparqlError::ConnectionError,
                        -1));
    }
    stopAndWait();
}

bool QTrackerDirectResult::isFinished() const
{
    return resultFinished == 1;
}

