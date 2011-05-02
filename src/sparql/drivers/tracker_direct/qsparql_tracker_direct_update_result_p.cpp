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

#include "qsparql_tracker_direct_update_result_p.h"
#include "qsparql_tracker_direct_driver_p.h"
#include "qsparql_tracker_direct_result_p.h"

#include <qsparqlerror.h>
#include <qsparqlbinding.h>
#include <qsparqlquery.h>
#include <qsparqlqueryoptions.h>
#include <qsparqlresultrow.h>

#include <QtCore/qcoreapplication.h>
#include <QtCore/qeventloop.h>

#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

////////////////////////////////////////////////////////////////////////////

class QTrackerDirectUpdateResultPrivate : public QObject {
    Q_OBJECT
public:
    QTrackerDirectUpdateResultPrivate(QTrackerDirectUpdateResult* result, QTrackerDirectDriverPrivate *dpp,
                                      const QSparqlQueryOptions& options);

    ~QTrackerDirectUpdateResultPrivate();
    void terminate();
    void setLastError(const QSparqlError& e);

    QAtomicInt isFinished;
    bool resultAlive; // whether the corresponding Result object is still alive
    QEventLoop *loop;

    QTrackerDirectUpdateResult* q;
    QTrackerDirectDriverPrivate *driverPrivate;
    QSparqlQueryOptions options;
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

    if (error) {
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

QTrackerDirectUpdateResultPrivate::QTrackerDirectUpdateResultPrivate(QTrackerDirectUpdateResult* result,
                                                                     QTrackerDirectDriverPrivate *dpp,
                                                                     const QSparqlQueryOptions& options)
  : resultAlive(true), loop(0),
  q(result), driverPrivate(dpp), options(options)
{
}

QTrackerDirectUpdateResultPrivate::~QTrackerDirectUpdateResultPrivate()
{
}

void QTrackerDirectUpdateResultPrivate::terminate()
{
    isFinished = 1;
    q->Q_EMIT finished();

    if (loop)
        loop->exit();
}

void QTrackerDirectUpdateResultPrivate::setLastError(const QSparqlError& e)
{
    q->setLastError(e);
}


////////////////////////////////////////////////////////////////////////////

QTrackerDirectUpdateResult::QTrackerDirectUpdateResult(QTrackerDirectDriverPrivate* p,
                                           const QString& query,
                                           QSparqlQuery::StatementType type,
                                           const QSparqlQueryOptions& options)
{
    setQuery(query);
    setStatementType(type);
    d = new QTrackerDirectUpdateResultPrivate(this, p, options);
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
                                            qSparqlPriorityToGlib(d->options.priority()),
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

QT_END_NAMESPACE

#include "qsparql_tracker_direct_update_result_p.moc"
