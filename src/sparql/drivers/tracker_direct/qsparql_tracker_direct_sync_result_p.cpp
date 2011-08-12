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

#include "qsparql_tracker_direct_sync_result_p.h"
#include "qsparql_tracker_direct_p.h"
#include "qsparql_tracker_direct_driver_p.h"

#include <QtSparql/qsparqlerror.h>
#include <QtSparql/qsparqlbinding.h>
#include <QtSparql/qsparqlquery.h>
#include <QtSparql/qsparqlqueryoptions.h>
#include <QtSparql/qsparqlresultrow.h>
#define XSD_INTEGER
#include "../../kernel/qsparqlxsd_p.h"

#include <QtCore/qvariant.h>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

QTrackerDirectSyncResult::QTrackerDirectSyncResult(QTrackerDirectDriverPrivate* p,
                                                   const QString& query,
                                                   QSparqlQuery::StatementType type,
                                                   const QSparqlQueryOptions& options)
    : cursor(0), n_columns(-1), options(&options)
{
    setQuery(query);
    setStatementType(type);
    driverPrivate = p;
}

QTrackerDirectSyncResult::~QTrackerDirectSyncResult()
{
    stopAndWait();
}

void QTrackerDirectSyncResult::exec()
{
    if (statementType() == QSparqlQuery::AskStatement || statementType() == QSparqlQuery::SelectStatement) {
        selectQuery();
    } else if (statementType() == QSparqlQuery::InsertStatement || statementType() == QSparqlQuery::DeleteStatement) {
        updateQuery();
    }
}

void QTrackerDirectSyncResult::selectQuery()
{
    if (!driverPrivate->driver->isOpen()) {
        setLastError(QSparqlError(driverPrivate->error,
                                  QSparqlError::ConnectionError));
        return;
    }

    GError * error = 0;
    cursor = tracker_sparql_connection_query(driverPrivate->connection, query().toUtf8().constData(), 0, &error);
    if (error || !cursor) {
        setLastError(QSparqlError(QString::fromUtf8(error ? error->message : "unknown error"),
                        error ? errorCodeToType(error->code) : QSparqlError::StatementError,
                        error ? error->code : -1));
        if (error)
            g_error_free(error);
        qWarning() << "QTrackerDirectSyncResult:" << lastError() << query();
    }
}

void QTrackerDirectSyncResult::updateQuery()
{
    if (!driverPrivate->driver->isOpen()) {
        setLastError(QSparqlError(driverPrivate->error,
                                  QSparqlError::ConnectionError));
        return;
    }

    GError * error = 0;
    tracker_sparql_connection_update(driverPrivate->connection,
                                     query().toUtf8().constData(),
                                     qSparqlPriorityToGlib(options->priority()),
                                     0,
                                     &error);
    if (error) {
        setLastError(QSparqlError(QString::fromUtf8(error->message),
                        errorCodeToType(error->code),
                        error->code));
        g_error_free(error);
        qWarning() << "QTrackerDirectSyncResult:" << lastError() << query();
    }
}

bool QTrackerDirectSyncResult::next()
{
    if (!cursor) {
        // The cursor may have been unreferenced because the connection was deleted
        // and now the user is calling next(), so set the row here
        updatePos(QSparql::AfterLastRow);
        return false;
    }

    GError * error = 0;
    const gboolean active = tracker_sparql_cursor_next(cursor, 0, &error);

    // if this is an ask query, get the result
    if (isBool() && active && tracker_sparql_cursor_get_value_type(cursor, 0) == TRACKER_SPARQL_VALUE_TYPE_BOOLEAN) {
        const gboolean value = tracker_sparql_cursor_get_boolean(cursor, 0);
        setBoolValue(value != FALSE);
    }

    if (error) {
        setLastError(QSparqlError(QString::fromUtf8(error->message),
                       errorCodeToType(error->code),
                       error->code));
        g_error_free(error);
        qWarning() << "QTrackerDirectSyncResult:" << lastError() << query();
        g_object_unref(cursor);
        cursor = 0;
        return false;
    }

    if (!active) {
        g_object_unref(cursor);
        cursor = 0;
        updatePos(QSparql::AfterLastRow);
        return false;
    }
    const int oldPos = pos();
    if (oldPos == QSparql::BeforeFirstRow)
        updatePos(0);
    else
        updatePos(oldPos + 1);
    return true;
}

QSparqlResultRow QTrackerDirectSyncResult::current() const
{
    // Note: this function reads and constructs the data again every time it's called.
    if (!cursor || pos() == QSparql::BeforeFirstRow || pos() == QSparql::AfterLastRow)
        return QSparqlResultRow();

    QSparqlResultRow resultRow;
    // get the no. of columns only once; it won't change between rows
    if (n_columns < 0)
        n_columns = tracker_sparql_cursor_get_n_columns(cursor);

    for (int i = 0; i < n_columns; i++) {
        resultRow.append(binding(i));
    }
    return resultRow;
}

QSparqlBinding QTrackerDirectSyncResult::binding(int i) const
{
    // Note: this function reads and constructs the data again every time it's called.
    if (!cursor || pos() == QSparql::BeforeFirstRow || pos() == QSparql::AfterLastRow)
        return QSparqlBinding();

    // get the no. of columns only once; it won't change between rows
    if (n_columns < 0)
        n_columns = tracker_sparql_cursor_get_n_columns(cursor);

    if (i < 0 || i >= n_columns)
        return QSparqlBinding();

    const gchar* name = tracker_sparql_cursor_get_variable_name(cursor, i);
    const QVariant& value = readVariant(cursor, i);

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
    if (!cursor || pos() == QSparql::BeforeFirstRow || pos() == QSparql::AfterLastRow)
        return QVariant();

    // get the no. of columns only once; it won't change between rows
    if (n_columns < 0)
        n_columns = tracker_sparql_cursor_get_n_columns(cursor);

    if (i < 0 || i >= n_columns)
        return QVariant();

    return readVariant(cursor, i);
}

QString QTrackerDirectSyncResult::stringValue(int i) const
{
    if (!cursor || pos() == QSparql::BeforeFirstRow || pos() == QSparql::AfterLastRow)
        return QString();

    // get the no. of columns only once; it won't change between rows
    if (n_columns < 0)
        n_columns = tracker_sparql_cursor_get_n_columns(cursor);

    if (i < 0 || i >= n_columns)
        return QString();

    return QString::fromUtf8(tracker_sparql_cursor_get_string(cursor, i, 0));
}

void QTrackerDirectSyncResult::stopAndWait()
{
    if (cursor)
        g_object_unref(cursor);
    cursor = 0;
}

bool QTrackerDirectSyncResult::isFinished() const
{
    return !cursor;
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

QT_END_NAMESPACE
