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

#include "qsparqldriver_p.h"

#include <QtCore/qmap.h>
#include <QtCore/qdatetime.h>

#include "qsparqlerror.h"
#include "qsparqlbinding.h"

QT_BEGIN_NAMESPACE

class QSparqlDriverPrivate
{
public:
    QSparqlDriverPrivate();
    virtual ~QSparqlDriverPrivate();

public:
    // @CHECK: this member is never used. It was named q, which expanded to q_func().
    QSparqlDriver *q_func();
    uint isOpen : 1;
    uint isOpenError : 1;
    QSparqlError error;
    QMap<QString, QUrl> prefixes;
};

inline QSparqlDriverPrivate::QSparqlDriverPrivate()
    : isOpen(false), isOpenError(false)
{
}

QSparqlDriverPrivate::~QSparqlDriverPrivate()
{
}

/*  FIXME: ! removed from this line on purpose, enable this doc when Driver is public
    \class QSparqlDriver
    \brief The QSparqlDriver class is an abstract base class for accessing
    specific SPARQL based RDF stores.

    \ingroup database

    This class should not be used directly. Use QSparqlConnection instead.

    If you want to create your own SPARQL drivers, you can subclass this
    class and reimplement its pure virtual functions and those
    virtual functions that you need. See \l{How to Write Your Own
    Database Driver} for more information.

    \sa QSparqlConnection, QSparqlResult
*/

/*!
    Constructs a new driver with the given \a parent.
*/

QSparqlDriver::QSparqlDriver(QObject *parent)
    : QObject(parent)
{
    d = new QSparqlDriverPrivate();
}

/*!
    Destroys the object and frees any allocated resources.
*/

QSparqlDriver::~QSparqlDriver()
{
    delete d;
}

/*!
    \fn bool QSparqlDriver::open(const QSparqlConnectionOptions& options)

    Derived classes must reimplement this pure virtual function to
    open a database connection with the given \a options.

    The function must return true on success and false on failure.
*/

/*!
    \fn bool QSparqlDriver::close()

    Derived classes must reimplement this pure virtual function in
    order to close the database connection. Return true on success,
    false on failure.

    \sa open()
*/

/*!
    Returns true if the database connection is open; otherwise returns
    false.
*/

bool QSparqlDriver::isOpen() const
{
    return d->isOpen;
}

/*!
    Returns true if the there was an error opening the database
    connection; otherwise returns false.
*/

bool QSparqlDriver::isOpenError() const
{
    return d->isOpenError;
}

/*!
    \fn bool QSparqlDriver::hasFeature(QSparqlConnection::Feature feature) const

    Returns true if the driver supports feature \a feature; otherwise
    returns false.

    Note that some databases need to be open() before this can be
    determined.

    \sa DriverFeature
*/

/*!
    This function sets the open state of the database to \a open.
    Derived classes can use this function to report the status of
    open().

    \sa open(), setOpenError()
*/

void QSparqlDriver::setOpen(bool open)
{
    d->isOpen = open;
}

/*!
    This function sets the open error state of the database to \a
    error. Derived classes can use this function to report the status
    of open(). Note that if \a error is true the open state of the
    database is set to closed (i.e., isOpen() returns false).

    \sa open(), setOpen()
*/

void QSparqlDriver::setOpenError(bool error)
{
    d->isOpenError = error;
    if (error)
        d->isOpen = false;
}

/*!
    This function is called to begin a transaction. If successful,
    return true, otherwise return false. The default implementation
    does nothing and returns false.

    \sa commitTransaction(), rollbackTransaction()
*/

bool QSparqlDriver::beginTransaction()
{
    return false;
}

/*!
    This function is called to commit a transaction. If successful,
    return true, otherwise return false. The default implementation
    does nothing and returns false.

    \sa beginTransaction(), rollbackTransaction()
*/

bool QSparqlDriver::commitTransaction()
{
    return false;
}

/*!
    This function is called to rollback a transaction. If successful,
    return true, otherwise return false. The default implementation
    does nothing and returns false.

    \sa beginTransaction(), commitTransaction()
*/

bool QSparqlDriver::rollbackTransaction()
{
    return false;
}

/*!
    This function is used to set the value of the last error, \a error,
    that occurred on the database.

    \sa lastError()
*/

void QSparqlDriver::setLastError(const QSparqlError &error)
{
    d->error = error;
}

/*!
    Returns a QSparqlError object which contains information about the
    last error that occurred on the database.
*/

QSparqlError QSparqlDriver::lastError() const
{
    return d->error;
}

/*!
    Returns the low-level database handle wrapped in a QVariant or an
    invalid variant if there is no handle.

    \warning Use this with uttermost care and only if you know what you're doing.

    \warning The handle returned here can become a stale pointer if the connection
    is modified (for example, if you close the connection).

    \warning The handle can be NULL if the connection is not open yet.

    The handle returned here is database-dependent, you should query the type
    name of the variant before accessing it.

    \sa QSparqlResult::handle()
*/
QVariant QSparqlDriver::handle() const
{
    return QVariant();
}

QSparqlResult* QSparqlDriver::syncExec(const QString& query, QSparqlQuery::StatementType)
{
    return 0;
}

/*!
    Adds a prefix/uri pair to the connection. Each SPARQL query made
    with the connection will have the prefixes prepended to it.

    \sa prefixes() clearPrefixes()
*/
void QSparqlDriver::addPrefix(const QString& prefix, const QUrl& uri)
{
    d->prefixes.insert(prefix, uri);
}

/*!
    Returns the text of the prefix/uri pairs added with addPrefix(),
    in a form suitable for prepending to a SPARQL query.

    \sa addPrefix() clearPrefixes()
*/

QString QSparqlDriver::prefixes() const
{
    QString result;
    QMap<QString, QUrl>::const_iterator i = d->prefixes.constBegin();

    while (i != d->prefixes.constEnd()) {
        QString prefix = QString::fromLatin1("PREFIX %1: <%2>\n");
        result.append(prefix.arg(i.key()).arg(QString::fromUtf8(i.value().toEncoded())));

        ++i;
    }

    return result;
}

/*!
    Removes all prefix/uri pairs that were added with addPrefix()

    \sa addPrefix() prefixes()
*/

void QSparqlDriver::clearPrefixes()
{
    d->prefixes.clear();
}

QT_END_NAMESPACE
