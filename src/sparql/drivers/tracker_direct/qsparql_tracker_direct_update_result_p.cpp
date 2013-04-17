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

#include "qsparql_tracker_direct_update_result_p.h"
#include "qsparql_tracker_direct_p.h"
#include "qsparql_tracker_direct_driver_p.h"
#include "atomic_int_operations_p.h"

#include <qsparqlbinding.h>
#include <qsparqlquery.h>
#include <qsparqlresultrow.h>

#include <QtCore/qvariant.h>
#include <QtCore/qdebug.h>

using namespace AtomicIntOperations;

QT_BEGIN_NAMESPACE

QTrackerDirectUpdateResult::QTrackerDirectUpdateResult(QTrackerDirectDriverPrivate* p,
                                           const QString& query,
                                           QSparqlQuery::StatementType type,
                                           const QSparqlQueryOptions& options)
  : QTrackerDirectResult(options)
{
    setQuery(query);
    setStatementType(type);
    driverPrivate = p;
    queryRunner = new QTrackerDirectQueryRunner(this);
}

QTrackerDirectUpdateResult::~QTrackerDirectUpdateResult()
{
    stopAndWait();
    delete queryRunner;
}

void QTrackerDirectUpdateResult::exec()
{
    if (!driverPrivate)
        return;

    if (!driverPrivate->driver->isOpen()) {
        setLastError(QSparqlError(driverPrivate->error,
                                  QSparqlError::ConnectionError));
        terminate();
        return;
    }
    queryRunner->queue(driverPrivate->threadPool);
}

void QTrackerDirectUpdateResult::run()
{
    if (driverPrivate) {
        GError * error = 0;
        tracker_sparql_connection_update(driverPrivate->connection,
                                         query().toUtf8().constData(),
                                         qSparqlPriorityToGlib(options.priority()),
                                         0,
                                         &error);

        if (error) {
            setLastError(QSparqlError(QString::fromUtf8(error->message),
                             errorCodeToType(error->code),
                             error->code));
            g_error_free(error);
            qWarning() << "QTrackerDirectUpdateResult:" << lastError() << query();
        }
        QMetaObject::invokeMethod(this, "terminate", Qt::QueuedConnection);
    }

}

QSparqlBinding QTrackerDirectUpdateResult::binding(int /*field*/) const
{
    return QSparqlBinding();
}

QVariant QTrackerDirectUpdateResult::value(int /*field*/) const
{
    return QVariant();
}

void QTrackerDirectUpdateResult::waitForFinished()
{
    if (isFinished())
        return;

    // We first need the connection to be ready before doing anything
    if (driverPrivate) {
        driverPrivate->waitForConnectionOpen();

        if (!driverPrivate->driver->isOpen()) {
            setLastError(QSparqlError(driverPrivate->error,
                                      QSparqlError::ConnectionError));
            terminate();
            return;
        }
        queryRunner->runOrWait();
        terminate();
    }
}

void QTrackerDirectUpdateResult::terminate()
{
    if (getValue(resultFinished) == 0) {
        setValue(resultFinished, 1);
        Q_EMIT finished();
    }
}

int QTrackerDirectUpdateResult::size() const
{
    return 0;
}

QSparqlResultRow QTrackerDirectUpdateResult::current() const
{
    return QSparqlResultRow();
}

void QTrackerDirectUpdateResult::stopAndWait()
{
    if (queryRunner) {
        queryRunner->wait();
    }
    driverPrivate = 0;
    setValue(resultFinished, 1);
    delete queryRunner; queryRunner = 0;
}

QT_END_NAMESPACE

