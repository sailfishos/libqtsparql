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

#include "qsparqlqueryoptions.h"
#include <QDebug>
QT_BEGIN_NAMESPACE

class QSparqlQueryOptionsPrivate: public QSharedData
{
public:
    QSparqlQueryOptionsPrivate();
    // Compiler-generated copy ctor, assignment operator and destructor are OK for this class

    bool operator==(const QSparqlQueryOptionsPrivate& other) const;

    QSparqlQueryOptions::ExecutionMethod executionMethod;
    QSparqlQueryOptions::Priority priority;
    bool forwardOnly;
};

QSparqlQueryOptionsPrivate::QSparqlQueryOptionsPrivate()
    // Set the options to their default values
    : executionMethod(QSparqlQueryOptions::AsyncExec)
    , priority(QSparqlQueryOptions::NormalPriority)
    , forwardOnly(false)
{
}

bool QSparqlQueryOptionsPrivate::operator==(const QSparqlQueryOptionsPrivate& other) const
{
    return (executionMethod == other.executionMethod &&
            priority == other.priority);
}

/*!
    \class QSparqlQueryOptions

    \brief Encapsulates query execution options
    given to QSparqlConnection::exec(const QSparqlQuery&, const QSparqlQueryOptions&).
    Some options are used only by some drivers.

    \sa QSparqlConnection::exec(const QSparqlQuery&, const QSparqlQueryOptions&)
*/

QSparqlQueryOptions::QSparqlQueryOptions()
    : d(new QSparqlQueryOptionsPrivate)
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
    return (d == other.d || *d == *other.d);
}

/*!
    \enum QSparqlQueryOptions::ExecutionMethod
    Execution method of the query.

    \var QSparqlQueryOptions::ExecutionMethod QSparqlQueryOptions::AsyncExec
    Query is to be executed asynchronously. This is the default value of QSparqlQueryOptions.

    \var QSparqlQueryOptions::ExecutionMethod QSparqlQueryOptions::SyncExec
    Query is to be executed synchronously.

    \sa setExecutionMethod, executionMethod
*/

/// Sets the exeuction method of the query.
/// The default execution method is QSparqlQueryOptions::AsyncExec.
/// \sa executionMethod
void QSparqlQueryOptions::setExecutionMethod(ExecutionMethod em)
{
    d->executionMethod = em;
}

/// Returns the execution method of the query.
/// \sa setExecutionMethod
QSparqlQueryOptions::ExecutionMethod QSparqlQueryOptions::executionMethod() const
{
    return d->executionMethod;
}

/*!
    Sets whether or not to execute asynchronous queries in a ForwardOnly manner.
    Support for this option is currently limited to the QTRACKER_DIRECT driver.
    \sa \ref trackerdirectspecific "QTRACKER_DIRECT specific usage"
*/
void QSparqlQueryOptions::setForwardOnly(bool forward)
{
    d->forwardOnly = forward;
}

/*!
    Returns whether or not an asynchronous query will be executed in a forward only manner.
*/
bool QSparqlQueryOptions::isForwardOnly() const
{
    return d->forwardOnly;
}

/*!
    \enum QSparqlQueryOptions::Priority
    Priority of the query.

    \var QSparqlQueryOptions::Priority QSparqlQueryOptions::NormalPriority
    Query is to be executed with normal priority. This is the default value of QSparqlQueryOptions.

    \var QSparqlQueryOptions::Priority QSparqlQueryOptions::LowPriority
    Query is to be executed with low priority. Support for this option is driver-specfifc.

    \sa setPriority, priority
*/

/// Sets the priority of the query.
/// The default priority is QSparqlQueryOptions::NormalPriority.
/// \sa priority
void QSparqlQueryOptions::setPriority(Priority p)
{
    d->priority = p;
}

/// Returns the priority of the query.
/// \sa setPriority
QSparqlQueryOptions::Priority QSparqlQueryOptions::priority() const
{
    return d->priority;
}

QT_END_NAMESPACE
