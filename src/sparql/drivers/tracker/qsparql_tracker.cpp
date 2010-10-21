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

// #include "qsparql_tracker.h"
#include "qsparql_tracker_p.h"

#include <qsparqlerror.h>
#include <qsparqlbinding.h>
#include <qsparqlquery.h>
#include <qsparqlresultrow.h>

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

class QTrackerDriver;

QT_BEGIN_NAMESPACE

class QTrackerDriverPrivate {
public:
    QTrackerDriverPrivate();
    ~QTrackerDriverPrivate();
    QDBusConnection connection;
    QDBusInterface* iface;
    bool doBatch; // true: call BatchSparqlUpdate on Tracker instead of
                  // SparqlUpdateBlank
};

class QTrackerResultPrivate : public QObject {
    Q_OBJECT
public:
    QTrackerResultPrivate(QTrackerResult* res,
                          QSparqlQuery::StatementType tp);

    ~QTrackerResultPrivate();
    QDBusPendingCallWatcher* watcher;
    QVector<QStringList> data;
    QSparqlQuery::StatementType type;
    void setCall(QDBusPendingCall& call);

private slots:
    void onDBusCallFinished();
private:
    QTrackerResult* q; // public part
};

namespace {

// How to recognize tracker
QLatin1String service("org.freedesktop.Tracker1");
QLatin1String basePath("/org/freedesktop/Tracker1");
QLatin1String resourcesInterface("org.freedesktop.Tracker1.Resources");
QLatin1String resourcesPath("/org/freedesktop/Tracker1/Resources");

/*

QDBusConnection and threading problems

A QDBusConnection can only be used from one thread, see e.g.,
http://bugreports.qt.nokia.com/browse/QTBUG-11413

We'd like QSparqlConnection instances to share the QDBusConnection if they
are on the same thread. We would also like the D-Bus connections to get closed
when all the QSparqlConnection instances using them are destroyed.

Approaches which use QObject::thread() as an ID fail: QObject::thread() returns
the memory address of a QThread, and they get recycled when using a QThreadPool.

Approaches which use QThreadStorage need to be robust enough to deal with the
possibility that QSparqlConnection is itself put into a QThreadStorage. The
destruction order of QThreadStorage instances is arbitrary.

*/

// ClosingDBusConnection is like QDBusConnection but it disconnects the D-Bus
// connection when it is destroyed. It has a refcount for counting how many
// objects are using it (in the current thread). When all objects are destoyed,
// the ref count will go to 0, and dropConnection will destroy the
// ClosingDBusConnection. The D-Bus connection is also disconnected then.
class ClosingDBusConnection : public QDBusConnection
{
public:
    ClosingDBusConnection(const QDBusConnection& c)
        : QDBusConnection(c), refCount(0)
        {
        }
    ~ClosingDBusConnection()
        {
            disconnectFromBus(name());
        }
    int refCount;
};

QThreadStorage<ClosingDBusConnection*> dbusTls;
QAtomicInt connectionCounter = 0;

QDBusConnection getConnection()
{
    // Create a separate D-Bus connection for each thread. Use
    // ClosingDBusConnection so that the bus gets disconnected when the thread
    // storage is deleted.
    if (!dbusTls.hasLocalData()) {
        QString id = QString::number(connectionCounter.fetchAndAddRelaxed(1))
            .prepend(QString::fromLatin1("qtsparql-dbus-"));
        dbusTls.setLocalData(new ClosingDBusConnection
                             (QDBusConnection::connectToBus(QDBusConnection::SessionBus, id)));
    }
    ++(dbusTls.localData()->refCount);
    return *dbusTls.localData();
}

void dropConnection()
{
    // Deal with a possibility that the QThreadStorages we use have already been
    // destroyed; in this case, do nothing. The D-Bus connection has already
    // been disconnected by ~ClosingDBusConnection.
    if (!dbusTls.hasLocalData())
        return;

    ClosingDBusConnection* conn = dbusTls.localData();
    --(conn->refCount);
    if (conn->refCount == 0) {
        dbusTls.setLocalData(0); // this deletes the connection
        // If this thread needs to reconnect, a QDBusConnection with a different
        // name will be created.
    }
}

} // end of unnamed namespace

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

    QString funcToCall;
    switch (type) {
    case QSparqlQuery::SelectStatement:
    {
        funcToCall = QString::fromLatin1("SparqlQuery");
        break;
    }
    case QSparqlQuery::InsertStatement: // fall-through
    case QSparqlQuery::DeleteStatement:
    {
        if (d->doBatch) {
            funcToCall = QString::fromLatin1("BatchSparqlUpdate");
        }
        else {
            funcToCall = QString::fromLatin1("SparqlUpdateBlank");
        }
        break;
    }
    default:
        qWarning() << "Tracker backend: non-supported statement";
        res->setLastError(QSparqlError(
                              QLatin1String("Non-supported statement type"),
                              QSparqlError::BackendError));
        return res;
        break;
    }
    QDBusPendingCall call = d->iface->asyncCall(funcToCall,
                                                QVariant(query));
    res->d->setCall(call);
    return res;
}

QSparqlBinding QTrackerResult::binding(int field) const
{
    if (!isValid()) {
        return QSparqlBinding();
    }

    int i = pos();
    if (field >= d->data[i].count() || field < 0) {
        qWarning() << "QTrackerResult::data: column" << field << "out of range";
        return QSparqlBinding();
    }

    QString name = QString::fromLatin1("$%1").arg(field + 1);
    return QSparqlBinding(name, QVariant(d->data[i][field]));
}

QVariant QTrackerResult::value(int field) const
{
    if (!isValid()) {
        return QVariant();
    }

    // The upper layer calls this function only when this Result is positioned
    // in a valid position, so we don't need to check that.
    int i = pos();
    if (field >= d->data[i].count() || field < 0) {
        qWarning() << "QTrackerResult::data: column" << field << "out of range";
        return QVariant();
    }
    return d->data[i][field];
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

int QTrackerResult::size() const
{
    return d->data.count();
}

QSparqlResultRow QTrackerResult::current() const
{
    if (!isValid()) {
        return QSparqlResultRow();
    }

    QSparqlResultRow info;
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
    : connection(QString::fromLatin1("")), iface(0),  doBatch(false)
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
    switch (f) {
    case QSparqlConnection::AskQueries:
        return false;
    case QSparqlConnection::ConstructQueries:
        return false;
    case QSparqlConnection::UpdateQueries:
        return false;
    case QSparqlConnection::DefaultGraph:
        return true;
    default:
        return false;
    }
    return false;
}

bool QTrackerDriver::open(const QSparqlConnectionOptions& options)
{
    QVariant batchOption = options.option(QString::fromLatin1("batch"));
    if (!batchOption.isNull()) {
        d->doBatch = batchOption.toBool();
    }

    if (isOpen())
        close();
    d->connection = getConnection();
    d->iface = new QDBusInterface(service, resourcesPath,
                                  resourcesInterface, d->connection);

    setOpen(true);
    setOpenError(false);

    return true;
}

void QTrackerDriver::close()
{
    if (isOpen()) {
        dropConnection();
        delete d->iface;
        d->iface = 0;
        setOpen(false);
        setOpenError(false);
    }
}

QT_END_NAMESPACE

#include "qsparql_tracker.moc"
