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

#include <tracker-sparql.h>

#include "qsparql_tracker_direct_update_result_p.h"
#include "qsparql_tracker_direct.h"
#include "qsparql_tracker_direct_driver_p.h"

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
    void startUpdate(const QString& query);
    Q_INVOKABLE void terminate();
    void setLastError(const QSparqlError& e);
    QString query;

public:
    enum State {
        Idle, Executing, Finished
    };
    State state;
    QEventLoop *loop;

    QTrackerDirectUpdateResult* q;
    QTrackerDirectDriverPrivate *driverPrivate;
    QSparqlQueryOptions options;
};

QTrackerDirectUpdateResultPrivate::QTrackerDirectUpdateResultPrivate(QTrackerDirectUpdateResult* result,
                                                                     QTrackerDirectDriverPrivate *dpp,
                                                                     const QSparqlQueryOptions& options)
  : state(Idle), loop(0),
  q(result), driverPrivate(dpp), options(options)
{

}

QTrackerDirectUpdateResultPrivate::~QTrackerDirectUpdateResultPrivate()
{
}

void QTrackerDirectUpdateResultPrivate::startUpdate(const QString& query)
{
    q->queryRunner->queue(driverPrivate->threadPool);
    state = QTrackerDirectUpdateResultPrivate::Executing;
}

void QTrackerDirectUpdateResultPrivate::terminate()
{
    state = Finished;
    q->Q_EMIT finished();
    q->disconnect(SIGNAL(finished()));
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
    stopAndWait();
    delete d;
}

void QTrackerDirectUpdateResult::exec()
{
    if (!d->driverPrivate)
        return;

    if (!d->driverPrivate->driver->isOpen()) {
        setLastError(QSparqlError(d->driverPrivate->error,
                                  QSparqlError::ConnectionError));
        d->terminate();
        return;
    }
    queryRunner->queue(d->driverPrivate->threadPool);
    d->state = QTrackerDirectUpdateResultPrivate::Executing;
}

void QTrackerDirectUpdateResult::run()
{
    if (d->driverPrivate) {
        GError * error = 0;
        tracker_sparql_connection_update(d->driverPrivate->connection,
                                         query().toUtf8().constData(),
                                         qSparqlPriorityToGlib(d->options.priority()),
                                         0,
                                         &error);

        if (error) {
            d->setLastError(QSparqlError(QString::fromUtf8(error->message),
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
    if (d->driverPrivate) {
        d->driverPrivate->waitForConnectionOpen();

        if (!d->driverPrivate->driver->isOpen()) {
            setLastError(QSparqlError(d->driverPrivate->error,
                                      QSparqlError::ConnectionError));
            d->terminate();
            return;
        }
        queryRunner->runOrWait();
        terminate();
    }
}

bool QTrackerDirectUpdateResult::isFinished() const
{
    return (d->state == QTrackerDirectUpdateResultPrivate::Finished);
}

void QTrackerDirectUpdateResult::terminate()
{
    d->state = QTrackerDirectUpdateResultPrivate::Finished;
    Q_EMIT finished();
    this->disconnect(SIGNAL(finished()));
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
    d->driverPrivate = 0;
    d->state = QTrackerDirectUpdateResultPrivate::Finished;
    delete queryRunner; queryRunner = 0;
}
QT_END_NAMESPACE

#include "qsparql_tracker_direct_update_result_p.moc"
