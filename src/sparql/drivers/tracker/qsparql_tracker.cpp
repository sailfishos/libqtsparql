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

// #include "qsparql_tracker.h"
#include "qsparql_tracker_p.h"

#include <qsparqlerror.h>
#include <qsparqlbinding.h>
#include <qsparqlquery.h>
#include <qsparqlqueryoptions.h>
#include <qsparqlresultrow.h>

#include <qcoreapplication.h>
#include <qvariant.h>
#include <qstringlist.h>
#include <qvector.h>
#include <qstring.h>
#include <qregexp.h>

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

// This enum is defined in tracker-sparql.h, but copy it here for now
// to avoid a dependency on that header.
typedef enum  {
    TRACKER_SPARQL_ERROR_PARSE,
    TRACKER_SPARQL_ERROR_UNKNOWN_CLASS,
    TRACKER_SPARQL_ERROR_UNKNOWN_PROPERTY,
    TRACKER_SPARQL_ERROR_TYPE,
    TRACKER_SPARQL_ERROR_CONSTRAINT,
    TRACKER_SPARQL_ERROR_NO_SPACE,
    TRACKER_SPARQL_ERROR_INTERNAL,
    TRACKER_SPARQL_ERROR_UNSUPPORTED
} TrackerSparqlError;

class QTrackerDriverPrivate {
public:
    QTrackerDriverPrivate();
    ~QTrackerDriverPrivate();
    QDBusInterface* iface;
    bool doBatch; // true: call BatchSparqlUpdate on Tracker instead of
                  // SparqlUpdateBlank
};

class QTrackerResultPrivate : public QObject {
    Q_OBJECT
public:
    QTrackerResultPrivate(QTrackerResult* res,
                          QTrackerDriverPrivate* dp);

    ~QTrackerResultPrivate();
    QDBusPendingCallWatcher* watcher;
    QVector<QStringList> data;
    QTrackerDriverPrivate* driverPrivate;
    void setCall(QDBusPendingCall& call);
    static TrackerSparqlError errorNameToCode(const QString& name);
    static QSparqlError::ErrorType errorCodeToType(TrackerSparqlError code);

private Q_SLOTS:
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

} // end of unnamed namespace

QTrackerResultPrivate::QTrackerResultPrivate(QTrackerResult* res,
                                             QTrackerDriverPrivate* dp)
: watcher(0), driverPrivate(dp), q(res)
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

TrackerSparqlError QTrackerResultPrivate::errorNameToCode(const QString& name)
{
    if (name == QLatin1String("org.freedesktop.Tracker1.SparqlError.Parse")) {
        return TRACKER_SPARQL_ERROR_PARSE;
    } else if (name == QLatin1String("org.freedesktop.Tracker1.SparqlError.UnknownClass")) {
        return TRACKER_SPARQL_ERROR_UNKNOWN_CLASS;
    } else if (name == QLatin1String("org.freedesktop.Tracker1.SparqlError.UnknownProperty")) {
        return TRACKER_SPARQL_ERROR_UNKNOWN_PROPERTY;
    } else if (name == QLatin1String("org.freedesktop.Tracker1.SparqlError.Type")) {
        return TRACKER_SPARQL_ERROR_TYPE;
    } else if (name == QLatin1String("org.freedesktop.Tracker1.SparqlError.Constraint")) {
        return TRACKER_SPARQL_ERROR_CONSTRAINT;
    } else if (name == QLatin1String("org.freedesktop.Tracker1.SparqlError.NoSpace")) {
        return TRACKER_SPARQL_ERROR_NO_SPACE;
    } else if (name == QLatin1String("org.freedesktop.Tracker1.SparqlError.Internal")) {
        return TRACKER_SPARQL_ERROR_INTERNAL;
    } else if (name == QLatin1String("org.freedesktop.Tracker1.SparqlError.Unsupported")) {
        return TRACKER_SPARQL_ERROR_UNSUPPORTED;
    } else {
        return static_cast<TrackerSparqlError>(-1);
    }
}

QSparqlError::ErrorType QTrackerResultPrivate::errorCodeToType(TrackerSparqlError code)
{
    switch (code) {
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

void QTrackerResultPrivate::onDBusCallFinished()
{
    if (watcher->isError()) {
        QSparqlError error(watcher->error().message());
        if (watcher->error().type() == QDBusError::Other) {
            TrackerSparqlError code = errorNameToCode(watcher->error().name());
            error.setNumber(code);
            error.setType(errorCodeToType(code));
        } else {
            // Error from D-Bus
            error.setNumber((int)watcher->error().type());
            error.setType(QSparqlError::ConnectionError);
        }

        q->setLastError(error);
        qWarning() << "QTrackerResult:" << q->lastError() << q->query();
        Q_EMIT q->finished();
        return;
    }

    switch (q->statementType()) {
    case QSparqlQuery::AskStatement:
    case QSparqlQuery::SelectStatement:
    {
        QDBusPendingReply<QVector<QStringList> > reply = *watcher;
        data = reply.argumentAt<0>();

        if (q->statementType() == QSparqlQuery::AskStatement && data.count() == 1 && data[0].count() == 1)
        {
            QVariant boolValue = data[0][0];
            q->setBoolValue(boolValue.toBool());
        }

        Q_EMIT q->dataReady(data.size());
        break;
    }
    default:
        // TODO: handle update results here
        break;
    }
    Q_EMIT q->finished();
}

QTrackerResult::QTrackerResult(const QString& query, QSparqlQuery::StatementType tp, QTrackerDriver* driver)
{
    setQuery(query);
    setStatementType(tp);
    d = new QTrackerResultPrivate(this, driver->d);
    connect(driver, SIGNAL(closing()), this, SLOT(driverClosing()), Qt::DirectConnection);
}

QTrackerResult::~QTrackerResult()
{
    delete d;
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
    Q_FOREACH (const QString& str, resultStrings) {
        // This only creates a binding with the values but with empty column
        // names.
        // TODO: how to add column names?
        QSparqlBinding b(QString(), str);
        info.append(b);
    }
    return info;
}

bool QTrackerResult::hasFeature(QSparqlResult::Feature feature) const
{
    switch (feature) {
        case QSparqlResult::Sync:
        case QSparqlResult::ForwardOnly:
            return false;
        case QSparqlResult::QuerySize:
            return true;
        default:
            return false;
    }
}

void QTrackerResult::exec(const QSparqlQueryOptions& options)
{
    QString funcToCall;
    bool fireAndForget = false;
    switch (statementType()) {
    case QSparqlQuery::AskStatement:
    case QSparqlQuery::SelectStatement:
    {
        funcToCall = QString::fromLatin1("SparqlQuery");
        break;
    }
    case QSparqlQuery::InsertStatement: // fall-through
    case QSparqlQuery::DeleteStatement:
    {
        // only check this here since it's only useful for
        // update queries
        if (options.isFireAndForget()) {
            fireAndForget = true;
        }

        if (d->driverPrivate->doBatch || options.priority() == QSparqlQueryOptions::LowPriority) {
            funcToCall = QString::fromLatin1("BatchSparqlUpdate");
        }
        else {
            funcToCall = QString::fromLatin1("SparqlUpdateBlank");
        }
        break;
    }
    default:
        setLastError(QSparqlError(
                QLatin1String("Non-supported statement type"),
                QSparqlError::BackendError));
        qWarning() << "QTrackerResult:" << lastError() << query();
        return;
    }
    QDBusPendingCall call = d->driverPrivate->iface->asyncCall(funcToCall,
                                                QVariant(query()));
    // if it's an insert or delete, and fireAndForget was set to true, don't
    // bother creating the watcher, by not doing this isFinished() will return
    // true
    if (!fireAndForget) {
        d->setCall(call);
    }
}

void QTrackerResult::driverClosing()
{
    if (!isFinished()) {
        setLastError(QSparqlError(
                QString::fromUtf8("QSparqlConnection closed before QSparqlResult"),
                QSparqlError::ConnectionError));
    }
    delete d->watcher;
    d->watcher = 0;
    qWarning() << "QTrackerResult: QSparqlConnection closed before QSparqlResult with query:" << query();
}

QTrackerDriverPrivate::QTrackerDriverPrivate()
    : iface(0),  doBatch(false)
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
    case QSparqlConnection::QuerySize:
    case QSparqlConnection::DefaultGraph:
    case QSparqlConnection::AskQueries:
    case QSparqlConnection::UpdateQueries:
    case QSparqlConnection::AsyncExec:
        return true;
    case QSparqlConnection::ConstructQueries:
    case QSparqlConnection::SyncExec:
        return false;
    default:
        return false;
    }
    return false;
}

bool QTrackerDriver::open(const QSparqlConnectionOptions& options)
{
    // This option has been removed from API documentation as it is replaced by
    // QSparqlQueryOption::LowPriority. The implemenation needs to be kept
    // for backward compatibility.
    QVariant batchOption = options.option(QString::fromLatin1("batch"));
    if (!batchOption.isNull()) {
        d->doBatch = batchOption.toBool();
    }

    if (isOpen())
        close();
    d->iface = new QDBusInterface(service, resourcesPath,
                                  resourcesInterface,
                                  QDBusConnection::sessionBus());

    setOpen(true);
    setOpenError(false);

    return true;
}

void QTrackerDriver::close()
{
    if (isOpen()) {
        Q_EMIT closing();
        delete d->iface;
        d->iface = 0;
        setOpen(false);
        setOpenError(false);
    }
}

QTrackerResult* QTrackerDriver::exec(const QString& query,
                          QSparqlQuery::StatementType type,
                          const QSparqlQueryOptions& options)
{
    if (options.executionMethod() == QSparqlQueryOptions::SyncExec)
        return 0;
    QString query_with_prefixes(query);
    query_with_prefixes.prepend(prefixes());
    QTrackerResult* res = new QTrackerResult(query_with_prefixes, type, this);
    res->exec(options);
    return res;
}

QT_END_NAMESPACE

#include "qsparql_tracker.moc"
