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

#include "qsparql_tracker_direct_select_result_p.h"
#include "qsparql_tracker_direct_p.h"
#include "qsparql_tracker_direct_driver_p.h"
#include "atomic_int_operations_p.h"

#include <qsparqlerror.h>
#include <qsparqlbinding.h>
#include <qsparqlquery.h>
#include <qsparqlresultrow.h>
#define XSD_INTEGER
#include "../../kernel/qsparqlxsd_p.h"

#include <QtCore/qvector.h>
#include <QtCore/qvariant.h>
#include <QtCore/qdebug.h>

using namespace AtomicIntOperations;

QT_BEGIN_NAMESPACE

////////////////////////////////////////////////////////////////////////////

QTrackerDirectSelectResult::QTrackerDirectSelectResult(QTrackerDirectDriverPrivate* p,
                                           const QString& query,
                                           QSparqlQuery::StatementType type,
                                           const QSparqlQueryOptions& options)
  : QTrackerDirectResult(options), cursor(0), resultMutex(QMutex::Recursive)
{
    setQuery(query);
    setStatementType(type);
    driverPrivate = p;
    queryRunner = new QTrackerDirectQueryRunner(this);
}

QTrackerDirectSelectResult::~QTrackerDirectSelectResult()
{
    stopAndWait();
    delete queryRunner;
}

void QTrackerDirectSelectResult::exec()
{
    if (!driverPrivate->driver->isOpen()) {
        setLastError(QSparqlError(driverPrivate->error,
                                  QSparqlError::ConnectionError));
        terminate();
        return;
    }

    // Queue calling exec() on the result. This way the finished() and
    // dataReady() signals won't be emitted before the user connects to
    // them, and the result won't be in the "finished" state before the
    // thread that calls this function has entered its event loop.
    QMetaObject::invokeMethod(this, "startFetcher",  Qt::QueuedConnection);
}

void QTrackerDirectSelectResult::startFetcher()
{
    QMutexLocker resultLocker(&resultMutex);
    if (queryRunner && !queryRunner->started && !isFinished()) {
        queryRunner->started = true;
        //first attempt to acquire the semaphore, if we can, then add the
        //fetcher to the threadPool queue, if we can't then waitForFinished
        //has it, so we don't need to refetch the results using this thread
        queryRunner->queue(driverPrivate->threadPool);
    }
}

void QTrackerDirectSelectResult::run()
{
    if(runQuery()) {
        if (isTable()) {
            while (!isFinished() && fetchNextResult()) {
                ;
            }
        } else if (isBool()) {
            fetchBoolResult();
        } else {
            terminate();
        }
     }
}

bool QTrackerDirectSelectResult::runQuery()
{
    if (isFinished())
        return false;

    QMutexLocker connectionLocker(&(driverPrivate->connectionMutex));

    GError * error = 0;
    cursor = tracker_sparql_connection_query(    driverPrivate->connection,
                                                    query().toUtf8().constData(),
                                                    0,
                                                    &error );
    if (error || !cursor) {
        QMutexLocker resultLocker(&resultMutex);
        setLastError(QSparqlError(QString::fromUtf8(error ? error->message : "unknown error"),
                        error ? errorCodeToType(error->code) : QSparqlError::StatementError,
                        error ? error->code : -1));
        if (error)
            g_error_free(error);
        terminate();
        qWarning() << "QTrackerDirectSelectResult:" << lastError() << query();
        return false;
    }

    return true;
}

bool QTrackerDirectSelectResult::fetchNextResult()
{
    QMutexLocker connectionLocker(&(driverPrivate->connectionMutex));
    GError * error = 0;
    gboolean active = tracker_sparql_cursor_next(cursor, 0, &error);

    if (error) {
        setLastError(QSparqlError(QString::fromUtf8(error->message),
                       errorCodeToType(error->code),
                       error->code));
        g_error_free(error);
        terminate();
        qWarning() << "QTrackerDirectSelectResult:" << lastError() << query();
        return false;
    }

    if (!active) {
        terminate();
        return false;
    }

    QMutexLocker resultLocker(&resultMutex);

    const gint n_columns = tracker_sparql_cursor_get_n_columns(cursor);

    if (columnNames.empty()) {
        for (int i = 0; i < n_columns; i++) {
            columnNames.append(QString::fromUtf8(tracker_sparql_cursor_get_variable_name(cursor, i)));
        }
    }

    QVector<QVariant> resultRow;
    resultRow.reserve(n_columns);
    for (int i = 0; i < n_columns; i++) {
        resultRow.append(readVariant(cursor, i));
    }

    results.append(resultRow);
    if (results.count() % driverPrivate->dataReadyInterval == 0) {
        emitDataReady(results.count());
    }

    return true;
}

bool QTrackerDirectSelectResult::fetchBoolResult()
{
    QMutexLocker resultLocker(&(resultMutex));

    if (!fetchNextResult())
        return false;

    if (results.count() == 1 && results[0].count() == 1) {
        const QVariant result = results[0][0];
        if (result.canConvert<bool>()) {
            setBoolValue(result.toBool());
        }
    }

    terminate();
    return true;
}

QSparqlBinding QTrackerDirectSelectResult::binding(int field) const
{
    QMutexLocker resultLocker(&resultMutex);

    if (!isValid()) {
        return QSparqlBinding();
    }

    if (field >= results[pos()].count() || field < 0) {
        qWarning() << "QTrackerDirectSelectResult::data[" << pos() << "]: column" << field << "out of range";
        return QSparqlBinding();
    }
    // A special case: we store TRACKER_SPARQL_VALUE_TYPE_INTEGER as longlong,
    // but its data type uri should be xsd:integer. Set it manually here.
    QSparqlBinding b;
    const QVariant& value = results[pos()][field];
    if (value.type() == QVariant::LongLong) {
        b.setValue(value.toString(), *XSD::Integer());
    }
    else {
        b.setValue(value);
    }

    if (field < columnNames.count()) {
        b.setName(columnNames[field]);
    }
    return b;
}

QVariant QTrackerDirectSelectResult::value(int field) const
{
    QMutexLocker resultLocker(&resultMutex);

    if (!isValid()) {
        return QVariant();
    }

    if (field >= results[pos()].count() || field < 0) {
        qWarning() << "QTrackerDirectSelectResult::data[" << pos() << "]: column" << field << "out of range";
        return QVariant();
    }

    return results[pos()].value(field);
}

void QTrackerDirectSelectResult::waitForFinished()
{
    if (isFinished())
        return;

    // We first need the connection to be ready before doing anything
    driverPrivate->waitForConnectionOpen();

    if (!driverPrivate->driver->isOpen()) {
        setLastError(QSparqlError(driverPrivate->error,
                                  QSparqlError::ConnectionError));
        terminate();
        return;
    }

    //if we can acquire the semaphore then run fetcher directly
    //if we can't then fetcher is in the threadpool, so we wait
    //for it to complete
    queryRunner->runOrWait();
}

void QTrackerDirectSelectResult::terminate()
{

    QMutexLocker resultLocker(&resultMutex);

    if (results.count() % driverPrivate->dataReadyInterval != 0) {
        emitDataReady(results.count());
    }

    if (getValue(resultFinished) == 0) {
        setValue(resultFinished, 1);
        Q_EMIT finished();
    }
    if (cursor) {
        g_object_unref(cursor);
        cursor = 0;
    }
}

void QTrackerDirectSelectResult::stopAndWait()
{
    if (queryRunner)
    {
        setValue(resultFinished, 1);
        queryRunner->wait();
    }

    if (cursor) {
        g_object_unref(cursor);
        cursor = 0;
    }

    delete queryRunner; queryRunner = 0;
}

int QTrackerDirectSelectResult::size() const
{
    QMutexLocker resultLocker(&resultMutex);
    return results.size();
}

QSparqlResultRow QTrackerDirectSelectResult::current() const
{

    QMutexLocker resultLocker(&resultMutex);

    if (!isValid()) {
        return QSparqlResultRow();
    }

    if (columnNames.size() != results[pos()].size())
        return QSparqlResultRow();

    QSparqlResultRow resultRow;
    for (int i = 0; i < results[pos()].size(); ++i) {
        QSparqlBinding b(columnNames[i], results[pos()][i]);
        resultRow.append(b);
    }
    return resultRow;
}

void QTrackerDirectSelectResult::emitDataReady(int totalCount)
{
    Q_EMIT dataReady(totalCount);
}

bool QTrackerDirectSelectResult::hasFeature(QSparqlResult::Feature feature) const
{
    switch (feature) {
        case QSparqlResult::Sync:
        case QSparqlResult::ForwardOnly:
            return false;
        case QSparqlResult::QuerySize:
            return true;
        default:
            return false;
    }
}

QT_END_NAMESPACE
