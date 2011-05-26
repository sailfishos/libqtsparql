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

#include "qsparql_tracker_direct_driver_p.h"
#include "qsparql_tracker_direct_result_p.h"
#include "qsparql_tracker_direct_sync_result_p.h"
#include "qsparql_tracker_direct_update_result_p.h"

#include <qsparqlconnection.h>

#include <QtCore/qdatetime.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qeventloop.h>

#include <QtCore/qdebug.h>
#include <QtCore/qmetaobject.h>

QT_BEGIN_NAMESPACE

// Helper functions used both by QTrackerDirectResult and
// QTrackerDirectSyncResult

namespace {

QVariant makeVariant(TrackerSparqlValueType type, TrackerSparqlCursor* cursor, int col)
{
    glong strLen = 0;
    const gchar* strData = 0;

    // Get string data for types returned as strings from tracker
    switch (type) {
    case TRACKER_SPARQL_VALUE_TYPE_URI:
    case TRACKER_SPARQL_VALUE_TYPE_STRING:
    case TRACKER_SPARQL_VALUE_TYPE_DATETIME:
        strData = tracker_sparql_cursor_get_string(cursor, col, &strLen);
        break;
    default:
        break;
    }

    switch (type) {
    case TRACKER_SPARQL_VALUE_TYPE_UNBOUND:
        break;
    case TRACKER_SPARQL_VALUE_TYPE_URI:
    {
        const QByteArray ba(strData, strLen);
        return QVariant(QUrl::fromEncoded(ba));
    }
    case TRACKER_SPARQL_VALUE_TYPE_STRING:
    {
        return QVariant(QString::fromUtf8(strData, strLen));
    }
    case TRACKER_SPARQL_VALUE_TYPE_INTEGER:
    {
        const gint64 value = tracker_sparql_cursor_get_integer(cursor, col);
        return QVariant(qlonglong(value));
    }
    case TRACKER_SPARQL_VALUE_TYPE_DOUBLE:
    {
        const gdouble value = tracker_sparql_cursor_get_double(cursor, col);
        return QVariant(double(value));
    }
    case TRACKER_SPARQL_VALUE_TYPE_DATETIME:
        {
        return QVariant(QDateTime::fromString(QString::fromUtf8(strData, strLen), Qt::ISODate));
        }
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
        const gboolean value = tracker_sparql_cursor_get_boolean(cursor, col);
        return QVariant(value != FALSE);
    }
    default:
        break;
    }

    return QVariant();
}

}  // namespace

QVariant readVariant(TrackerSparqlCursor* cursor, int col)
{
    const TrackerSparqlValueType type =
        tracker_sparql_cursor_get_value_type(cursor, col);
    return makeVariant(type, cursor, col);
}

QSparqlError::ErrorType errorCodeToType(gint code)
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

gint qSparqlPriorityToGlib(QSparqlQueryOptions::Priority priority)
{
    switch (priority) {
    case QSparqlQueryOptions::LowPriority:
        return G_PRIORITY_LOW;
    case QSparqlQueryOptions::NormalPriority:
    default:
        return G_PRIORITY_DEFAULT;
    }
}

////////////////////////////////////////////////////////////////////////////

static void
async_open_callback(GObject         * /*source_object*/,
                    GAsyncResult    *result,
                    gpointer         user_data)
{
    QTrackerDirectDriverPrivate *d = static_cast<QTrackerDirectDriverPrivate*>(user_data);
    d->asyncOpenComplete(result);
}

QTrackerDirectDriverPrivate::QTrackerDirectDriverPrivate(QTrackerDirectDriver *driver)
    : connection(0), dataReadyInterval(1), connectionMutex(QMutex::Recursive), driver(driver),
      asyncOpenCalled(false)
{
}

QTrackerDirectDriverPrivate::~QTrackerDirectDriverPrivate()
{
}

void QTrackerDirectDriverPrivate::asyncOpenComplete(GAsyncResult *result)
{
    if (!connection) {
        GError * error = 0;
        connection = tracker_sparql_connection_get_finish(result, &error);
        checkConnectionError(connection, error);
    }
    asyncOpenCalled = true;
    Q_EMIT driver->opened();
}

void QTrackerDirectDriverPrivate::checkConnectionError(TrackerSparqlConnection *conn, GError* gerr)
{
    if (!conn) {
        QString trackerErr(QString::fromUtf8("unknown error"));
        if (gerr) {
            if (gerr->message)
                trackerErr = QString::fromUtf8(gerr->message);
            g_error_free(gerr);
        }
        error = QString::fromUtf8("Couldn't obtain a direct connection to the Tracker store: ") + trackerErr;
        qWarning() << error;
        driver->setOpen(false);
    }
}

void QTrackerDirectDriverPrivate::addActiveResult(QTrackerDirectResult* result)
{
    for (QList<QPointer<QTrackerDirectResult> >::iterator it = activeResults.begin();
            it != activeResults.end(); ++it) {
        if (it->isNull()) {
            // Replace entry for deleted result if one is found. This is done
            // to avoid the activeResults list from growing indefinitely.
            *it = result;
            return;
        }
    }
    // No deleted result found, append new one
    activeResults.append(result);
}

void QTrackerDirectDriverPrivate::onConnectionOpen(QObject* object, const char* method, const char* slot)
{
    if (connection || asyncOpenCalled) {
        QMetaObject::invokeMethod(object, method, Qt::DirectConnection);
    }
    else if (!asyncOpenCalled) {
        QObject::connect(driver, SIGNAL(opened()), object, slot, Qt::UniqueConnection);
    }
}

void QTrackerDirectDriverPrivate::waitForConnectionOpen()
{
    if (!asyncOpenCalled) {
        if (QCoreApplication::instance()) {
            QEventLoop loop;
            QObject::connect(driver, SIGNAL(opened()), &loop, SLOT(quit()));
            loop.exec();
        }
        else {
            qWarning() << "QTRACKER_DIRECT: QCoreApplication instance not found: cannot wait for asynchronous connection open";
        }
    }
}

void QTrackerDirectDriverPrivate::openConnectionSync()
{

    if (!asyncOpenCalled && !connection) {
        GError* error = 0;
        this->connection = tracker_sparql_connection_get(0, &error);
        checkConnectionError(this->connection, error);
    }
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
    case QSparqlConnection::AsyncExec:
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

    //Get the options for the thread pool, if no expiry time has been set
    //set our own value of 2 seconds (the default value is 30 seconds)
    if(options.threadExpiryTime() != -1)
        d->threadPool.setExpiryTimeout(options.threadExpiryTime());
    else
        d->threadPool.setExpiryTimeout(2000);

    //get the max thread count from an option if it was set, else
    //we'll set it to 2 * the qt default of number of cores
    if(options.maxThreadCount() > 0)
        d->threadPool.setMaxThreadCount(options.maxThreadCount());
    else {
        int maxThreads = d->threadPool.maxThreadCount() * 2;
        d->threadPool.setMaxThreadCount(maxThreads);
    }

    return true;
}

void QTrackerDirectDriver::close()
{
    Q_FOREACH(QPointer<QTrackerDirectResult> result, d->activeResults) {
        if (!result.isNull()) {
            qWarning() << "QSparqlConnection closed before QSparqlResult with query:" <<
                result->query();
            disconnect(this, SIGNAL(opened()), result.data(), SLOT(exec()));
            result->stopAndWait();
        }
    }
    d->activeResults.clear();
    QMutexLocker connectionLocker(&(d->connectionMutex));

    // Need to wait for the connection to open because there is no good way
    // to cancel it synchronously
    d->waitForConnectionOpen();
    if (d->connection) {
        g_object_unref(d->connection);
        d->connection = 0;
    }

    if (isOpen()) {
        setOpen(false);
        setOpenError(false);
    }
}

QSparqlResult* QTrackerDirectDriver::exec(const QString &query, QSparqlQuery::StatementType type, const QSparqlQueryOptions& options)
{
    QSparqlResult* result = 0;

    switch (options.executionMethod()) {
    case QSparqlQueryOptions::AsyncExec:
        result = asyncExec(query, type, options);
        break;
    case QSparqlQueryOptions::SyncExec:
        result = syncExec(query, type, options);
        break;
    }

    return result;
}

QSparqlResult* QTrackerDirectDriver::asyncExec(const QString &query, QSparqlQuery::StatementType type, const QSparqlQueryOptions& options)
{
    if (type == QSparqlQuery::AskStatement || type == QSparqlQuery::SelectStatement) {
        QTrackerDirectResult *result = new QTrackerDirectResult(d, query, type);
        d->addActiveResult(result);
        d->onConnectionOpen(result, "exec", SLOT(exec()));
        return result;
    } else {
        QTrackerDirectUpdateResult *result = new QTrackerDirectUpdateResult(d, query, type, options);
        d->onConnectionOpen(result, "exec", SLOT(exec()));
        return result;
    }
}

QSparqlResult* QTrackerDirectDriver::syncExec
        (const QString& query, QSparqlQuery::StatementType type, const QSparqlQueryOptions& options)
{
    QTrackerDirectSyncResult* result = new QTrackerDirectSyncResult(d, options);
    result->setQuery(query);
    result->setStatementType(type);
    if (type == QSparqlQuery::AskStatement || type == QSparqlQuery::SelectStatement) {
        d->openConnectionSync();
        result->exec();
    } else if (type == QSparqlQuery::InsertStatement || type == QSparqlQuery::DeleteStatement) {
        d->openConnectionSync();
        result->update();
    }

    return result;
}

QT_END_NAMESPACE
