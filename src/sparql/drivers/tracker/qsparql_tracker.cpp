/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtSparql module of the Qt Toolkit.
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
** Nokia at qt-info@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qsparql_tracker.h"
#include "qsparql_tracker_p.h"

#include <qsparqlerror.h>
#include <qsparqlbinding.h>
#include <qsparqlquery.h>
#include <qsparqlbindingset.h>

#include <qcoreapplication.h>
#include <qvariant.h>
#include <qstringlist.h>
#include <qvector.h>

#include <qdebug.h>

Q_DECLARE_METATYPE(QVector<QStringList>)

/*
    Allows a metatype to be declared for a type containing commas.
    For example:
	Q_DECLARE_METATYPE_COMMA(QList<QPair<QByteArray,QByteArray> >)
*/
#define Q_DECLARE_METATYPE_COMMA(...)                                   \
    QT_BEGIN_NAMESPACE                                                  \
    template <>                                                         \
    struct QMetaTypeId< __VA_ARGS__ >                                   \
    {                                                                   \
        enum { Defined = 1 };                                           \
        static int qt_metatype_id()                                     \
        {                                                               \
            static QBasicAtomicInt metatype_id = Q_BASIC_ATOMIC_INITIALIZER(0); \
            if (!metatype_id)                                           \
                metatype_id = qRegisterMetaType< __VA_ARGS__ >( #__VA_ARGS__, \
                              reinterpret_cast< __VA_ARGS__ *>(quintptr(-1))); \
            return metatype_id;                                         \
        }                                                               \
    };                                                                  \
    QT_END_NAMESPACE

Q_DECLARE_METATYPE_COMMA(QMap<QString, QString>)
Q_DECLARE_METATYPE_COMMA(QVector<QMap<QString, QString> >)
Q_DECLARE_METATYPE_COMMA(QVector<QVector<QMap<QString, QString> > >)

QT_BEGIN_NAMESPACE

QTrackerResultPrivate::QTrackerResultPrivate(QTrackerResult* res,
                                             QSparqlQuery::StatementType tp)
: watcher(0), type(tp), q(res)
{
}

void QTrackerResultPrivate::setCall(QDBusPendingCall& call)
{
    // This function should be called only once
    watcher = new QDBusPendingCallWatcher(call);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(onDBusCallFinished()));
}

QTrackerResultPrivate::~QTrackerResultPrivate()
{
    delete watcher;
}

void QTrackerResultPrivate::onDBusCallFinished()
{
    if (watcher->isError()) {
        qWarning() << "Tracker driver error:" << watcher->error().message();

        QSparqlError error(watcher->error().message());
        if (watcher->error().type() == QDBusError::Other) {
            error.setType(QSparqlError::BackendError);
        }
        else {
            // Error from D-Bus
            error.setType(QSparqlError::ConnectionError);
        }
        error.setNumber((int)watcher->error().type());
        q->setLastError(error);
        emit q->finished();
        return;
    }
    switch (type) {
    case QSparqlQuery::SelectStatement:
    {
        QDBusPendingReply<QVector<QStringList> > reply = *watcher;
        data = reply.argumentAt<0>();
        emit q->dataReady(data.size());
        break;
    }
    default:
        // TODO: handle update results here
        break;
    }
    emit q->finished();
}

QTrackerResult::QTrackerResult(QSparqlQuery::StatementType tp)
{
    d = new QTrackerResultPrivate(this, tp);
}

QTrackerResult::~QTrackerResult()
{
    delete d;
}

QTrackerResult* QTrackerDriver::exec(const QString& query,
                          QSparqlQuery::StatementType type)
{
    QTrackerResult* res = new QTrackerResult(type);
    switch (type) {
    case QSparqlQuery::SelectStatement:
    {
        QDBusPendingCall call = d->iface->asyncCall(QString::fromLatin1("SparqlQuery"),
                                   QVariant(query));
        res->d->setCall(call);
        break;
    }
    case QSparqlQuery::InsertStatement: // fall-through
    case QSparqlQuery::DeleteStatement:
    {
        QDBusPendingCall call = d->iface->asyncCall(QString::fromLatin1("SparqlUpdateBlank"),
                                   QVariant(query));
        res->d->setCall(call);
        break;
    }
    default:
        qWarning() << "Tracker backend: non-supported statement";
        res->setLastError(QSparqlError(
                              QLatin1String("Non-supported statement type"),
                              QSparqlError::BackendError));
        break;
    }
    return res;
}

void QTrackerResult::cleanup()
{
    setPos(QSparql::BeforeFirstRow);
}

bool QTrackerResult::fetch(int i)
{
    if (i < 0) {
        setPos(QSparql::BeforeFirstRow);
        return false;
    }
    if (i >= d->data.size()) {
        setPos(QSparql::AfterLastRow);
        return false;
    }
    setPos(i);
    return true;
}

bool QTrackerResult::fetchLast()
{
    if (d->data.count() == 0)
        return false;
    setPos(d->data.count() - 1);
    return true;
}

bool QTrackerResult::fetchFirst()
{
    if (pos() == 0)
        return true;
    return fetch(0);
}

QVariant QTrackerResult::data(int field) const
{
    // The upper layer calls this function only when this Result is positioned
    // in a valid position, so we don't need to check that.
    int i = pos();
    if (field >= d->data[i].count() || field < 0) {
        qWarning() << "QTrackerResult::data: column" << field << "out of range";
        return QVariant();
    }

    return QVariant(d->data[i][field]);
}

void QTrackerResult::waitForFinished()
{
    if (d->watcher)
        d->watcher->waitForFinished();
}

bool QTrackerResult::isFinished() const
{
    if (d->watcher)
        return d->watcher->isFinished();
    return true;
}

bool QTrackerResult::isNull(int field) const
{
    Q_UNUSED(field);
    return false;
}

int QTrackerResult::size() const
{
    return d->data.count();
}

QSparqlBindingSet QTrackerResult::bindingSet() const
{
    QSparqlBindingSet info;
    if (pos() >= d->data.count() || pos() < 0)
        return info;

    QStringList resultStrings = d->data[pos()];
    foreach (const QString& str, resultStrings) {
        // This only creates a binding with the values but with empty column
        // names.
        // TODO: how to add column names?
        QSparqlBinding b(QString(), str);
        info.append(b);
    }
    return info;
}

QTrackerDriverPrivate::QTrackerDriverPrivate()
    : connection(QString::fromLatin1("")), iface(0)
{
}

QTrackerDriverPrivate::~QTrackerDriverPrivate()
{
    delete iface;
}

QTrackerDriver::QTrackerDriver(QObject* parent)
    : QSparqlDriver(parent)
{
    d = new QTrackerDriverPrivate();

    d->service = QString::fromLatin1("org.freedesktop.Tracker1");
    d->basePath = QString::fromLatin1("/org/freedesktop/Tracker1");

    d->searchInterface = d->service + QLatin1String(".Search");
    d->searchPath = d->basePath + QLatin1String("/Search");

    d->resourcesInterface = d->service + QLatin1String(".Resources");
    d->resourcesPath = d->basePath + QLatin1String("/Resources");

    d->resourcesClassInterface = d->service + QLatin1String(".Resources.Class");
    d->resourcesClassPath = d->basePath + QLatin1String("/Resources/Classes");

    qRegisterMetaType<QVector<QStringList> >();
    qDBusRegisterMetaType<QVector<QStringList> >();

    qRegisterMetaType<QMap<QString, QString> >();
    qRegisterMetaType<QVector<QMap<QString, QString> > >();
    qDBusRegisterMetaType<QVector<QMap<QString, QString> > >();

    /*
    if(!trackerBus().interface()->isServiceRegistered(service_g)
        && (trackerBus().interface()->startService(service_g)
            , !trackerBus().interface()->isServiceRegistered(service_g)))
        qCritical() << "cannot connect to org.freedesktop.Tracker1 service";
    */
}

QTrackerDriver::~QTrackerDriver()
{
    delete d;
}

bool QTrackerDriver::hasFeature(QSparqlConnection::Feature f) const
{
    switch(f) {
    case QSparqlConnection::QuerySize:
    case QSparqlConnection::BackwardsIteration:
        return true;
    default:
        return false;
    }
    // FIXME: decide features
}

QAtomicInt connectionCounter = 0;

bool QTrackerDriver::open(const QSparqlConnectionOptions& options)
{
    Q_UNUSED(options);

    if (isOpen())
        close();

    QString id = QString::number(connectionCounter.fetchAndAddRelaxed(1))
        .prepend(QString::fromLatin1("qsparql-dbus-"));
    d->connection = QDBusConnection::connectToBus(QDBusConnection::SessionBus,
                                                  id);
    d->iface = new QDBusInterface(d->service, d->resourcesPath,
                                  d->resourcesInterface, d->connection);

    setOpen(true);
    setOpenError(false);

    return true;
}

void QTrackerDriver::close()
{
    if (isOpen()) {
        d->connection.disconnectFromBus(d->connection.name());
        delete d->iface;
        d->iface = 0;
        setOpen(false);
        setOpenError(false);
    }
}

QString QTrackerDriver::formatValue(const QSparqlBinding &field, bool trimStrings) const
{
    Q_UNUSED(field);
    Q_UNUSED(trimStrings);
    // FIXME: how to deal with nulls?
    QString r;
    return r;
}

QT_END_NAMESPACE
