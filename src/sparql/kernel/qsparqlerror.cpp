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

#include "qsparqlerror.h"
#include "qdebug.h"

QT_BEGIN_NAMESPACE

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QSparqlError &s)
{
    dbg.nospace() << "QSparqlError(" << s.number() << ", " << s.message() << ')';
    return dbg.space();
}
#endif

/*!
    \class QSparqlError
    \brief The QSparqlError class provides SPARQL error information.

    \ingroup database
    \inmodule QtSparql

    A QSparqlError object can provide driver-specific error data, including the
    error message(), number() and type(). The functions all have setters so that
    you can create and return QSparqlError objects from your own classes, for
    example from your own SPARQL drivers.

    \sa QSparqlConnection::lastError(), QSparqlResult::lastError()
*/

/*!
    \enum QSparqlError::ErrorType

    This enum type describes the context in which the error occurred, e.g., a connection error, a statement error, etc.

    \value NoError  No error occurred.
    \value ConnectionError  Connection error.
    \value StatementError  SPARQL statement syntax error.
    \value TransactionError  Transaction failed error.
    \value BackendError  Other backend-specific error
    \value UnknownError  Unknown error.

    \omitvalue None
    \omitvalue Connection
    \omitvalue Statement
    \omitvalue Transaction
    \omitvalue Backend
    \omitvalue Unknown
*/

/*!
    Constructs an error containing the driver error \a message, the
    type \a type and the optional error number \a number.
*/

QSparqlError::QSparqlError(const QString& message,
                           ErrorType type, int number)
    : errorMessage(message), errorType(type), errorNumber(number)
{
}

/*!
    Creates a copy of \a other.
*/
QSparqlError::QSparqlError(const QSparqlError& other)
    : errorMessage(other.errorMessage), errorType(other.errorType),
      errorNumber(other.errorNumber)
{
}

/*!
    Assigns the \a other error's values to this error.
*/

QSparqlError& QSparqlError::operator=(const QSparqlError& other)
{
    errorMessage = other.errorMessage;
    errorType = other.errorType;
    errorNumber = other.errorNumber;
    return *this;
}

/*!
    Destroys the object and frees any allocated resources.
*/

QSparqlError::~QSparqlError()
{
}

/*!
    Sets the error message to the value of \a message.

    \sa message()
*/

void QSparqlError::setMessage(const QString& message)
{
    errorMessage = message;
}

/*!
    Returns the error type, or -1 if the type cannot be determined.

    \sa setType()
*/

QSparqlError::ErrorType QSparqlError::type() const
{
    return errorType;
}

/*!
    Sets the error type to the value of \a type.

    \sa type()
*/

void QSparqlError::setType(ErrorType type)
{
    errorType = type;
}

/*!
    Returns the error message.

    \sa setMessage()
*/

QString QSparqlError::message() const
{
    return errorMessage;
}


/*!
    Returns the connection-specific error number, or -1 if it cannot be
    determined.

    \sa setNumber()
*/

int QSparqlError::number() const
{
    return errorNumber;
}

/*!
    Sets the connection-specific error number to \a number.

    \sa number()
*/

void QSparqlError::setNumber(int number)
{
    errorNumber = number;
}

/*!
    Returns true if an error is set, otherwise false.

    Example:
    \snippet doc/src/snippets/code/src_sparql_kernel_qsparqlerror.cpp 0

    \sa type()
*/
bool QSparqlError::isValid() const
{
    return errorType != NoError;
}

QT_END_NAMESPACE
