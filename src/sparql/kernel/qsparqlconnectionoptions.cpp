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

#include "qsparqlconnectionoptions.h"

#include "QtNetwork/qnetworkproxy.h"

QT_BEGIN_NAMESPACE

class QSparqlConnectionOptionsPrivate: public QSharedData
{
public:
    void copy(const QSparqlConnectionOptionsPrivate *other);

    QMap<QString,QVariant> map;

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
};

static QString hostKey = QString::fromLatin1("host");
static QString pathKey = QString::fromLatin1("path");
static QString portKey = QString::fromLatin1("port");
static QString dataReadyIntervalKey = QString::fromLatin1("dataReadyInterval");
static QString userKey = QString::fromLatin1("user");
static QString passwordKey = QString::fromLatin1("password");
static QString databaseKey = QString::fromLatin1("database");

template<> void QSharedDataPointer<QSparqlConnectionOptionsPrivate>::detach()
{
    if (d && d->ref == 1)
        return;
    QSparqlConnectionOptionsPrivate *x = (d ? new QSparqlConnectionOptionsPrivate(*d)
                               : new QSparqlConnectionOptionsPrivate);
    x->ref.ref();
    if (d && !d->ref.deref())
        delete d;
    d = x;
}

/*!
    \class QSparqlConnectionOptions

    \brief The QSparqlConnectionOptions class encapsulates options
    given to QSparqlConnection. Some options are used only by some
    drivers.

    \ingroup database
    \ingroup shared
    \inmodule QtSparql

    The function setOption() can be used for setting an option and
    option() for getting a value of an option. An option can be unset
    by passing a QVariant() to setOption().

    For frequently used options, there are separate convenience functions,
    e.g., setHost() and host().

    \sa QSparqlConnection
*/

QSparqlConnectionOptions::QSparqlConnectionOptions()
    : d(0)
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
    return d == other.d || (d && other.d && *d == *other.d);
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
    d->map[name] = value;
}

/*!
    Convenience function for setting the database name.

    \sa setOption()
*/
void QSparqlConnectionOptions::setDatabaseName(const QString& name)
{
    setOption(databaseKey, name);
}

/*!
    Convenience function for setting the user name. Used by
    connections requiring authentication.

    \sa setOption()
*/
void QSparqlConnectionOptions::setUserName(const QString& name)
{
    setOption(userKey, name);
}

/*!
    Convenience function for setting the password. Used by
    connections requiring authentication.

    \sa setOption()
*/
void QSparqlConnectionOptions::setPassword(const QString& password)
{
    setOption(passwordKey, password);
}

/*!
    Convenience function for setting the host name of the RDF store.

    \sa setOption()
*/
void QSparqlConnectionOptions::setHostName(const QString& host)
{
    setOption(hostKey, host);
}

/*!
    Convenience function for setting the path of the RDF store.

    \sa setOption()
*/
void QSparqlConnectionOptions::setPath(const QString& path)
{
    setOption(pathKey, path);
}

/*!
    Convenience function for setting the port of the RDF store.

    \sa setOption()
*/
void QSparqlConnectionOptions::setPort(int p)
{
    setOption(portKey, p);
}

/*!
    Convenience function for setting the interval between when
    dataReady(int) signals are emitted

    \sa setOption()
*/
void QSparqlConnectionOptions::setDataReadyInterval(int interval)
{
    if (interval > 0)
        setOption(dataReadyIntervalKey, interval);
    else
        qWarning() << "QSparqlConnectionOptions: invalid dataReady interval:" << interval;
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
    return d ? option(databaseKey).toString() : QString();
}

/*!
    Convenience function for getting the path.

    \sa option()
*/
QString QSparqlConnectionOptions::path() const
{
    return d ? option(pathKey).toString() : QString();
}

/*!
    Convenience function for getting the user name. Used by
    connections requiring authentication.

    \sa option()
*/
QString QSparqlConnectionOptions::userName() const
{
    return d ? option(userKey).toString() : QString();
}

/*!
    Convenience function for getting the password. Used by
    connections requiring authentication.

    \sa option()
*/
QString QSparqlConnectionOptions::password() const
{
    return d ? option(passwordKey).toString() : QString();
}

/*!
    Convenience function for getting the host name.

    \sa option()
*/
QString QSparqlConnectionOptions::hostName() const
{
    return d ? option(hostKey).toString() : QString();
}

/*!
    Convenience function for getting the port.

    \sa option()
*/
int QSparqlConnectionOptions::port() const
{
    QVariant v = option(portKey);
    return v.canConvert(QVariant::Int) ? v.toInt() : -1;
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
    QVariant v = option(dataReadyIntervalKey);
    return v.canConvert(QVariant::Int) ? v.toInt() : 1;
}

/*!
    Convenience function for getting the QNetworkAccessManager. Used
    by connections which use the network.

    This option cannot be set using the option() function.
*/
QNetworkAccessManager* QSparqlConnectionOptions::networkAccessManager() const
{
    return d ? d->nam : 0;
}

#ifndef QT_NO_NETWORKPROXY
/*!
    Convenience function for setting the QNetworkProxy. Used
    by connections which use the network.

    This option cannot be set using the setOption() function.
*/
QNetworkProxy QSparqlConnectionOptions::proxy() const
{
    return d ? d->proxy : QNetworkProxy();
}
#endif

/*!
    Returns the value of a given option, \a name, or QVariant(), if
    there is no value.

    \sa setOption()
*/
QVariant QSparqlConnectionOptions::option(const QString& name) const
{
    if (d == 0)
        return QVariant();
    if (d->map.contains(name))
        return d->map[name];
    else
        return QVariant();
}

QT_END_NAMESPACE
