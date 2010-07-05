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

#include "qsparqlconnection.h"
#include "qsparqlconnection_p.h"
#include "qsparqlquery.h"

#ifdef QT_SPARQL_VIRTUOSO
#include "../drivers/virtuoso/qsparql_virtuoso.h"
#endif
#ifdef QT_SPARQL_TRACKER
#include "../drivers/tracker/qsparql_tracker.h"
#endif
#ifdef QT_SPARQL_ENDPOINT
#include "../drivers/endpoint/qsparql_endpoint.h"
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
#include "qhash.h"

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
    static QSparqlDriver* findDriverWithPluginLoader(const QString& type);
    static void registerConnectionCreator(const QString &type,
                                          QSparqlDriverCreatorBase* creator);

    static QSparqlConnectionPrivate* shared_null();

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
#if WE_ARE_QT
    return findDriverWithFactoryLoader(type);
#else
    return findDriverWithPluginLoader(type);
#endif
}

QSparqlDriver* QSparqlConnectionPrivate::findDriverWithFactoryLoader(const QString &type)
{
#if WE_ARE_QT
    QSparqlDriver * driver = 0;
    if (!driver) {
        DriverDict dict = QSparqlConnectionPrivate::driverDict();
        for (DriverDict::const_iterator it = dict.constBegin();
             it != dict.constEnd() && !driver; ++it) {
            if (type == it.key()) {
                driver = ((QSparqlDriverCreatorBase*)(*it))->createObject();
            }
        }
    }

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
#endif
}

QSparqlDriver* QSparqlConnectionPrivate::findDriverWithPluginLoader(const QString &type)
{
#if WE_ARE_QT
    return 0;
#else
    static bool keysRead = false;
    static QHash<QString, QSparqlDriverPlugin*> plugins;

    if (!keysRead) {
        keysRead = true;
        QStringList paths = QCoreApplication::libraryPaths();
        foreach(const QString& path, paths) {
            QString realPath = path + QLatin1String("/sparqldrivers");
            QStringList pluginNames = QDir(realPath).entryList(QDir::Files);
            for (int j = 0; j < pluginNames.count(); ++j) {
                QString fileName = QDir::cleanPath(realPath +
                                                   QLatin1Char('/') +
                                                   pluginNames.at(j));
                QPluginLoader loader(fileName);
                QObject* instance = loader.instance();
                QFactoryInterface *factory = qobject_cast<QFactoryInterface*>(instance);
                QSparqlDriverPlugin* driPlu = dynamic_cast<QSparqlDriverPlugin*>(factory);
                if (instance && factory && driPlu) {
                    QStringList keys = factory->keys();
                    for (int k = 0; k < keys.size(); ++k) {
                        plugins[keys[k]] = driPlu;
                    }
                }
            }
        }
    }
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
    Destroys the object and frees any allocated resources.

    If this is the last QSparqlConnection object that uses a certain
    database connection, the is automatically closed.

    \sa close()
*/

QSparqlConnection::QSparqlConnection(QObject* parent)
    : QObject(parent)
{
    QSparqlDriver* driver = QSparqlConnectionPrivate::shared_null()->driver;
    d = new QSparqlConnectionPrivate(driver, QString(),
                                     QSparqlConnectionOptions());
}

QSparqlConnection::QSparqlConnection(const QString& type,
                                     const QSparqlConnectionOptions& options,
                                     QObject* parent)
    : QObject(parent)
{
    QSparqlDriver* driver = QSparqlConnectionPrivate::findDriver(type);
    d = new QSparqlConnectionPrivate(driver, type, options);
    d->driver->open(d->options);
}

QSparqlConnection::~QSparqlConnection()
{
    d->driver->close();
    delete d;
}

/*!
    Executes a SPARQL query on the database and returns a pointer to a
    QSparqlResult object. The user is responsible for freeing it.

    If \a query is empty or if the this QSparqlConnection is not
    valid, exec() returns a QSparqlResult which is in the error
    state. It won't emit the finished() signal.

    // TODO: isn't it quite bad that the user must check the error
    // state of the result? Or should the "error result" emit the
    // finished() signal when the main loop is entered the next time,
    // so that the user has a change to connect to it?

    \sa QSparqlQuery, QSparqlResult
*/

QSparqlResult* QSparqlConnection::exec(const QSparqlQuery& query) const
{
    if (!d->driver->isOpen() || d->driver->isOpenError()) {
        qWarning("QSparqlConnection::exec: database not open");

        QSparqlResult* result = new QSparqlNullResult();
        result->setLastError(QSparqlError(QLatin1String("Connection not open"),
                                          QSparqlError::ConnectionError));
        return result;
    }
    QString queryText = query.preparedQueryText();
    if (queryText.isEmpty()) {
        qWarning("QSparqlConnection::exec: empty query");
        QSparqlResult* result = new QSparqlNullResult();
        result->setLastError(QSparqlError(QLatin1String("Query is empty"),
                                          QSparqlError::ConnectionError));
        return result;
    }
    return d->driver->exec(queryText, query.type());
}

/*!
    Returns the connection's driver name.
*/
QString QSparqlConnection::driverName() const
{
    return d->drvName;
}

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
    \snippet doc/src/snippets/code/src_sql_kernel_qsparqlconnection.cpp 8
*/
bool QSparqlConnection::isValid() const
{
    return d->driver && d->driver != d->shared_null()->driver;
}

QStringList QSparqlConnection::drivers()
{
    QStringList list;
    return list; // a hack

#if WE_ARE_QT
#ifdef QT_SPARQL_ODBC
    list << QLatin1String("QODBC3");
    list << QLatin1String("QODBC");
#endif
#ifdef QT_SPARQL_TRACKER
    list << QLatin1String("QTRACKER");
#endif
#ifdef QT_SPARQL_ENDPOINT
    list << QLatin1String("QENDPOINT");
#endif

#if !defined(QT_NO_LIBRARY) && !defined(QT_NO_SETTINGS)
    if (QFactoryLoader *fl = loader()) {
        QStringList keys = fl->keys();
        for (QStringList::const_iterator i = keys.constBegin(); i != keys.constEnd(); ++i) {
            if (!list.contains(*i))
                list << *i;
        }
    }
#endif

    DriverDict dict = QSparqlConnectionPrivate::driverDict();
    for (DriverDict::const_iterator i = dict.constBegin(); i != dict.constEnd(); ++i) {
        if (!list.contains(i.key()))
            list << i.key();
    }

    return list;
#endif
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
