/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (ivan.frade@nokia.com)
**
** This file is part of the QtSparql module (not yet part of the Qt Toolkit).
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you have questions regarding the use of this file, please contact
** Nokia at ivan.frade@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include <tracker-sparql.h>

#include "qsparql_tracker_direct_p.h"

#include <qsparqlerror.h>
#include <qsparqlbinding.h>
#include <qsparqlquery.h>
#include <qsparqlresultrow.h>
#include <qsparqlconnection.h>

#include <qcoreapplication.h>
#include <qvariant.h>
#include <qstringlist.h>
#include <qvector.h>

#include <qdebug.h>

QT_BEGIN_NAMESPACE

namespace XSD {
Q_GLOBAL_STATIC_WITH_ARGS(QUrl, Boolean,
                          (QLatin1String("http://www.w3.org/2001/XMLSchema#boolean")))

Q_GLOBAL_STATIC_WITH_ARGS(QUrl, DateTime,
                          (QLatin1String("http://www.w3.org/2001/XMLSchema#dateTime")))

Q_GLOBAL_STATIC_WITH_ARGS(QUrl, Double,
                          (QLatin1String("http://www.w3.org/2001/XMLSchema#double")))

Q_GLOBAL_STATIC_WITH_ARGS(QUrl, Integer,
                          (QLatin1String("http://www.w3.org/2001/XMLSchema#integer")))
}

class QTrackerDirectFetcherPrivate : public QThread
{
public:
    QTrackerDirectFetcherPrivate(QTrackerDirectResult *res) : result(res) { }

    void run()
    {
        setTerminationEnabled(false);
        if (result->exec()) {
            if (result->isTable()) {
                while (result->fetchNextResult()) {
                    setTerminationEnabled(true);
                    setTerminationEnabled(false);
                }
            } else if (result->isBool()) {
                result->fetchBoolResult();
            } else {
                result->terminate();
            }
        }
        setTerminationEnabled(true);
    }

private:
    QTrackerDirectResult *result;
};

class QTrackerDirectDriverPrivate {
public:
    QTrackerDirectDriverPrivate();
    ~QTrackerDirectDriverPrivate();

    TrackerSparqlConnection *connection;
    int dataReadyInterval;
    // This mutex is for ensuring that only one thread at a time
    // is using the connection to make tracker queries. This mutex
    // probably isn't needed as a TrackerSparqlConnection is
    // already thread safe.
    QMutex mutex;
};

class QTrackerDirectResultPrivate : public QObject {
    Q_OBJECT
public:
    QTrackerDirectResultPrivate(QTrackerDirectResult* result, QTrackerDirectDriverPrivate *dpp, QTrackerDirectFetcherPrivate *f);

    ~QTrackerDirectResultPrivate();
    void terminate();
    void setLastError(const QSparqlError& e);
    void setBoolValue(bool v);
    void dataReady(int totalCount);

    TrackerSparqlCursor * cursor;
    QVector<QString> columnNames;
    QVector<QSparqlResultRow> results;
    bool isFinished;
    bool resultAlive; // whether the corresponding Result object is still alive
    QEventLoop *loop;

    QTrackerDirectResult* q;
    QTrackerDirectDriverPrivate *driverPrivate;
    QTrackerDirectFetcherPrivate *fetcher;
    // This mutex is for ensuring that only one thread at a time
    // is accessing the results array
    QMutex mutex;
};

QTrackerDirectResultPrivate::QTrackerDirectResultPrivate(   QTrackerDirectResult* result,
                                                            QTrackerDirectDriverPrivate *dpp,
                                                            QTrackerDirectFetcherPrivate *f)
: cursor(0), isFinished(false), resultAlive(true), loop(0),
  q(result), driverPrivate(dpp), fetcher(f), mutex(QMutex::Recursive)
{
}

QTrackerDirectResultPrivate::~QTrackerDirectResultPrivate()
{
    if (cursor != 0)
        g_object_unref(cursor);

    if (fetcher->isRunning()) {
        fetcher->terminate();
        if (!fetcher->wait(500))
            return;
    }
}

void QTrackerDirectResultPrivate::terminate()
{
    if (results.count() % driverPrivate->dataReadyInterval != 0) {
        dataReady(results.count());
    }

    isFinished = true;
    q->emit finished();
    if (cursor != 0) {
        g_object_unref(cursor);
        cursor = 0;
    }
    if (loop != 0)
        loop->exit();
}

void QTrackerDirectResultPrivate::setLastError(const QSparqlError& e)
{
    q->setLastError(e);
}

void QTrackerDirectResultPrivate::setBoolValue(bool v)
{
    q->setBoolValue(v);
}

void QTrackerDirectResultPrivate::dataReady(int totalCount)
{
    emit q->dataReady(totalCount);
}

QTrackerDirectResult::QTrackerDirectResult(QTrackerDirectDriverPrivate* p)
{
    d = new QTrackerDirectResultPrivate(this, p, new QTrackerDirectFetcherPrivate(this));
}

QTrackerDirectResult::~QTrackerDirectResult()
{
    QMutexLocker connectionLocker(&(d->driverPrivate->mutex));
    delete d;
}

QTrackerDirectResult* QTrackerDirectDriver::exec(const QString& query, QSparqlQuery::StatementType type)
{
    QTrackerDirectResult* res = new QTrackerDirectResult(d);
    res->exec(query, type);
    return res;
}

void QTrackerDirectResult::exec(const QString& query, QSparqlQuery::StatementType type)
{
    setQuery(query);
    setStatementType(type);

    if (    type != QSparqlQuery::AskStatement
            && type != QSparqlQuery::SelectStatement
            && type != QSparqlQuery::InsertStatement
            && type != QSparqlQuery::DeleteStatement )
    {
        setLastError(QSparqlError(
                              QLatin1String("Unsupported statement type"),
                              QSparqlError::BackendError));
        qWarning() << "QTrackerDirectResult:" << lastError() << query;
        terminate();
        return;
    }

    d->fetcher->start();
}

bool QTrackerDirectResult::exec()
{
    QMutexLocker connectionLocker(&(d->driverPrivate->mutex));

    GError * error = 0;
    if (isTable() || isBool()) {
        d->cursor = tracker_sparql_connection_query(    d->driverPrivate->connection,
                                                        query().toLatin1().constData(),
                                                        0,
                                                        &error );
        if (error != 0 || d->cursor == 0) {
            QSparqlError e(QString::fromLatin1(error ? error->message : "unknown error"));
            e.setType(QSparqlError::StatementError);
            setLastError(e);
            g_error_free(error);
            qWarning() << "QTrackerDirectResult:" << lastError() << query();
            terminate();
            return false;
        }
    } else {
        tracker_sparql_connection_update(   d->driverPrivate->connection,
                                            query().toLatin1().constData(),
                                            0,
                                            0,
                                            &error );
        if (error != 0) {
            QSparqlError e(QString::fromLatin1(error ? error->message : "unknown error"));
            g_error_free(error);
            e.setType(QSparqlError::StatementError);
            setLastError(e);
            qWarning() << "QTrackerDirectResult:" << lastError() << query();
            terminate();
            return false;
        }
    }

    return true;
}

void QTrackerDirectResult::cleanup()
{
    setPos(QSparql::BeforeFirstRow);
}

bool QTrackerDirectResult::fetchNextResult()
{
    QMutexLocker connectionLocker(&(d->driverPrivate->mutex));

    GError * error = 0;
    gboolean active = tracker_sparql_cursor_next(d->cursor, 0, &error);

    if (error != 0) {
        QSparqlError e(QString::fromLatin1(error ? error->message : "unknown error"));
        g_error_free(error);
        e.setType(QSparqlError::BackendError);
        setLastError(e);
        qWarning() << "QTrackerDirectResult:" << lastError() << query();
        terminate();
        return false;
    }

    if (!active) {
        terminate();
        return false;
    }

    QMutexLocker resultLocker(&(d->mutex));

    QSparqlResultRow resultRow;
    gint n_columns = tracker_sparql_cursor_get_n_columns(d->cursor);

    if (d->columnNames.empty()) {
        for (int i = 0; i < n_columns; i++) {
            d->columnNames.append(QString::fromUtf8(tracker_sparql_cursor_get_variable_name(d->cursor, i)));
        }
    }

    for (int i = 0; i < n_columns; i++) {
        QString value = QString::fromUtf8(tracker_sparql_cursor_get_string(d->cursor, i, 0));
        QSparqlBinding binding;
        binding.setName(d->columnNames[i]);
        TrackerSparqlValueType type = tracker_sparql_cursor_get_value_type(d->cursor, i);

        switch (type) {
        case TRACKER_SPARQL_VALUE_TYPE_UNBOUND:
            break;
        case TRACKER_SPARQL_VALUE_TYPE_URI:
            binding.setValue(QUrl(value));
            break;
        case TRACKER_SPARQL_VALUE_TYPE_STRING:
            binding.setValue(value);
            break;
        case TRACKER_SPARQL_VALUE_TYPE_INTEGER:
            binding.setValue(value, *XSD::Integer());
            break;
        case TRACKER_SPARQL_VALUE_TYPE_DOUBLE:
            binding.setValue(value, *XSD::Double());
            break;
        case TRACKER_SPARQL_VALUE_TYPE_DATETIME:
            binding.setValue(value, *XSD::DateTime());
            break;
        case TRACKER_SPARQL_VALUE_TYPE_BLANK_NODE:
            binding.setBlankNodeLabel(value);
            break;
        case TRACKER_SPARQL_VALUE_TYPE_BOOLEAN:
            if (value == QLatin1String("1") || value.toLower() == QLatin1String("true"))
                binding.setValue(QString::fromLatin1("true"), *XSD::Boolean());
            else
                binding.setValue(QString::fromLatin1("false"), *XSD::Boolean());
            break;
        default:
            break;
        }

        resultRow.append(binding);
    }

    d->results.append(resultRow);
    if (d->results.count() % d->driverPrivate->dataReadyInterval == 0) {
        d->dataReady(d->results.count());
    }

    return true;
}

bool QTrackerDirectResult::fetchBoolResult()
{
    QMutexLocker connectionLocker(&(d->driverPrivate->mutex));

    GError * error = 0;
    tracker_sparql_cursor_next(d->cursor, 0, &error);
    if (error != 0) {
        QSparqlError e(QString::fromLatin1(error ? error->message : "unknown error"));
        g_error_free(error);
        e.setType(QSparqlError::BackendError);
        setLastError(e);
        qWarning() << "QTrackerDirectResult:" << lastError() << query();
        terminate();
        return false;
    }

    QMutexLocker resultLocker(&(d->mutex));

    if (tracker_sparql_cursor_get_n_columns(d->cursor) == 1)  {
        QString value = QString::fromUtf8(tracker_sparql_cursor_get_string(d->cursor, 0, 0));
        TrackerSparqlValueType type = tracker_sparql_cursor_get_value_type(d->cursor, 0);

        if (    type == TRACKER_SPARQL_VALUE_TYPE_BOOLEAN
                && (value == QLatin1String("1") || value.toLower() == QLatin1String("true")) )
        {
            d->setBoolValue(true);
        }
    }

    terminate();
    return true;
}

QSparqlBinding QTrackerDirectResult::binding(int field) const
{
    QMutexLocker resultLocker(&(d->mutex));

    if (!isValid()) {
        return QSparqlBinding();
    }

    if (field >= d->results[pos()].count() || field < 0) {
        qWarning() << "QTrackerDirectResult::data[" << pos() << "]: column" << field << "out of range";
        return QSparqlBinding();
    }

    return d->results[pos()].binding(field);
}

QVariant QTrackerDirectResult::value(int field) const
{
    QMutexLocker resultLocker(&(d->mutex));

    if (!isValid()) {
        return QVariant();
    }

    if (field >= d->results[pos()].count() || field < 0) {
        qWarning() << "QTrackerDirectResult::data[" << pos() << "]: column" << field << "out of range";
        return QVariant();
    }

    return d->results[pos()].value(field);
}

void QTrackerDirectResult::waitForFinished()
{
    if (d->isFinished)
        return;

    QEventLoop loop;
    d->loop = &loop;
    loop.exec();
    d->loop = 0;
}

bool QTrackerDirectResult::isFinished() const
{
    return d->isFinished;
}

void QTrackerDirectResult::terminate()
{
    d->terminate();
}

int QTrackerDirectResult::size() const
{
    QMutexLocker resultLocker(&(d->mutex));
    return d->results.count();
}

QSparqlResultRow QTrackerDirectResult::current() const
{
    QMutexLocker resultLocker(&(d->mutex));

    if (!isValid()) {
        return QSparqlResultRow();
    }

    if (pos() < 0 || pos() >= d->results.count())
        return QSparqlResultRow();

    return d->results[pos()];
}

QTrackerDirectDriverPrivate::QTrackerDirectDriverPrivate()
    : dataReadyInterval(1), mutex(QMutex::Recursive)
{
}

QTrackerDirectDriverPrivate::~QTrackerDirectDriverPrivate()
{
}

QTrackerDirectDriver::QTrackerDirectDriver(QObject* parent)
    : QSparqlDriver(parent)
{
    d = new QTrackerDirectDriverPrivate();
    /* Initialize GLib type system */
    g_type_init();

}

QTrackerDirectDriver::~QTrackerDirectDriver()
{
    delete d;
}

bool QTrackerDirectDriver::hasFeature(QSparqlConnection::Feature f) const
{
    switch (f) {
    case QSparqlConnection::QuerySize:
        return true;
    case QSparqlConnection::AskQueries:
        return true;
    case QSparqlConnection::ConstructQueries:
        return false;
    case QSparqlConnection::UpdateQueries:
        return true;
    case QSparqlConnection::DefaultGraph:
        return true;
    }
    return false;
}

QAtomicInt connectionCounter = 0;

bool QTrackerDirectDriver::open(const QSparqlConnectionOptions& options)
{
    QMutexLocker connectionLocker(&(d->mutex));

    d->dataReadyInterval = options.dataReadyInterval();

    if (isOpen())
        close();

    GError *error = 0;
    d->connection = tracker_sparql_connection_get(0, &error);
    // maybe-TODO: Add the GCancellable
    if (!d->connection) {
        qWarning("Couldn't obtain a direct connection to the Tracker store: %s",
                    error ? error->message : "unknown error");
        g_error_free(error);
        return false;
    }

    setOpen(true);
    setOpenError(false);

    return true;
}

void QTrackerDirectDriver::close()
{
    QMutexLocker connectionLocker(&(d->mutex));
//    g_object_unref(d->connection);

    if (isOpen()) {
        setOpen(false);
        setOpenError(false);
    }
}

QT_END_NAMESPACE

#include "qsparql_tracker_direct.moc"
