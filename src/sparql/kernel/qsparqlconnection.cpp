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

#include "qsparqlconnection.h"
#include "qsparqlconnection_p.h"
#include "qsparqlquery.h"

#ifdef QT_SPARQL_VIRTUOSO
#include "../drivers/virtuoso/qsparql_virtuoso_p.h"
#endif
#ifdef QT_SPARQL_TRACKER
#include "../drivers/tracker/qsparql_tracker_p.h"
#endif
#ifdef QT_SPARQL_TRACKER_DIRECT
#include "../drivers/tracker_direct/qsparql_tracker_direct_p.h"
#endif
#ifdef QT_SPARQL_ENDPOINT
#include "../drivers/endpoint/qsparql_endpoint_p.h"
#endif

#include "qdebug.h"
#include "qcoreapplication.h"
#include "qsparqlresult.h"
#include "qsparqlconnectionoptions.h"
#include "qsparqldriver_p.h"
#include "qsparqldriverplugin_p.h"
#if WE_ARE_QT
// QFactoryLoader is an internal part of Qt; we'll use it when where part of Qt
// (or when Qt publishes it.)
# include "qfactoryloader_p.h"
#else
# include "qdir.h"
# include "qpluginloader.h"
#endif
#include "qsparqlnulldriver_p.h"

#include <QtCore/qhash.h>
#include <QtCore/quuid.h>
#include <QtCore/qmutex.h>

QT_BEGIN_NAMESPACE

#if WE_ARE_QT

#if !defined(QT_NO_LIBRARY) && !defined(QT_NO_SETTINGS)
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
                          (QSparqlDriverFactoryInterface_iid,
                           QLatin1String("/sparqldrivers")))
#endif
#endif

typedef QHash<QString, QSparqlDriverCreatorBase*> DriverDict;

class QSparqlConnectionPrivate
{
public:
    QSparqlConnectionPrivate(QSparqlDriver *dr,
                             const QString& name,
                             const QSparqlConnectionOptions& opts):
        driver(dr),
        drvName(name),
        options(opts)
    {
    }
    ~QSparqlConnectionPrivate();

    static DriverDict &driverDict();
    static QSparqlDriver* findDriver(const QString& type);
    static QSparqlDriver* findDriverWithFactoryLoader(const QString& type);
    static void initKeys();
    static QSparqlDriver* findDriverWithPluginLoader(const QString& type);
    static void registerConnectionCreator(const QString &type,
                                          QSparqlDriverCreatorBase* creator);

    static QSparqlConnectionPrivate* shared_null();
    QSparqlResult* checkErrors(const QString& queryText) const;

    static QStringList allKeys;
    static QHash<QString, QSparqlDriverPlugin*> plugins;
    static QMutex pluginMutex; // protexts allKeys, plugins and driverDict

    QSparqlDriver* driver;
    QString drvName;
    QSparqlConnectionOptions options;
};

QSparqlConnectionPrivate *QSparqlConnectionPrivate::shared_null()
{
    static QSparqlNullDriver dr;
    static QSparqlConnectionPrivate n(&dr, QString(),
                                      QSparqlConnectionOptions());
    return &n;
}

QSparqlConnectionPrivate::~QSparqlConnectionPrivate()
{
    if (driver != shared_null()->driver) {
        delete driver;
        driver = shared_null()->driver;
    }
}

static bool qDriverDictInit = false;
static void cleanDriverDict()
{
    qDeleteAll(QSparqlConnectionPrivate::driverDict());
    QSparqlConnectionPrivate::driverDict().clear();
    qDriverDictInit = false;
}

DriverDict &QSparqlConnectionPrivate::driverDict()
{
    static DriverDict dict;
    if (!qDriverDictInit) {
        qDriverDictInit = true;
        qAddPostRoutine(cleanDriverDict);
    }
    return dict;
}
/*!
    \internal

    Create the actual driver instance \a type.
*/

QSparqlDriver* QSparqlConnectionPrivate::findDriver(const QString &type)
{
    QMutexLocker locker(&pluginMutex);
    // separately defined drivers (e.g., for tests)
    QSparqlDriver * driver = 0;
    DriverDict dict = QSparqlConnectionPrivate::driverDict();
    for (DriverDict::const_iterator it = dict.constBegin();
         it != dict.constEnd() && !driver; ++it) {
        if (type == it.key()) {
            driver = ((QSparqlDriverCreatorBase*)(*it))->createObject();
        }
    }
    if (driver)
        return driver;

    // drivers built into the .so
#ifdef QT_SPARQL_TRACKER
    if (type == QLatin1String("QTRACKER"))
        driver = new QTrackerDriver();
#endif
#ifdef QT_SPARQL_TRACKER_DIRECT
    if (type == QLatin1String("QTRACKER_DIRECT"))
        driver = new QTrackerDirectDriver();
#endif
#ifdef QT_SPARQL_ENDPOINT
    if (type == QLatin1String("QSPARQL_ENDPOINT"))
        driver = new EndpointDriver();
#endif
#ifdef QT_SPARQL_VIRTUOSO
    if (type == QLatin1String("QVIRTUOSO"))
        driver = new QVirtuosoDriver();
#endif
    if (driver)
        return driver;

    // drivers built as plugins
#if WE_ARE_QT
    return findDriverWithFactoryLoader(type);
#else
    return findDriverWithPluginLoader(type);
#endif
}

QSparqlDriver* QSparqlConnectionPrivate::findDriverWithFactoryLoader(const QString &type)
{
#if WE_ARE_QT
#if !defined(QT_NO_LIBRARY) && !defined(QT_NO_SETTINGS)
    if (!driver && loader()) {
        if (QSparqlDriverFactoryInterface *factory = qobject_cast<QSparqlDriverFactoryInterface*>(loader()->instance(type)))
            driver = factory->create(type);
    }
#endif // QT_NO_LIBRARY

    if (!driver) {
        qWarning("QSparqlConnection: %s driver not loaded", type.toLatin1().data());
        qWarning("QSparqlConnection: available drivers: %s",
                        QSparqlConnection::drivers().join(QLatin1String(" ")).toLatin1().data());
        if (QCoreApplication::instance() == 0)
            qWarning("QSparqlConnectionPrivate: an instance of QCoreApplication is required for loading driver plugins");
        driver = QSparqlConnectionPrivate::shared_null()->driver;
    }
    return driver;
#else
    return 0;
#endif // WE_ARE_QT
}

QStringList QSparqlConnectionPrivate::allKeys;
QHash<QString, QSparqlDriverPlugin*> QSparqlConnectionPrivate::plugins;
QMutex QSparqlConnectionPrivate::pluginMutex(QMutex::Recursive);

void QSparqlConnectionPrivate::initKeys()
{
    static bool keysRead = false;

    if (keysRead)
        return;

    int debugLevel = QString::fromLatin1(getenv("QT_DEBUG_PLUGINS")).toInt();

    QStringList paths = QCoreApplication::libraryPaths();
    foreach(const QString& path, paths) {
        QString realPath = path + QLatin1String("/sparqldrivers");
        QStringList pluginNames = QDir(realPath).entryList(QDir::Files);
        for (int j = 0; j < pluginNames.count(); ++j) {
            QString fileName = QDir::cleanPath(realPath +
                                                QLatin1Char('/') +
                                                pluginNames.at(j));
            if (debugLevel) {
                qDebug() << "QSparqlConnection looking at" << fileName;
            }

            QPluginLoader loader(fileName);
            QObject* instance = loader.instance();
            QFactoryInterface *factory = qobject_cast<QFactoryInterface*>(instance);
            QSparqlDriverPlugin* driPlu = dynamic_cast<QSparqlDriverPlugin*>(factory);
            if (instance && factory && driPlu) {
                QStringList keys = factory->keys();
                for (int k = 0; k < keys.size(); ++k) {
                    // Don't override values in plugins; this prefers plugins
                    // that are found first.  E.g.,
                    // QCoreApplication::addLibraryPath() prepends a path to the
                    // list of library paths, and this say custom plugins are
                    // found first.
                    if (!plugins.contains(keys[k]))
                        plugins[keys[k]] = driPlu;
                }
                allKeys.append(keys);
                if (debugLevel) {
                    qDebug() << "keys" << keys;
                }
            }
            else {
                if (debugLevel) {
                    qDebug() << "not a plugin";
                }
            }

        }
    }

    keysRead = true;
    return;
}

QSparqlDriver* QSparqlConnectionPrivate::findDriverWithPluginLoader(const QString &type)
{
#if WE_ARE_QT
    return 0;
#else
    initKeys();

    if (plugins.contains(type))
        return plugins[type]->create(type);
    return QSparqlConnectionPrivate::shared_null()->driver;
#endif
}


void qSparqlRegisterConnectionCreator(const QString& type,
                               QSparqlDriverCreatorBase* creator)
{
    QSparqlConnectionPrivate::registerConnectionCreator(type, creator);
}

void QSparqlConnectionPrivate::registerConnectionCreator(const QString& name,
                                                         QSparqlDriverCreatorBase* creator)
{
    delete QSparqlConnectionPrivate::driverDict().take(name);
    if (creator)
        QSparqlConnectionPrivate::driverDict().insert(name, creator);
}

/*!

  \class QSparqlConnection

  \brief The QSparqlConnection class provides an interface for accessing an RDF store.
    \inmodule QtSparql

*/

/*!
    Constructs and invalid QSparqlConnection.
*/
QSparqlConnection::QSparqlConnection(QObject* parent)
    : QObject(parent)
{
    QSparqlDriver* driver = QSparqlConnectionPrivate::shared_null()->driver;
    d = new QSparqlConnectionPrivate(driver, QString(),
                                     QSparqlConnectionOptions());
}

/*!

Constructs a QSparqlConnection of the given \a type with the given \a options.
The \a type is a string which identifies the driver.  To get a list of available
drivers, use drivers().  The accepted connection options depend on the selected
driver.  The drivers ignore unneeded connection options.

*/
QSparqlConnection::QSparqlConnection(const QString& type,
                                     const QSparqlConnectionOptions& options,
                                     QObject* parent)
    : QObject(parent)
{
    QSparqlDriver* driver = QSparqlConnectionPrivate::findDriver(type);
    d = new QSparqlConnectionPrivate(driver, type, options);
    d->driver->open(d->options);
}

/*!
    Destroys the QSparqlConnection object and frees up any
    resources. Note that QSparqlResult objects that are returned from
    this class have this object set as their parents, which means that
    they will be deleted along with it if you don't call
    QObject::setParent() on them.
*/
QSparqlConnection::~QSparqlConnection()
{
    QList<QSparqlResult*> children = findChildren<QSparqlResult *>();
    foreach (QSparqlResult *result, children) {
        if (!result->isFinished())
            qWarning() << "QSparqlConnection: Deleting active query:" <<
                result->query();
    }

    qDeleteAll(children);
    d->driver->close();
    delete d;
}

/*!
    Executes a SPARQL query on the database and returns a pointer
    to a QSparqlResult object. The user is responsible for
    freeing it when it's no longer used (but not after the
    QSparqlConnection is deleted). The QSparqlResult object is
    also a child of the QSparqlConnection, so after the
    QSparqlConnection has been deleted, the QSparqlResult is no
    longer valid (and doesn't need to be freed).

    If \a query is empty or if the this QSparqlConnection is not
    valid, exec() returns a QSparqlResult which is in the error
    state. It won't emit the finished() signal.

    If this function fails with "connection not open" error, the
    most probable reason is that the required driver is not
    installed.

    \sa QSparqlQuery, QSparqlResult, QSparqlResult::hasError
*/

/// Returns a QSparqlNullResult with an appropriate error if executing a query
/// would fail immediately (e.g., connection is not open). Otherwise returns 0.
QSparqlResult* QSparqlConnectionPrivate::checkErrors(const QString& queryText) const
{
    QSparqlResult* result = 0;
    if (driver->isOpenError()) {
        qWarning("QSparqlConnection::exec: connection not open");

        result = new QSparqlNullResult();
        result->setLastError(driver->lastError());
    } else if (!driver->isOpen()) {
        qWarning("QSparqlConnection::exec: connection not open");

        result = new QSparqlNullResult();
        result->setLastError(QSparqlError(QLatin1String("Connection not open"),
                                          QSparqlError::ConnectionError));
    } else {
        if (queryText.isEmpty()) {
            qWarning("QSparqlConnection::exec: empty query");
            result = new QSparqlNullResult();
            result->setLastError(QSparqlError(QLatin1String("Query is empty"),
                                            QSparqlError::ConnectionError));
        }
    }
    return result;
}

// TODO: isn't it quite bad that the user must check the error
// state of the result? Or should the "error result" emit the
// finished() signal when the main loop is entered the next time,
// so that the user has a change to connect to it?

QSparqlResult* QSparqlConnection::exec(const QSparqlQuery& query)
{
    QString queryText = query.preparedQueryText();
    QSparqlResult* result = d->checkErrors(queryText);
    if (!result) {
        // No error. FIXME: it's evil to return a 0 pointer to indicate "no
        // error".
        result = d->driver->exec(queryText, query.type());
    }
    result->setParent(this);
    return result;
}

QSparqlResult* QSparqlConnection::syncExec(const QSparqlQuery& query)
{
    QString queryText = query.preparedQueryText();
    QSparqlResult* result = d->checkErrors(queryText);
    if (!result) {
        // No error. FIXME: it's evil to return a 0 pointer to indicate "no
        // error".
        if (d->driver->hasFeature(QSparqlConnection::SyncExec)) {
            result = d->driver->syncExec(queryText, query.type());
        }
        else {
            // TODO: insert a wrapper here
            result = new QSparqlNullResult();
            result->setLastError(
                QSparqlError(QLatin1String("SyncWrapper not implemented yet"),
                             QSparqlError::ConnectionError));
        }
    }
    result->setParent(this);
    return result;
}

/*!
    Returns the connection's driver name.
*/
QString QSparqlConnection::driverName() const
{
    return d->drvName;
}

/*!
    \enum QSparqlConnection::Feature

    This enum contains a list of features a driver might support. Use
    hasFeature() to query whether a feature is supported or not.

    \value QuerySize  Whether the database is capable of reporting the size
    of a query. Note that some databases do not support returning the size
    (i.e. number of rows returned) of a query, in which case
    QSparqlQuery::size() will return -1.
    
    \value DefaultGraph  The store has a default graph which doesn't have
    to be specified. Some stores, like Virtuoso, don't have a default graph.
    
    \value AskQueries The driver supports ASK queries
    
    \value ConstructQueries The driver supports CONSTRUCT queries
    
    \value UpdateQueries The driver supports INSERT and UPDATE queries

    \sa hasFeature()
*/

/*!
    Returns true if the QSparqlConnection supports feature \a feature;
    otherwise returns false.
*/
bool QSparqlConnection::hasFeature(Feature feature) const
{
    return d->driver->hasFeature(feature);
}

/*!
    Returns true if the QSparqlConnection has a valid driver, i.e.,
    the name of the driver given in the constructor was valid.

    Example:
    \snippet doc/src/snippets/code/src_sparql_kernel_qsparqlconnection.cpp 8
*/
bool QSparqlConnection::isValid() const
{
    return d->driver && d->driver != d->shared_null()->driver;
}

/*!
    Adds a prefix/uri pair to the connection. Each SPARQL query made
    with the connection will have the prefixes prepended to it.
*/

void QSparqlConnection::addPrefix(const QString& prefix, const QUrl& uri)
{
    d->driver->addPrefix(prefix, uri);
}

/*!
    Removes any prefix/uri pairs which have been added to the connection.
*/

void QSparqlConnection::clearPrefixes()
{
    d->driver->clearPrefixes();
}

/*!
    Creates a new Urn based Uri
*/

QUrl QSparqlConnection::createUrn() const
{
    QByteArray uuid = QUuid::createUuid().toString().toLatin1();
    QByteArray urn = "urn:uuid:" + uuid.mid(1, uuid.size() - 2);
    return QUrl::fromEncoded(urn);
}

/*!
    Creates a Urn for use in an insert query. The given name can be
    used to substitute the value into the query string.
*/

QSparqlBinding QSparqlConnection::createUrn(const QString& name) const
{
    return QSparqlBinding(name, createUrn());
}

/*!
     Returns the list of available drivers.  The list contains driver names
     which can be passed to QSparqlConnection constructor.
*/
QStringList QSparqlConnection::drivers()
{
    QMutexLocker locker(&(QSparqlConnectionPrivate::pluginMutex));
    QStringList list;

#ifdef QT_SPARQL_VIRTUOSO
    list << QLatin1String("QVIRTUOSO");
#endif
#ifdef QT_SPARQL_TRACKER
    list << QLatin1String("QTRACKER");
#endif
#ifdef QT_SPARQL_TRACKER_DIRECT
    list << QLatin1String("QTRACKER_DIRECT");
#endif
#ifdef QT_SPARQL_ENDPOINT
    list << QLatin1String("QSPARQL_ENDPOINT");
#endif

#if WE_ARE_QT
#if !defined(QT_NO_LIBRARY) && !defined(QT_NO_SETTINGS)
    if (QFactoryLoader *fl = loader()) {
        QStringList keys = fl->keys();
        for (QStringList::const_iterator i = keys.constBegin(); i != keys.constEnd(); ++i) {
            if (!list.contains(*i))
                list << *i;
        }
    }
#endif

#else
    QSparqlConnectionPrivate::initKeys();
    QStringList keys = QSparqlConnectionPrivate::allKeys;
    for (QStringList::const_iterator i = keys.constBegin(); i != keys.constEnd(); ++i) {
        if (!list.contains(*i))
            list << *i;
    }
#endif

    DriverDict dict = QSparqlConnectionPrivate::driverDict();
    for (DriverDict::const_iterator i = dict.constBegin(); i != dict.constEnd(); ++i) {
        if (!list.contains(i.key()))
            list << i.key();
    }

    return list;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QSparqlConnection &d)
{
    if (!d.isValid()) {
        dbg.nospace() << "QSparqlConnection(invalid)";
        return dbg.space();
    }

    dbg.nospace() << "QSparqlConnection(driver=\"" << d.driverName() << ")";
    return dbg.space();
}
#endif

QT_END_NAMESPACE
