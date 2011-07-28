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

#include "qsparqlconnectionoptions.h"

#include "QtNetwork/qnetworkproxy.h"

QT_BEGIN_NAMESPACE

class QSparqlConnectionOptionsPrivate: public QSharedData
{
public:
    typedef QMap<QString,QVariant> OptionMap;
    OptionMap map;

    // Settings that cannot easily be put in a QVariant
#ifndef QT_NO_NETWORKPROXY
    QNetworkProxy proxy;
#endif
    QNetworkAccessManager* nam;

    inline QSparqlConnectionOptionsPrivate()
        : nam(0)
        {
        }

    inline bool operator==(const QSparqlConnectionOptionsPrivate &other) const
        {
            return map == other.map &&
#ifndef QT_NO_NETWORKPROXY
                proxy == other.proxy &&
#endif
                nam == other.nam;
    }

    QVariant option(const QString& name) const;
    QVariant optionOrDefaultValue(const QString& name) const;
    static bool validateOptionValue(const QString& name, const QVariant& value, QVariant& convertedValue);
    void setOption(const QString& name, const QVariant& value);

    class OptionInfo;
    class OptionRegistry;
    };

Q_GLOBAL_STATIC_WITH_ARGS(const QString, hostKey,  (QString::fromLatin1("host")));
Q_GLOBAL_STATIC_WITH_ARGS(const QString, pathKey, (QString::fromLatin1("path")));
Q_GLOBAL_STATIC_WITH_ARGS(const QString, portKey, (QString::fromLatin1("port")));
Q_GLOBAL_STATIC_WITH_ARGS(const QString, dataReadyIntervalKey, (QString::fromLatin1("dataReadyInterval")));
Q_GLOBAL_STATIC_WITH_ARGS(const QString, userKey, (QString::fromLatin1("user")));
Q_GLOBAL_STATIC_WITH_ARGS(const QString, passwordKey, (QString::fromLatin1("password")));
Q_GLOBAL_STATIC_WITH_ARGS(const QString, databaseKey, (QString::fromLatin1("database")));
Q_GLOBAL_STATIC_WITH_ARGS(const QString, maxThreadKey, (QString::fromLatin1("maxThread")));
Q_GLOBAL_STATIC_WITH_ARGS(const QString, threadExpiryKey, (QString::fromLatin1("threadExpiry")));

class QSparqlConnectionOptionsPrivate::OptionInfo {
public:
    typedef bool (*ValidationFunc)(const QVariant& value);

    const QVariant defaultValue;

    OptionInfo(const QVariant& dv, ValidationFunc vf=0)
        : defaultValue(dv), validationFunc(vf)
    {
    }

    bool validateValue(const QVariant& value, QVariant& convertedValue) const {
        QVariant newValue(value);
        if (!newValue.convert(defaultValue.type()))
            return false;
        if (validationFunc && !(*validationFunc)(newValue))
            return false;
        convertedValue = newValue;
        return true;
    }

private:
    ValidationFunc validationFunc;
};

class QSparqlConnectionOptionsPrivate::OptionRegistry {
public:
    OptionRegistry()
    {
        registry.insert(*hostKey(),              new OptionInfo(QVariant(QString())) );
        registry.insert(*pathKey(),              new OptionInfo(QVariant(QString())) );
        registry.insert(*userKey(),              new OptionInfo(QVariant(QString())) );
        registry.insert(*passwordKey(),          new OptionInfo(QVariant(QString())) );
        registry.insert(*databaseKey(),          new OptionInfo(QVariant(QString())) );
        registry.insert(*portKey(),              new OptionInfo(QVariant(int(-1)))   );
        registry.insert(*dataReadyIntervalKey(), new OptionInfo(QVariant(int(1)),  &greaterThanZero) );
        registry.insert(*maxThreadKey(),         new OptionInfo(QVariant(int(-1)), &greaterThanZero) );
        registry.insert(*threadExpiryKey(),      new OptionInfo(QVariant(int(-1))) );
    }

    ~OptionRegistry()
    {
        qDeleteAll(registry);
    }

    const OptionInfo* lookup(const QString& optionName) const
    {
        return registry.value(optionName);
    }

private:
    static bool greaterThanZero(const QVariant& value)
    {
        return (value.toInt() > 0);
    }

private:
    QMap<QString, const OptionInfo*> registry;
};

Q_GLOBAL_STATIC(QSparqlConnectionOptionsPrivate::OptionRegistry, connectionOptionRegistry);

QVariant QSparqlConnectionOptionsPrivate::option(const QString& name) const
{
    return map.value(name);
}

QVariant QSparqlConnectionOptionsPrivate::optionOrDefaultValue(const QString& name) const
{
    OptionMap::const_iterator optionIter = map.constFind(name);
    if (optionIter != map.constEnd()) {
        return *optionIter;
    }
    else {
        const OptionInfo* optionInfo = connectionOptionRegistry()->lookup(name);
        return (optionInfo ? optionInfo->defaultValue : QVariant());
    }
}

bool QSparqlConnectionOptionsPrivate::validateOptionValue(const QString& name, const QVariant& value, QVariant& convertedValue)
{
    const OptionInfo* optionInfo = connectionOptionRegistry()->lookup(name);
    if (!optionInfo) {
        // Use driver-specific option values as-is
        convertedValue = value;
        return true;
    }

    if (!optionInfo->validateValue(value, convertedValue)) {
        qWarning() << "QSparqlConnectionOptions: invalid value for option" << name << ":" << value;
        return false;
    }
    return true;
}

void QSparqlConnectionOptionsPrivate::setOption(const QString& name, const QVariant& value)
{
    if (value.isValid()) {
        QVariant convertedValue;
        if (validateOptionValue(name, value, convertedValue))
            map.insert(name, convertedValue);
    }
    else {
        map.remove(name);
    }
}

/*!
    \class QSparqlConnectionOptions

    \brief The QSparqlConnectionOptions class encapsulates options
    given to QSparqlConnection. Some options are used only by some
    drivers.

    The function setOption() can be used for setting an option and
    option() for getting a value of an option. An option can be unset
    by passing a QVariant() to setOption().

    For frequently used options, there are separate convenience functions,
    e.g., setHost() and host().

    \sa QSparqlConnection
*/

QSparqlConnectionOptions::QSparqlConnectionOptions()
    : d(new QSparqlConnectionOptionsPrivate)
{
}

QSparqlConnectionOptions::~QSparqlConnectionOptions()
{
    // QSharedDataPointer takes care of deleting for us
}

/// Returns true if the QSparqlConnection objects are equal. The objects are
/// equal if they describe the same set of key-value pairs.
bool QSparqlConnectionOptions::operator==(const QSparqlConnectionOptions &other) const
{
    return d == other.d || *d == *other.d;
}

/// Creates a QSparqlConnectionOptions object based on \a other, copying all
/// key-value pairs from it.
QSparqlConnectionOptions::QSparqlConnectionOptions(const QSparqlConnectionOptions& other)
    : d(other.d)
{
}

/// Assigns the QSparqlConnectionOptions object. Copies all key-value pairs from
/// \a other.
QSparqlConnectionOptions& QSparqlConnectionOptions::operator=(const QSparqlConnectionOptions& other)
{
    d = other.d;
    return *this;
}

/*!
    Sets the option \a name to a given \a value. Use a null QVariant,
    i.e. QVariant(), to unset an option.
*/
void QSparqlConnectionOptions::setOption(const QString& name, const QVariant& value)
{
    d->setOption(name, value);
}

/*!
    Convenience function for setting the database name.

    \sa setOption()
*/
void QSparqlConnectionOptions::setDatabaseName(const QString& name)
{
    setOption(*databaseKey(), name);
}

/*!
    Convenience function for setting the user name. Used by
    connections requiring authentication.

    \sa setOption()
*/
void QSparqlConnectionOptions::setUserName(const QString& name)
{
    setOption(*userKey(), name);
}

/*!
    Convenience function for setting the password. Used by
    connections requiring authentication.

    \sa setOption()
*/
void QSparqlConnectionOptions::setPassword(const QString& password)
{
    setOption(*passwordKey(), password);
}

/*!
    Convenience function for setting the host name of the RDF store.

    \sa setOption()
*/
void QSparqlConnectionOptions::setHostName(const QString& host)
{
    setOption(*hostKey(), host);
}

/*!
    Convenience function for setting the path of the RDF store.

    \sa setOption()
*/
void QSparqlConnectionOptions::setPath(const QString& path)
{
    setOption(*pathKey(), path);
}

/*!
    Convenience function for setting the port of the RDF store.

    \sa setOption()
*/
void QSparqlConnectionOptions::setPort(int p)
{
    setOption(*portKey(), p);
}

/*!
    Convenience function for setting the interval between when
    dataReady(int) signals are emitted.

    \sa setOption()
*/
void QSparqlConnectionOptions::setDataReadyInterval(int interval)
{
    setOption(*dataReadyIntervalKey(), interval);
}

/*!
    Convenience function for setting the maximum number of
    threads for the thread pool to use.

    \sa setOption()
*/
void QSparqlConnectionOptions::setMaxThreadCount(int p)
{
    setOption(*maxThreadKey(), p);
}

/*!
    Convenience function for setting the expiry time (in milliseconds)
    of threads created by the threadpool.

    \sa setOption()
*/
void QSparqlConnectionOptions::setThreadExpiryTime(int p)
{
    //do not need to check for > 0, if the value is negative
    //threads will not expire until the threadpool is destroyed
    setOption(*threadExpiryKey(), p);
}
#ifndef QT_NO_NETWORKPROXY
/*!
    Convenience function for setting the QNetworkProxy. Valid
    for drivers connecting to databases over the network.

    This option cannot be set using the setOption() function.
*/
void QSparqlConnectionOptions::setProxy(const QNetworkProxy& proxy)
{
    d->proxy = proxy;
}
#endif

/*!
    Convenience function for setting the QNetworkAccessManager. Valid
    for drivers connecting to databases over the network.

    This option cannot be set using the setOption() function.
*/
void QSparqlConnectionOptions::setNetworkAccessManager(QNetworkAccessManager* nam)
{
    d->nam = nam;
}

/*!
    Convenience function for getting the database name.

    \sa option()
*/
QString QSparqlConnectionOptions::databaseName() const
{
    return d->optionOrDefaultValue(*databaseKey()).value<QString>();
}

/*!
    Convenience function for getting the path.

    \sa option()
*/
QString QSparqlConnectionOptions::path() const
{
    return d->optionOrDefaultValue(*pathKey()).value<QString>();
}

/*!
    Convenience function for getting the user name. Used by
    connections requiring authentication.

    \sa option()
*/
QString QSparqlConnectionOptions::userName() const
{
    return d->optionOrDefaultValue(*userKey()).value<QString>();
}

/*!
    Convenience function for getting the password. Used by
    connections requiring authentication.

    \sa option()
*/
QString QSparqlConnectionOptions::password() const
{
    return d->optionOrDefaultValue(*passwordKey()).value<QString>();
}

/*!
    Convenience function for getting the host name.

    \sa option()
*/
QString QSparqlConnectionOptions::hostName() const
{
    return d->optionOrDefaultValue(*hostKey()).value<QString>();
}

/*!
    Convenience function for getting the port.

    \sa option()
*/
int QSparqlConnectionOptions::port() const
{
    return d->optionOrDefaultValue(*portKey()).value<int>();
}

/*!
    Convenience function for getting the interval between when
    dataReady(int) signals are emitted. The default value is 1,
    which means that a dataReady() signal is emitted as soon as
    a driver has some results.

    \sa option()
*/
int QSparqlConnectionOptions::dataReadyInterval() const
{
    return d->optionOrDefaultValue(*dataReadyIntervalKey()).value<int>();
}

/*!
    Convenience function for getting the max thread count to be
    used by the thread pool.

    \sa option()
*/
int QSparqlConnectionOptions::maxThreadCount() const
{
    return d->optionOrDefaultValue(*maxThreadKey()).value<int>();
}

/*!
    Convenience function for getting the thread expiry time.

    \sa option()
*/
int QSparqlConnectionOptions::threadExpiryTime() const
{
    return d->optionOrDefaultValue(*threadExpiryKey()).value<int>();
}

/*!
    Convenience function for getting the QNetworkAccessManager. Used
    by connections which use the network.

    This option cannot be set using the option() function.
*/
QNetworkAccessManager* QSparqlConnectionOptions::networkAccessManager() const
{
    return d->nam;
}

#ifndef QT_NO_NETWORKPROXY
/*!
    Convenience function for setting the QNetworkProxy. Used
    by connections which use the network.

    This option cannot be set using the setOption() function.
*/
QNetworkProxy QSparqlConnectionOptions::proxy() const
{
    return d->proxy;
}
#endif

/*!
    Returns the value of a given option, \a name, or QVariant(), if
    there is no value.

    \sa setOption()
*/
QVariant QSparqlConnectionOptions::option(const QString& name) const
{
    return d->option(name);
}

QT_END_NAMESPACE
