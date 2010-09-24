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

#include "qsparql_tracker.h"
#include "qsparql_tracker_signals.h"
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

Q_DECLARE_METATYPE(QTrackerChangeNotifier::Quad)
Q_DECLARE_METATYPE(QList<QTrackerChangeNotifier::Quad>)

QT_BEGIN_NAMESPACE

// How to recognize tracker
static QLatin1String service("org.freedesktop.Tracker1");
static QLatin1String basePath("/org/freedesktop/Tracker1");
static QLatin1String resourcesInterface("org.freedesktop.Tracker1.Resources");
static QLatin1String resourcesPath("/org/freedesktop/Tracker1/Resources");
static QLatin1String changedSignal("GraphUpdated");
static QLatin1String changedSignature("sa(iiii)a(iiii)");

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

QSparqlBinding QTrackerResult::binding(int field) const
{
    if (!isValid()) {
        return QSparqlBinding();
    }

    // The upper layer calls this function only when this Result is positioned
    // in a valid position, so we don't need to check that.
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

// We'll open only one D-Bus connection per thread; it will be shared between
// QTrackerDriver and QTrackerChangeNotifier.
QThreadStorage<QDBusConnection*> dbusTls;
// how many instances are using the same D-Bus connection (in this thread)
QThreadStorage<int*> dbusRefCountTls;
QAtomicInt connectionCounter = 0;

static QDBusConnection getConnection()
{
    // Create a separate D-Bus connection for each thread. Use
    // ClosingDBusConnection so that the bus gets disconnected when the thread
    // storage is deleted.
    if (!dbusTls.hasLocalData()) {
        QString id = QString::number(connectionCounter.fetchAndAddRelaxed(1))
            .prepend(QString::fromLatin1("qtsparql-dbus-"));
        dbusTls.setLocalData(new QDBusConnection
                             (QDBusConnection::connectToBus(QDBusConnection::SessionBus, id)));
        dbusRefCountTls.setLocalData(new int(0));
    }
    ++(*dbusRefCountTls.localData());
    return *dbusTls.localData();
}

static void dropConnection()
{
    --(*dbusRefCountTls.localData());
    if (*(dbusRefCountTls.localData()) == 0) {
        QDBusConnection::disconnectFromBus(dbusTls.localData()->name());
        dbusTls.setLocalData(0); // this deletes the connection
        dbusRefCountTls.setLocalData(0);
        // If this thread needs to reconnect, a QDBusConnection with a different
        // name will be created.
    }
}

bool QTrackerDriver::open(const QSparqlConnectionOptions& options)
{
    Q_UNUSED(options);

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

// D-Bus marshalling
QDBusArgument &operator<<(QDBusArgument &argument, const QTrackerChangeNotifier::Quad &t)
{
    argument.beginStructure();
    argument << t.graph << t.subject << t.predicate << t.object;
    argument.endStructure();
    return argument;
}

// D-Bus demarshalling
const QDBusArgument &operator>>(const QDBusArgument &argument, QTrackerChangeNotifier::Quad &t)
{
    argument.beginStructure();
    argument >> t.graph >> t.subject >> t.predicate >> t.object;
    argument.endStructure();
    return argument;
}

QTrackerChangeNotifier::QTrackerChangeNotifier(const QString& className,
                                               QObject* parent)
    : QObject(parent)
{
    qDBusRegisterMetaType<QTrackerChangeNotifier::Quad>();
    qDBusRegisterMetaType<QList<QTrackerChangeNotifier::Quad> >();

    d = new QTrackerChangeNotifierPrivate(className, getConnection(), this);
}

QTrackerChangeNotifier::~QTrackerChangeNotifier()
{
    dropConnection();
}

QTrackerChangeNotifierPrivate::QTrackerChangeNotifierPrivate(
    const QString& className,
    QDBusConnection c,
    QTrackerChangeNotifier* q)
    : QObject(q), q(q), className(className), connection(c)
{
    // Start listening to the actual signal
    bool ok = connection.connect(service, resourcesPath,
                                 resourcesInterface, changedSignal,
                                 QStringList() << className,
                                 changedSignature,
                                 this, SLOT(changed(QString,
                                                    QList<QTrackerChangeNotifier::Quad>,
                                                    QList<QTrackerChangeNotifier::Quad>)));

    if (!ok) {
        qWarning() << "Cannot connect to signal from Tracker";
    }
}

QString QTrackerChangeNotifier::watchedClass() const
{
    return d->className;
}

void QTrackerChangeNotifierPrivate::changed(QString,
                                            QList<QTrackerChangeNotifier::Quad> deleted,
                                            QList<QTrackerChangeNotifier::Quad> inserted)
{
    // D-Bus will filter based on the class name, so we only get relevant
    // signals here.
    emit q->changed(deleted, inserted);
}

QT_END_NAMESPACE
