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
#include "qsparql_tracker_direct.h"
#include "qsparql_tracker_direct_driver_p.h"

#include <qsparqlerror.h>
#include <qsparqlbinding.h>
#include <qsparqlquery.h>
#include <qsparqlresultrow.h>
#define XSD_INTEGER
#include "../../kernel/qsparqlxsd_p.h"

#include <QtCore/qvariant.h>
#include <QtCore/qvector.h>
#include <QtCore/qpointer.h>
#include <QtCore/qthread.h>
#include <QtCore/qmutex.h>
#include <QtCore/qsemaphore.h>
#include <QtCore/qeventloop.h>

#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

////////////////////////////////////////////////////////////////////////////
// FIXME: refactor QTrackerDirectSelectResult to use QTrackerDirectSyncResult +
// sync->async wrapper.

class QTrackerDirectFetcherPrivate : public QTrackerDirectQueryRunner
{

 //   QTrackerDirectQueryRunner() : started(false), runFinished(0), runSemaphore(1) { setAutoDelete(false); };
public:
    QTrackerDirectFetcherPrivate(QTrackerDirectSelectResult *res) : result(res)  { started = false; runFinished = 0; runSemaphore.release(1); setAutoDelete(false); }//runFinished = 0; runSemaphore.release(1);  }

private:
    QTrackerDirectSelectResult *result;

    void run()
    {
        qWarning() << "Subclassed run!";
        //check to make sure we are not going to do this twice
        if(!runFinished)
        {
            if(result->runQuery()) {
                if (result->isTable()) {
                    while (!result->isFinished() && result->fetchNextResult()) {
                        ;
                    }
                } else if (result->isBool()) {
                    result->fetchBoolResult();
                } else {
                    result->terminate();
                }
            }
        }
        runFinished=1;
        runSemaphore.release(1);
    }

};

class QTrackerDirectSelectResultPrivate : public QObject {
    Q_OBJECT
public:
    QTrackerDirectSelectResultPrivate(QTrackerDirectSelectResult* result, QTrackerDirectDriverPrivate *dpp);

    ~QTrackerDirectSelectResultPrivate();
    void terminate();
    void dataReady(int totalCount);

    TrackerSparqlCursor* cursor;
    QVector<QString> columnNames;
    QList<QVector<QVariant> > results;
    QAtomicInt isFinished;

    QTrackerDirectSelectResult* q;
    QTrackerDirectDriverPrivate *driverPrivate;
    // This mutex is for ensuring that only one thread at a time is accessing
    // the member variables of this class (mainly: "results").
    // Note that only the fetcher thread accesses the 'cursor', and so that
    // is protected by the connectionMutex in the driver, not this mutex.
    QMutex resultMutex;
};

QTrackerDirectSelectResultPrivate::QTrackerDirectSelectResultPrivate(   QTrackerDirectSelectResult* result,
                                                            QTrackerDirectDriverPrivate *dpp)
                                                           // QTrackerDirectFetcherPrivate *f)
  : cursor(0),
  q(result), driverPrivate(dpp), resultMutex(QMutex::Recursive)
{
}

QTrackerDirectSelectResultPrivate::~QTrackerDirectSelectResultPrivate()
{
    if (cursor)
        g_object_unref(cursor);
}

void QTrackerDirectSelectResultPrivate::terminate()
{
    QMutexLocker resultLocker(&resultMutex);

    if (results.count() % driverPrivate->dataReadyInterval != 0) {
        dataReady(results.count());
    }

    isFinished = 1;
    q->Q_EMIT finished();
    if (cursor) {
        g_object_unref(cursor);
        cursor = 0;
    }
}

void QTrackerDirectResultPrivate::dataReady(int totalCount)
{
    Q_EMIT q->dataReady(totalCount);
}

////////////////////////////////////////////////////////////////////////////

QTrackerDirectSelectResult::QTrackerDirectSelectResult(QTrackerDirectDriverPrivate* p,
                                           const QString& query,
                                           QSparqlQuery::StatementType type)
{
    setQuery(query);
    setStatementType(type);
    queryRunner = new QTrackerDirectFetcherPrivate(this);
    d = new QTrackerDirectSelectResultPrivate(this, p);
}

QTrackerDirectSelectResult::~QTrackerDirectSelectResult()
{
    stopAndWait();
    delete d;
}

void QTrackerDirectSelectResult::exec()
{
    if (!d->driverPrivate->driver->isOpen()) {
        setLastError(QSparqlError(d->driverPrivate->error,
                                  QSparqlError::ConnectionError));
        d->terminate();
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
    QMutexLocker resultLocker(&(d->resultMutex));
    if (queryRunner && !queryRunner->started && !isFinished()) {
        queryRunner->started = true;
        //first attempt to acquire the semaphore, if we can, then add the
        //fetcher to the threadPool queue, if we can't then waitForFinished
        //has it, so we don't need to refetch the results using this thread
        queryRunner->queue(d->driverPrivate->threadPool);
    }
}

bool QTrackerDirectSelectResult::runQuery()
{
    if (isFinished())
        return false;

    QMutexLocker connectionLocker(&(d->driverPrivate->connectionMutex));

    GError * error = 0;
    d->cursor = tracker_sparql_connection_query(    d->driverPrivate->connection,
                                                    query().toUtf8().constData(),
                                                    0,
                                                    &error );
    if (error || !d->cursor) {
        QMutexLocker resultLocker(&(d->resultMutex));
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
    QMutexLocker connectionLocker(&(d->driverPrivate->connectionMutex));

    GError * error = 0;
    gboolean active = tracker_sparql_cursor_next(d->cursor, 0, &error);

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

    QMutexLocker resultLocker(&(d->resultMutex));

    const gint n_columns = tracker_sparql_cursor_get_n_columns(d->cursor);

    if (d->columnNames.empty()) {
        for (int i = 0; i < n_columns; i++) {
            d->columnNames.append(QString::fromUtf8(tracker_sparql_cursor_get_variable_name(d->cursor, i)));
        }
    }

    QVector<QVariant> resultRow;
    resultRow.reserve(n_columns);
    for (int i = 0; i < n_columns; i++) {
        resultRow.append(readVariant(d->cursor, i));
    }

    d->results.append(resultRow);
    if (d->results.count() % d->driverPrivate->dataReadyInterval == 0) {
        d->dataReady(d->results.count());
    }

    return true;
}

bool QTrackerDirectSelectResult::fetchBoolResult()
{
    QMutexLocker resultLocker(&(d->resultMutex));

    if (!fetchNextResult())
        return false;

    if (d->results.count() == 1 && d->results[0].count() == 1) {
        const QVariant result = d->results[0][0];
        if (result.canConvert<bool>()) {
            setBoolValue(result.toBool());
        }
    }

    terminate();
    return true;
}

QSparqlBinding QTrackerDirectSelectResult::binding(int field) const
{
    QMutexLocker resultLocker(&(d->resultMutex));

    if (!isValid()) {
        return QSparqlBinding();
    }

    if (field >= d->results[pos()].count() || field < 0) {
        qWarning() << "QTrackerDirectSelectResult::data[" << pos() << "]: column" << field << "out of range";
        return QSparqlBinding();
    }
    // A special case: we store TRACKER_SPARQL_VALUE_TYPE_INTEGER as longlong,
    // but its data type uri should be xsd:integer. Set it manually here.
    QSparqlBinding b;
    const QVariant& value = d->results[pos()][field];
    if (value.type() == QVariant::LongLong) {
        b.setValue(value.toString(), *XSD::Integer());
    }
    else {
        b.setValue(value);
    }

    if (field < d->columnNames.count()) {
        b.setName(d->columnNames[field]);
    }
    return b;
}

QVariant QTrackerDirectSelectResult::value(int field) const
{
    QMutexLocker resultLocker(&(d->resultMutex));

    if (!isValid()) {
        return QVariant();
    }

    if (field >= d->results[pos()].count() || field < 0) {
        qWarning() << "QTrackerDirectSelectResult::data[" << pos() << "]: column" << field << "out of range";
        return QVariant();
    }

    return d->results[pos()].value(field);
}

void QTrackerDirectSelectResult::waitForFinished()
{
    if (isFinished())
        return;

    // We first need the connection to be ready before doing anything
    d->driverPrivate->waitForConnectionOpen();

    if (!d->driverPrivate->driver->isOpen()) {
        setLastError(QSparqlError(d->driverPrivate->error,
                                  QSparqlError::ConnectionError));
        d->terminate();
        return;
    }

    //if we can acquire the semaphore then run fetcher directly
    //if we can't then fetcher is in the threadpool, so we wait
    //for it to complete
    queryRunner->runOrWait();
}

void QTrackerDirectSelectResult::terminate()
{
    d->terminate();
}

bool QTrackerDirectSelectResult::isFinished() const
{
    return d->isFinished == 1;
}

void QTrackerDirectSelectResult::stopAndWait()
{
    if (queryRunner)
    {
        d->isFinished = 1;
        queryRunner->wait();
    }
    delete queryRunner; queryRunner = 0;
}

int QTrackerDirectSelectResult::size() const
{
    QMutexLocker resultLocker(&(d->resultMutex));
    return d->results.size();
}

QSparqlResultRow QTrackerDirectSelectResult::current() const
{
    QMutexLocker resultLocker(&(d->resultMutex));

    if (!isValid()) {
        return QSparqlResultRow();
    }

    if (d->columnNames.size() != d->results[pos()].size())
        return QSparqlResultRow();

    QSparqlResultRow resultRow;
    for (int i = 0; i < d->results[pos()].size(); ++i) {
        QSparqlBinding b(d->columnNames[i], d->results[pos()][i]);
        resultRow.append(b);
    }
    return resultRow;
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

#include "qsparql_tracker_direct_select_result_p.moc"
