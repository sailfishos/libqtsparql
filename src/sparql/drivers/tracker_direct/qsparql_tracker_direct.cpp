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

#include "qsparql_tracker_direct.h"
#include "qsparql_tracker_direct_p.h"

#include <qsparqlerror.h>
#include <qsparqlbinding.h>
#include <qsparqlquery.h>
#include <qsparqlresultrow.h>

#include <qcoreapplication.h>
#include <qvariant.h>
#include <qstringlist.h>
#include <qvector.h>

#include <qdebug.h>

QT_BEGIN_NAMESPACE

static void
async_cursor_next_callback( GObject *source_object,
                            GAsyncResult *result,
                            gpointer user_data)
{
    Q_UNUSED(source_object);
    QTrackerDirectResultPrivate *data = static_cast<QTrackerDirectResultPrivate*>(user_data);
    GError *error = 0;
    gboolean active = tracker_sparql_cursor_next_finish(data->cursor, result, &error);

    if (error != 0) {
        QSparqlError e(QString::fromLatin1(error ? error->message : "unknown error"));
        e.setType(QSparqlError::BackendError);
        data->setLastError(e);
        g_error_free(error);
        data->terminate();
        return;
    }

    if (!active) {
        if (data->q->isBool()) {
            data->setBoolValue( data->results.count() == 1
                                && data->results[0].count() == 1
                                && data->results[0].binding(0).value().toBool());
        }

        data->terminate();
        return;
    }

    QSparqlResultRow resultRow;
    gint n_columns = tracker_sparql_cursor_get_n_columns(data->cursor);

    if (data->columnNames.empty()) {
        for (int i = 0; i < n_columns; i++) {
            data->columnNames.append(QString::fromUtf8(tracker_sparql_cursor_get_variable_name(data->cursor, i)));
        }
    }

    for (int i = 0; i < n_columns; i++) {
        QString value = QString::fromUtf8(tracker_sparql_cursor_get_string(data->cursor, i, 0));
        QSparqlBinding binding;
        binding.setName(data->columnNames[i]);
        TrackerSparqlValueType type = tracker_sparql_cursor_get_value_type(data->cursor, i);

        switch (type) {
        case TRACKER_SPARQL_VALUE_TYPE_UNBOUND:
            break;
        case TRACKER_SPARQL_VALUE_TYPE_URI:
            binding.setValue(QUrl(value));
            break;
        case TRACKER_SPARQL_VALUE_TYPE_STRING:
            binding.setValue(value, QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#string"));
            break;
        case TRACKER_SPARQL_VALUE_TYPE_INTEGER:
            binding.setValue(value, QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#integer"));
            break;
        case TRACKER_SPARQL_VALUE_TYPE_DOUBLE:
            binding.setValue(value, QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#double"));
            break;
        case TRACKER_SPARQL_VALUE_TYPE_DATETIME:
            binding.setValue(value, QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#dateTime"));
            break;
        case TRACKER_SPARQL_VALUE_TYPE_BLANK_NODE:
            binding.setBlankNodeLabel(value);
            break;
        case TRACKER_SPARQL_VALUE_TYPE_BOOLEAN:
            binding.setValue(value == QLatin1String("1") ? QString::fromLatin1("true") : QString::fromLatin1("false"),
                             QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#boolean"));
            break;
        default:
            break;
        }

        resultRow.append(binding);
    }

    data->results.append(resultRow);
    data->dataReady(data->results.count());
    tracker_sparql_cursor_next_async(data->cursor, 0, async_cursor_next_callback, data);
}

static void
async_query_callback(   GObject *source_object,
                        GAsyncResult *result,
                        gpointer user_data)
{
    Q_UNUSED(source_object);
    QTrackerDirectResultPrivate *data = static_cast<QTrackerDirectResultPrivate*>(user_data);
    GError *error = 0;
    data->cursor = tracker_sparql_connection_query_finish(data->driverPrivate->connection, result, &error);

    if (error != 0 || data->cursor == 0) {
        QSparqlError e(QString::fromLatin1(error ? error->message : "unknown error"));
        e.setType(QSparqlError::StatementError);
        data->setLastError(e);
        g_error_free(error);
        data->terminate();
        return;
    }

    tracker_sparql_cursor_next_async(data->cursor, 0, async_cursor_next_callback, data);
}

static void
async_update_callback( GObject *source_object,
                       GAsyncResult *result,
                       gpointer user_data)
{
    Q_UNUSED(source_object);
    QTrackerDirectResultPrivate *data = static_cast<QTrackerDirectResultPrivate*>(user_data);
    GError *error = 0;
    tracker_sparql_connection_update_finish(data->driverPrivate->connection, result, &error);

    if (error != 0) {
        QSparqlError e(QString::fromLatin1(error ? error->message : "unknown error"));
        e.setType(QSparqlError::StatementError);
        data->setLastError(e);
        g_error_free(error);
        data->terminate();
        return;
    }

    data->terminate();
}

QTrackerDirectResultPrivate::QTrackerDirectResultPrivate(QTrackerDirectResult* result, QTrackerDirectDriverPrivate *dpp)
: cursor(0), isFinished(false), loop(0), q(result), driverPrivate(dpp)
{
}

QTrackerDirectResultPrivate::~QTrackerDirectResultPrivate()
{
    if (cursor != 0)
        g_object_unref(cursor);
}

void QTrackerDirectResultPrivate::terminate()
{
    isFinished = true;
    q->emit finished();

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
    d = new QTrackerDirectResultPrivate(this, p);
}

QTrackerDirectResult::~QTrackerDirectResult()
{
    delete d;
}

QTrackerDirectResult* QTrackerDirectDriver::exec(const QString& query,
                          QSparqlQuery::StatementType type)
{
    QTrackerDirectResult* res = new QTrackerDirectResult(d);
    res->setStatementType(type);

    switch (type) {
    case QSparqlQuery::AskStatement:
    case QSparqlQuery::SelectStatement:
    {
        tracker_sparql_connection_query_async(  d->connection,
                                                query.toLatin1().constData(),
                                                0,
                                                async_query_callback,
                                                res->d);
        break;
    }
    case QSparqlQuery::InsertStatement:
    case QSparqlQuery::DeleteStatement:
        tracker_sparql_connection_update_async( d->connection,
                                                query.toLatin1().constData(),
                                                0,
                                                0,
                                                async_update_callback,
                                                res->d);
    {
        break;
    }
    default:
        qWarning() << "Tracker backend: unsupported statement type";
        res->setLastError(QSparqlError(
                              QLatin1String("Unsupported statement type"),
                              QSparqlError::BackendError));
        break;
    }
    return res;
}

void QTrackerDirectResult::cleanup()
{
    setPos(QSparql::BeforeFirstRow);
}

QSparqlBinding QTrackerDirectResult::binding(int field) const
{
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

int QTrackerDirectResult::size() const
{
    return d->results.count();
}

QSparqlResultRow QTrackerDirectResult::current() const
{
    if (!isValid()) {
        return QSparqlResultRow();
    }

    if (pos() < 0 || pos() >= d->results.count())
        return QSparqlResultRow();

    return d->results[pos()];
}

QTrackerDirectDriverPrivate::QTrackerDirectDriverPrivate()
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
    Q_UNUSED(options);

    if (isOpen())
        close();

    GError *error = 0;
    d->connection = tracker_sparql_connection_get(&error);
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
    // This is commented out because of a bug in tracker (requesting a
    // connection twice fails). However, connections are singletons so this
    // leaks only one connection.  TODO: enable this when the tracker bug is
    // fixed.
    // g_object_unref(d->connection);

    if (isOpen()) {
        setOpen(false);
        setOpenError(false);
    }
}

QT_END_NAMESPACE
