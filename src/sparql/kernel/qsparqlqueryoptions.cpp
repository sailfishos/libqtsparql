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

#include "qsparqlqueryoptions.h"

QT_BEGIN_NAMESPACE

class QSparqlQueryOptionsPrivate: public QSharedData
{
public:
};

/*!
    \class QSparqlQueryOptions

    \brief Encapsulates query execution options
    given to QSparqlConnection::exec(const QSparqlQuery&, const QSparqlQueryOptions&).
    Some options are used only by some drivers.

    \sa QSparqlConnection::exec(const QSparqlQuery&, const QSparqlQueryOptions&)
*/

QSparqlQueryOptions::QSparqlQueryOptions()
    : d(0)
{
}

QSparqlQueryOptions::~QSparqlQueryOptions()
{
    // QSharedDataPointer takes care of deleting for us
}

/// Creates a QSparqlQueryOptions object based on \a other, copying all
/// option values from it.
QSparqlQueryOptions::QSparqlQueryOptions(const QSparqlQueryOptions& other)
    : d(other.d)
{
}

/// Assigns the QSparqlQueryOptions object. Copies all option values from
/// \a other.
QSparqlQueryOptions& QSparqlQueryOptions::operator=(const QSparqlQueryOptions& other)
{
    d = other.d;
    return *this;
}

/// Returns true if the QSparqlQueryOptions objects are equal.
bool QSparqlQueryOptions::operator==(const QSparqlQueryOptions &other) const
{
    return (d == other.d /*|| *d == *other.d*/);
}

/*!
    \enum QSparqlQueryOptions::ExecutionMethod
    Execution method of the query.

    \var QSparqlQueryOptions::ExecutionMethod QSparqlQueryOptions::ExecAsync
    Query is to be executed asynchronously. This is the default value of QSparqlQueryOptions.

    \var QSparqlQueryOptions::ExecutionMethod QSparqlQueryOptions::ExecSync
    Query is to be executed synchronously.

    \sa setExecutionMethod, executionMethod
*/

/// Sets the exeuction method of the query.
/// The default execution method is ExecAsync.
/// \sa executionMethod
void QSparqlQueryOptions::setExecutionMethod(ExecutionMethod em)
{
    Q_UNUSED(em);
}

/// Returns the exeuction method of the query.
/// \sa setExecutionMethod
QSparqlQueryOptions::ExecutionMethod QSparqlQueryOptions::executionMethod() const
{
    return ExecAsync;
}

/*!
    \enum QSparqlQueryOptions::Priority
    Priority of the query.

    \var QSparqlQueryOptions::Priority QSparqlQueryOptions::PriorityNormal
    Query is to be executed with normal priority. This is the default value of QSparqlQueryOptions.

    \var QSparqlQueryOptions::Priority QSparqlQueryOptions::PriorityLow
    Query is to be executed with low priority. Support for this option is driver-specfifc.

    \sa setPriority, priority
*/

/// Sets the priority of the query.
/// The default priority is PriorityNormal.
/// \sa priority
void QSparqlQueryOptions::setPriority(Priority p)
{
    Q_UNUSED(p);
}

/// Returns the priority of the query.
/// \sa setPriority
QSparqlQueryOptions::Priority QSparqlQueryOptions::priority() const
{
    return PriorityNormal;
}

QT_END_NAMESPACE
