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

#include <QtCore/qcoreapplication.h>
#include <QtCore/qvariant.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qvector.h>

#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

// TODO: centralize the xsd uris into one place
namespace XSD {
Q_GLOBAL_STATIC_WITH_ARGS(QUrl, Integer,
                          (QLatin1String("http://www.w3.org/2001/XMLSchema#integer")))
}

////////////////////////////////////////////////////////////////////////////

// Helper functions used both by QTrackerDirectResult and
// QTrackerDirectSyncResult

namespace {

static QVariant makeVariant(TrackerSparqlValueType type, const gchar* value, glong length)
{
    switch (type) {
    case TRACKER_SPARQL_VALUE_TYPE_UNBOUND:
        break;
    case TRACKER_SPARQL_VALUE_TYPE_URI:
        return QVariant(QUrl::fromEncoded(QByteArray(value, length)));
    case TRACKER_SPARQL_VALUE_TYPE_STRING:
        return QVariant(QString::fromUtf8(value, length));
    case TRACKER_SPARQL_VALUE_TYPE_INTEGER:
    {
        // It's safe to use QByteArray::setRawData here, since there won't be
        // pointers to the byte array after the conversion.
        QByteArray ba;
        ba.setRawData(value, length);
        return QVariant(ba.toLongLong());
    }
    case TRACKER_SPARQL_VALUE_TYPE_DOUBLE:
    {
        // It's safe to use QByteArray::setRawData here, since there won't be
        // pointers to the byte array after the conversion.
        QByteArray ba;
        ba.setRawData(value, length);
        return QVariant(ba.toDouble());
    }
    case TRACKER_SPARQL_VALUE_TYPE_DATETIME:
        return QVariant(QDateTime::fromString(QString::fromUtf8(value, length), Qt::ISODate));
    case TRACKER_SPARQL_VALUE_TYPE_BLANK_NODE:
        // Note: this type is not currently used by Tracker.  Here we're storing
        // it as a null QVariant and losing information that it was a blank
        // node.  If Tracker starts using this type, a possible solution would
        // be to store the blank node label into a QVariant along with some type
        // information indicating that it was a blank node, and utilizing that
        // information in binding().
        break;
    case TRACKER_SPARQL_VALUE_TYPE_BOOLEAN:
    {
        bool isTrue = (qstrcmp(value, "1") == 0 || qstricmp(value, "true") == 0);
        return QVariant(isTrue);
    }
    default:
        break;
    }

    return QVariant();
}

QVariant readVariant(TrackerSparqlCursor* cursor, int col)
{
    TrackerSparqlValueType type =
        tracker_sparql_cursor_get_value_type(cursor, col);
    glong len;
    const gchar* data = tracker_sparql_cursor_get_string(cursor, col, &len);
    return makeVariant(type, data, len);
}

}

static QSparqlError::ErrorType errorCodeToType(gint code)
{
    switch (static_cast<TrackerSparqlError>(code)) {
    case TRACKER_SPARQL_ERROR_PARSE:
        return QSparqlError::StatementError;
    case TRACKER_SPARQL_ERROR_UNKNOWN_CLASS:
        return QSparqlError::StatementError;
    case TRACKER_SPARQL_ERROR_UNKNOWN_PROPERTY:
        return QSparqlError::StatementError;
    case TRACKER_SPARQL_ERROR_TYPE:
        return QSparqlError::StatementError;
    case TRACKER_SPARQL_ERROR_CONSTRAINT:
        return QSparqlError::ConnectionError;
    case TRACKER_SPARQL_ERROR_NO_SPACE:
        return QSparqlError::BackendError;
    case TRACKER_SPARQL_ERROR_INTERNAL:
        return QSparqlError::BackendError;
    case TRACKER_SPARQL_ERROR_UNSUPPORTED:
        return QSparqlError::BackendError;
    default:
        return QSparqlError::BackendError;
    }
}

////////////////////////////////////////////////////////////////////////////
// FIXME: refactor QTrackerDirectResult to use QTrackerDirectSyncResult +
// sync->async wrapper.

class QTrackerDirectFetcherPrivate : public QThread
{
public:
    QTrackerDirectFetcherPrivate(QTrackerDirectResult *res) : result(res) { }

    void run()
    {
        if (result->runQuery()) {
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

private:
    QTrackerDirectResult *result;
};

class QTrackerDirectDriverPrivate {
public:
    QTrackerDirectDriverPrivate(QTrackerDirectDriver *driver);
    ~QTrackerDirectDriverPrivate();

    void setOpen(bool open);
    void opened();

    TrackerSparqlConnection *connection;
    int dataReadyInterval;
    // This mutex is for ensuring that only one thread at a time
    // is using the connection to make tracker queries. This mutex
    // probably isn't needed as a TrackerSparqlConnection is
    // already thread safe.
    QMutex connectionMutex;
    QTrackerDirectDriver *driver;
    bool asyncOpenCalled;
    QString error;
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

    TrackerSparqlCursor* cursor;
    QVector<QString> columnNames;
    QVector<QVariantList> results;
    QAtomicInt isFinished;

    QTrackerDirectResult* q;
    QTrackerDirectDriverPrivate *driverPrivate;
    QTrackerDirectFetcherPrivate *fetcher;
    bool fetcherStarted;
    // This mutex is for ensuring that only one thread at a time is accessing
    // the member variables of this class (mainly: "results").
    // Note that only the fetcher thread accesses the 'cursor', and so that
    // is protected by the connectionMutex in the driver, not this mutex.
    QMutex resultMutex;
};

class QTrackerDirectUpdateResultPrivate : public QObject {
    Q_OBJECT
public:
    QTrackerDirectUpdateResultPrivate(QTrackerDirectUpdateResult* result, QTrackerDirectDriverPrivate *dpp);

    ~QTrackerDirectUpdateResultPrivate();
    void terminate();
    void setLastError(const QSparqlError& e);

    QAtomicInt isFinished;
    bool resultAlive; // whether the corresponding Result object is still alive
    QEventLoop *loop;

    QTrackerDirectUpdateResult* q;
    QTrackerDirectDriverPrivate *driverPrivate;
};

static void
async_update_callback( GObject *source_object,
                       GAsyncResult *result,
                       gpointer user_data)
{
    Q_UNUSED(source_object);
    QTrackerDirectUpdateResultPrivate *data = static_cast<QTrackerDirectUpdateResultPrivate*>(user_data);
    if (!data->resultAlive) {
        // The user has deleted the Result object before this callback was
        // called. Just delete the ResultPrivate here and do nothing.
        delete data;
        return;
    }

    GError *error = 0;
    tracker_sparql_connection_update_finish(data->driverPrivate->connection, result, &error);

    if (error != 0) {
        QSparqlError e(QString::fromUtf8(error->message));
        e.setType(errorCodeToType(error->code));
        e.setNumber(error->code);
        data->setLastError(e);
        g_error_free(error);
    }

    // A workaround for http://bugreports.qt.nokia.com/browse/QTBUG-18434

    // We cannot emit the QSparqlResult::finished() signal directly here; so
    // delay it and emit it the next time the main loop spins.
    QMetaObject::invokeMethod(data->q, "terminate", Qt::QueuedConnection);
}

QTrackerDirectResultPrivate::QTrackerDirectResultPrivate(   QTrackerDirectResult* result,
                                                            QTrackerDirectDriverPrivate *dpp,
                                                            QTrackerDirectFetcherPrivate *f)
  : cursor(0),
  q(result), driverPrivate(dpp), fetcher(f), fetcherStarted(false),
  resultMutex(QMutex::Recursive)
{
}

QTrackerDirectResultPrivate::~QTrackerDirectResultPrivate()
{
    delete fetcher;

    if (cursor != 0) {
        g_object_unref(cursor);
        cursor = 0;
    }
}

void QTrackerDirectResultPrivate::terminate()
{
    QMutexLocker resultLocker(&resultMutex);

    if (results.count() % driverPrivate->dataReadyInterval != 0) {
        dataReady(results.count());
    }

    isFinished = 1;
    q->emit finished();
    if (cursor != 0) {
        g_object_unref(cursor);
        cursor = 0;
    }
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

////////////////////////////////////////////////////////////////////////////

QTrackerDirectResult::QTrackerDirectResult(QTrackerDirectDriverPrivate* p,
                                           const QString& query,
                                           QSparqlQuery::StatementType type)
{
    setQuery(query);
    setStatementType(type);
    d = new QTrackerDirectResultPrivate(this, p, new QTrackerDirectFetcherPrivate(this));
}

QTrackerDirectResult::~QTrackerDirectResult()
{
    if (d->fetcher->isRunning()) {
        d->isFinished = 1;
        d->fetcher->wait();
    }

    delete d;
}

void QTrackerDirectResult::exec()
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

void QTrackerDirectResult::startFetcher()
{
    QMutexLocker resultLocker(&(d->resultMutex));
    if (!d->fetcherStarted) {
        d->fetcherStarted = true;
        d->fetcher->start();
    }
}

bool QTrackerDirectResult::runQuery()
{
    QMutexLocker connectionLocker(&(d->driverPrivate->connectionMutex));

    GError * error = 0;
    d->cursor = tracker_sparql_connection_query(    d->driverPrivate->connection,
                                                    query().toUtf8().constData(),
                                                    0,
                                                    &error );
    if (error != 0 || d->cursor == 0) {
        QMutexLocker resultLocker(&(d->resultMutex));
        QSparqlError e(QString::fromUtf8(error ? error->message : "unknown error"),
                        QSparqlError::StatementError,
                        error ? error->code : -1);
        setLastError(e);
        if (error)
            g_error_free(error);
        qWarning() << "QTrackerDirectResult:" << lastError() << query();
        terminate();
        return false;
    }

    return true;
}

bool QTrackerDirectResult::fetchNextResult()
{
    QMutexLocker connectionLocker(&(d->driverPrivate->connectionMutex));

    GError * error = 0;
    gboolean active = tracker_sparql_cursor_next(d->cursor, 0, &error);

    if (error != 0) {
        QSparqlError e(QString::fromUtf8(error->message),
                       errorCodeToType(error->code),
                       error->code);
        g_error_free(error);
        setLastError(e);
        qWarning() << "QTrackerDirectResult:" << lastError() << query();
        terminate();
        return false;
    }

    if (!active) {
        terminate();
        return false;
    }

    QMutexLocker resultLocker(&(d->resultMutex));

    gint n_columns = tracker_sparql_cursor_get_n_columns(d->cursor);

    if (d->columnNames.empty()) {
        for (int i = 0; i < n_columns; i++) {
            d->columnNames.append(QString::fromUtf8(tracker_sparql_cursor_get_variable_name(d->cursor, i)));
        }
    }

    QVariantList resultRow;

    for (int i = 0; i < n_columns; i++) {
        resultRow.append(readVariant(d->cursor, i));
    }

    d->results.append(resultRow);
    if (d->results.count() % d->driverPrivate->dataReadyInterval == 0) {
        d->dataReady(d->results.count());
    }

    return true;
}

bool QTrackerDirectResult::fetchBoolResult()
{
    QMutexLocker connectionLocker(&(d->driverPrivate->connectionMutex));

    GError * error = 0;
    tracker_sparql_cursor_next(d->cursor, 0, &error);
    if (error != 0) {
        QSparqlError e(QString::fromUtf8(error->message),
                       errorCodeToType(error->code),
                       error->code);
        g_error_free(error);
        setLastError(e);
        qWarning() << "QTrackerDirectResult:" << lastError() << query();
        terminate();
        return false;
    }

    QMutexLocker resultLocker(&(d->resultMutex));

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
    QMutexLocker resultLocker(&(d->resultMutex));

    if (!isValid()) {
        return QSparqlBinding();
    }

    if (field >= d->results[pos()].count() || field < 0) {
        qWarning() << "QTrackerDirectResult::data[" << pos() << "]: column" << field << "out of range";
        return QSparqlBinding();
    }
    // A special case: we store TRACKER_SPARQL_VALUE_TYPE_INTEGER as longlong,
    // but its data type uri should be xsd:integer. Set it manually here.
    QSparqlBinding b;
    QVariant value = d->results[pos()][field];
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

QVariant QTrackerDirectResult::value(int field) const
{
    QMutexLocker resultLocker(&(d->resultMutex));

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
    if (d->isFinished == 1)
        return;

    // We first need the connection to be ready before doing anything
    if (!d->driverPrivate->asyncOpenCalled) {
        QEventLoop loop;
        connect(d->driverPrivate->driver, SIGNAL(opened()), &loop, SLOT(quit()));
        loop.exec();
    }

    if (!d->driverPrivate->driver->isOpen()) {
        setLastError(QSparqlError(d->driverPrivate->error,
                                  QSparqlError::ConnectionError));
        d->terminate();
        return;
    }

    startFetcher();
    d->fetcher->wait();
}

bool QTrackerDirectResult::isFinished() const
{
    return d->isFinished == 1;
}

void QTrackerDirectResult::terminate()
{
    d->terminate();
}

int QTrackerDirectResult::size() const
{
    QMutexLocker resultLocker(&(d->resultMutex));
    return d->results.size();
}

QSparqlResultRow QTrackerDirectResult::current() const
{
    QMutexLocker resultLocker(&(d->resultMutex));

    if (!isValid()) {
        return QSparqlResultRow();
    }

    if (pos() < 0 || pos() >= d->results.count())
        return QSparqlResultRow();

    if (d->columnNames.size() != d->results[pos()].size())
        return QSparqlResultRow();

    QSparqlResultRow resultRow;
    for (int i = 0; i < d->results[pos()].size(); ++i) {
        QSparqlBinding b(d->columnNames[i], d->results[pos()][i]);
        resultRow.append(b);
    }
    return resultRow;
}

////////////////////////////////////////////////////////////////////////////

QTrackerDirectUpdateResultPrivate::QTrackerDirectUpdateResultPrivate(   QTrackerDirectUpdateResult* result,
                                                            QTrackerDirectDriverPrivate *dpp)
  : resultAlive(true), loop(0),
  q(result), driverPrivate(dpp)
{
}

QTrackerDirectUpdateResultPrivate::~QTrackerDirectUpdateResultPrivate()
{
}

void QTrackerDirectUpdateResultPrivate::terminate()
{
    isFinished = 1;
    q->emit finished();

    if (loop != 0)
        loop->exit();
}

void QTrackerDirectUpdateResultPrivate::setLastError(const QSparqlError& e)
{
    q->setLastError(e);
}


////////////////////////////////////////////////////////////////////////////

QTrackerDirectUpdateResult::QTrackerDirectUpdateResult(QTrackerDirectDriverPrivate* p,
                                           const QString& query,
                                           QSparqlQuery::StatementType type)
{
    setQuery(query);
    setStatementType(type);
    d = new QTrackerDirectUpdateResultPrivate(this, p);
}

QTrackerDirectUpdateResult::~QTrackerDirectUpdateResult()
{
    if (d->isFinished == 0) {
        // We're deleting the Result before the async update has
        // finished. There is a pending async callback that will be called at
        // some point, and that will have our ResultPrivate as user_data. Don't
        // delete the ResultPrivate here, but just mark that we're no longer
        // alive. The callback will then delete the ResultPrivate.
        d->resultAlive = false;
        return;
    }

    delete d;
}

void QTrackerDirectUpdateResult::exec()
{
    if (!d->driverPrivate->driver->isOpen()) {
        setLastError(QSparqlError(d->driverPrivate->error,
                                  QSparqlError::ConnectionError));
        d->terminate();
        return;
    }

    tracker_sparql_connection_update_async( d->driverPrivate->connection,
                                            query().toUtf8().constData(),
                                            0,
                                            0,
                                            async_update_callback,
                                            d);
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
    if (d->isFinished == 1)
        return;

    // We first need the connection to be ready before doing anything
    if (!d->driverPrivate->asyncOpenCalled) {
        QEventLoop loop;
        connect(d->driverPrivate->driver, SIGNAL(opened()), &loop, SLOT(quit()));
        loop.exec();
    }

    if (!d->driverPrivate->driver->isOpen()) {
        setLastError(QSparqlError(d->driverPrivate->error,
                                  QSparqlError::ConnectionError));
        d->terminate();
        return;
    }

    QEventLoop loop;
    d->loop = &loop;
    loop.exec();
    d->loop = 0;
}

bool QTrackerDirectUpdateResult::isFinished() const
{
    return d->isFinished == 1;
}

void QTrackerDirectUpdateResult::terminate()
{
    d->terminate();
}

int QTrackerDirectUpdateResult::size() const
{
    return 0;
}

QSparqlResultRow QTrackerDirectUpdateResult::current() const
{
    return QSparqlResultRow();
}

////////////////////////////////////////////////////////////////////////////

struct QTrackerDirectSyncResultPrivate 
{
    QTrackerDirectSyncResultPrivate(QTrackerDirectDriverPrivate *dpp);
    ~QTrackerDirectSyncResultPrivate();
    TrackerSparqlCursor* cursor;
    int n_columns;
    QTrackerDirectDriverPrivate *driverPrivate;
};

QTrackerDirectSyncResultPrivate::QTrackerDirectSyncResultPrivate(QTrackerDirectDriverPrivate *dpp)
    : cursor(0), n_columns(-1), driverPrivate(dpp)
{
}

QTrackerDirectSyncResultPrivate::~QTrackerDirectSyncResultPrivate()
{
    if (cursor != 0)
        g_object_unref(cursor);
}

////////////////////////////////////////////////////////////////////////////

QTrackerDirectSyncResult::QTrackerDirectSyncResult(QTrackerDirectDriverPrivate* p)
{
    d = new QTrackerDirectSyncResultPrivate(p);
}

QTrackerDirectSyncResult::~QTrackerDirectSyncResult()
{
    delete d;
}

void QTrackerDirectSyncResult::exec()
{
    if (!d->driverPrivate->driver->isOpen()) {
        setLastError(QSparqlError(d->driverPrivate->error,
                                  QSparqlError::ConnectionError));
        return;
    }

    GError * error = 0;
    d->cursor = tracker_sparql_connection_query(d->driverPrivate->connection, query().toUtf8().constData(), 0, &error);
    if (error != 0 || d->cursor == 0) {
        QSparqlError e(QString::fromUtf8(error ? error->message : "unknown error"),
                        QSparqlError::StatementError,
                        error ? error->code : -1);
        setLastError(e);
        if (error != 0)
            g_error_free(error);
        qWarning() << "QTrackerDirectSyncResult:" << lastError() << query();
    }
}

void QTrackerDirectSyncResult::update()
{
    if (!d->driverPrivate->driver->isOpen()) {
        setLastError(QSparqlError(d->driverPrivate->error,
                                  QSparqlError::ConnectionError));
        return;
    }

    GError * error = 0;
    tracker_sparql_connection_update(d->driverPrivate->connection, query().toUtf8().constData(), 0, 0, &error);
    if (error != 0) {
        QSparqlError e(QString::fromUtf8(error->message),
                        errorCodeToType(error->code),
                        error->code);
        g_error_free(error);
        setLastError(e);
        qWarning() << "QTrackerDirectSyncResult:" << lastError() << query();
    }
}

bool QTrackerDirectSyncResult::next()
{
    if (d->cursor == 0)
        return false;

    GError * error = 0;
    gboolean active = tracker_sparql_cursor_next(d->cursor, 0, &error);

    if (error != 0) {
        QSparqlError e(QString::fromUtf8(error->message),
                       errorCodeToType(error->code),
                       error->code);
        g_error_free(error);
        setLastError(e);
        qWarning() << "QTrackerDirectSyncResult:" << lastError() << query();
        g_object_unref(d->cursor);
        d->cursor = 0;
        return false;
    }

    if (!active) {
        g_object_unref(d->cursor);
        d->cursor = 0;
        updatePos(QSparql::AfterLastRow);
        return false;
    }
    int oldPos = pos();
    if (oldPos == QSparql::BeforeFirstRow)
        updatePos(0);
    else
        updatePos(oldPos + 1);
    return true;
}

QSparqlResultRow QTrackerDirectSyncResult::current() const
{
    // Note: this function reads and constructs the data again every time it's called.
    if (d->cursor == 0 || pos() == QSparql::BeforeFirstRow || pos() == QSparql::AfterLastRow)
        return QSparqlResultRow();

    QSparqlResultRow resultRow;
    // get the no. of columns only once; it won't change between rows
    if (d->n_columns < 0)
        d->n_columns = tracker_sparql_cursor_get_n_columns(d->cursor);

    for (int i = 0; i < d->n_columns; i++) {
        resultRow.append(binding(i));
    }
    return resultRow;
}

QSparqlBinding QTrackerDirectSyncResult::binding(int i) const
{
    // Note: this function reads and constructs the data again every time it's called.
    if (d->cursor == 0 || pos() == QSparql::BeforeFirstRow || pos() == QSparql::AfterLastRow)
        return QSparqlBinding();

    // get the no. of columns only once; it won't change between rows
    if (d->n_columns < 0)
        d->n_columns = tracker_sparql_cursor_get_n_columns(d->cursor);

    if (i < 0 || i >= d->n_columns)
        return QSparqlBinding();

    const gchar* name = tracker_sparql_cursor_get_variable_name(d->cursor, i);
    QVariant value = readVariant(d->cursor, i);

    // A special case: we store TRACKER_SPARQL_VALUE_TYPE_INTEGER as longlong,
    // but its data type uri should be xsd:integer. Set it manually here.
    QSparqlBinding b;
    b.setName(QString::fromUtf8(name));
    if (value.type() == QVariant::LongLong) {
        b.setValue(value.toString(), *XSD::Integer());
    }
    else {
        b.setValue(value);
    }
    return b;
}

QVariant QTrackerDirectSyncResult::value(int i) const
{
    // Note: this function re-constructs the data every time it's called.
    if (d->cursor == 0 || pos() == QSparql::BeforeFirstRow || pos() == QSparql::AfterLastRow)
        return QVariant();

    // get the no. of columns only once; it won't change between rows
    if (d->n_columns < 0)
        d->n_columns = tracker_sparql_cursor_get_n_columns(d->cursor);

    if (i < 0 || i >= d->n_columns)
        return QVariant();

    return readVariant(d->cursor, i);
}

QString QTrackerDirectSyncResult::stringValue(int i) const
{
    if (d->cursor == 0 || pos() == QSparql::BeforeFirstRow || pos() == QSparql::AfterLastRow)
        return QString();

    // get the no. of columns only once; it won't change between rows
    if (d->n_columns < 0)
        d->n_columns = tracker_sparql_cursor_get_n_columns(d->cursor);

    if (i < 0 || i >= d->n_columns)
        return QString();

    return QString::fromUtf8(tracker_sparql_cursor_get_string(d->cursor, i, 0));
}

bool QTrackerDirectSyncResult::isFinished() const
{
    return (d->cursor == 0);
}

bool QTrackerDirectSyncResult::hasFeature(QSparqlResult::Feature feature) const
{
    switch (feature) {
    case QSparqlResult::Sync:
    case QSparqlResult::ForwardOnly:
        return true;
    case QSparqlResult::QuerySize:
        return false;
    default:
        return false;
    }
}

////////////////////////////////////////////////////////////////////////////

static void
async_open_callback(GObject         * /*source_object*/,
                    GAsyncResult    *result,
                    gpointer         user_data)
{
    QTrackerDirectDriverPrivate *d = static_cast<QTrackerDirectDriverPrivate*>(user_data);
    GError * error = 0;
    d->connection = tracker_sparql_connection_get_finish(result, &error);

    if (d->connection == 0) {
        d->error = QString::fromUtf8("Couldn't obtain a direct connection to the Tracker store: %1")
                                        .arg(QString::fromUtf8(error ? error->message : "unknown error"));
        qWarning() << d->error;
        g_error_free(error);
        d->setOpen(false);
    }

    d->asyncOpenCalled = true;
    d->opened();
}

QTrackerDirectDriverPrivate::QTrackerDirectDriverPrivate(QTrackerDirectDriver *driver)
    : connection(0), dataReadyInterval(1), connectionMutex(QMutex::Recursive), driver(driver),
      asyncOpenCalled(false)
{
}

QTrackerDirectDriverPrivate::~QTrackerDirectDriverPrivate()
{
}

void QTrackerDirectDriverPrivate::setOpen(bool open)
{
    driver->setOpen(open);
}

void QTrackerDirectDriverPrivate::opened()
{
    emit driver->opened();
}

QTrackerDirectDriver::QTrackerDirectDriver(QObject* parent)
    : QSparqlDriver(parent)
{
    d = new QTrackerDirectDriverPrivate(this);
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
    case QSparqlConnection::AskQueries:
    case QSparqlConnection::UpdateQueries:
    case QSparqlConnection::DefaultGraph:
    case QSparqlConnection::SyncExec:
        return true;
    case QSparqlConnection::ConstructQueries:
        return false;
    default:
        return false;
    }
    return false;
}

bool QTrackerDirectDriver::open(const QSparqlConnectionOptions& options)
{
    QMutexLocker connectionLocker(&(d->connectionMutex));

    d->dataReadyInterval = options.dataReadyInterval();

    if (isOpen())
        close();

    tracker_sparql_connection_get_async(0, async_open_callback, static_cast<gpointer>(d));
    setOpen(true);
    setOpenError(false);

    return true;
}

void QTrackerDirectDriver::close()
{
    QMutexLocker connectionLocker(&(d->connectionMutex));

    if (!d->asyncOpenCalled) {
        QEventLoop loop;
        connect(this, SIGNAL(opened()), &loop, SLOT(quit()));
        loop.exec();
    }

    if (d->connection != 0) {
        g_object_unref(d->connection);
        d->connection = 0;
    }

    if (isOpen()) {
        setOpen(false);
        setOpenError(false);
    }
}

QSparqlResult* QTrackerDirectDriver::exec(const QString &query, QSparqlQuery::StatementType type)
{
    if (type == QSparqlQuery::AskStatement || type == QSparqlQuery::SelectStatement) {
        QTrackerDirectResult *result = new QTrackerDirectResult(d, query, type);
        
        if (d->asyncOpenCalled) {
            result->exec();
        } else {
            connect(this, SIGNAL(opened()), result, SLOT(exec()));
        }
        
        return result;
    } else {
        QTrackerDirectUpdateResult *result = new QTrackerDirectUpdateResult(d, query, type);
        
        if (d->asyncOpenCalled) {
            result->exec();
        } else {
            connect(this, SIGNAL(opened()), result, SLOT(exec()));
        }
        
        return result;
    }
}

QSparqlResult* QTrackerDirectDriver::syncExec(const QString& query, QSparqlQuery::StatementType type)
{
    QTrackerDirectSyncResult* result = new QTrackerDirectSyncResult(d);
    result->setQuery(query);
    result->setStatementType(type);
    if (type == QSparqlQuery::AskStatement || type == QSparqlQuery::SelectStatement) {
        if (d->asyncOpenCalled) {
            result->exec();
        } else {
            QEventLoop loop;
            connect(this, SIGNAL(opened()), result, SLOT(exec()));
            connect(this, SIGNAL(opened()), &loop, SLOT(quit()));
            loop.exec();
        }
    } else if (type == QSparqlQuery::InsertStatement || type == QSparqlQuery::DeleteStatement) {
        if (d->asyncOpenCalled) {
            result->update();
        } else {
            QEventLoop loop;
            connect(this, SIGNAL(opened()), result, SLOT(update()));
            connect(this, SIGNAL(opened()), &loop, SLOT(quit()));
            loop.exec();
        }
    }

    return result;
}

QT_END_NAMESPACE

#include "qsparql_tracker_direct.moc"
