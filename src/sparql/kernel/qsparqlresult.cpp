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

#include "qvariant.h"
#include "qhash.h"
#include "qregexp.h"
#include "qsparqlerror.h"
#include "qsparqlbinding.h"
#include "qsparqlbindingset.h"
#include "qsparqlresult.h"
#include "qvector.h"
#include "qsparqldriver_p.h"
#include <QDebug>

QT_BEGIN_NAMESPACE

class QSparqlResultPrivate
{
public:
    QSparqlResultPrivate()
    : idx(QSparql::BeforeFirstRow)
    {}

public:
    int idx;
    QString sparql; // FIXME: needed?
    QSparqlError error;

    QString executedQuery; // FIXME: needed?
};

/*!
    \class QSparqlResult
    \brief The QSparqlResult class provides an abstract interface for
    accessing the asynchronous results of an executed QSparqlQuery.

    \ingroup database

    When QSparqlConnection::exec() is called, it asynchronously begins
    the execution of the given query. The returned result is in an
    unfinished state so that isFinished() returns false. When the execution
    finished, QSparqlResult emits the finished() signal and sets
    itself to finished state.

    Initially, QSparqlResult is positioned on an invalid position. It
    must be navigated to a valid position (so that isValid() returns
    true) before values can be retrieved.

    If you are implementing your own SQL driver (by subclassing
    QSparqlDriver), you will need to provide your own QSparqlResult
    subclass that implements all the pure virtual functions and other
    virtual functions that you need.

    \target QSparqlResult examples

    Navigating records is performed with the following functions:

    \list
    \o next()
    \o previous()
    \o first()
    \o last()
    \o seek()
    \endlist

    These functions allow the programmer to move forward, backward
    or arbitrarily through the records returned by the query. If you
    only need to move forward through the results (e.g., by using
    next()), you can use setForwardOnly(), which will save a
    significant amount of memory overhead and improve performance on
    some databases. Once a finished query is positioned on a valid
    record, data can be retrieved using value(). All data is
    transferred from the SQL backend using QVariants.

    For example:

    \snippet doc/src/snippets/sqldatabase/sqldatabase.cpp 7

    To access the data returned by a query, use value(int). Each
    field in the data returned by a \c SELECT statement is accessed
    by passing the field's position in the statement, starting from
    0. This makes using \c{SELECT *} queries inadvisable because the
    order of the fields returned is indeterminate.

    For the sake of efficiency, there are no functions to access a
    field by name (unless you use prepared queries with names, as
    explained below). To convert a field name into an index, use
    bindingSet().\l{QSparqlBindingSet::indexOf()}{indexOf()}, for example:

    \snippet doc/src/snippets/sqldatabase/sqldatabase.cpp 8

    \sa QSparqlDriver
*/

/*!
    Creates a QSparqlResult. The result is empty, positioned at
    "before first row" and unfinished.

    \sa driver()
*/

QSparqlResult::QSparqlResult()
{
    d = new QSparqlResultPrivate();
}

/*!
    Destroys the object and frees any allocated resources.
*/

QSparqlResult::~QSparqlResult()
{
    delete d;
}

QString QSparqlResult::lastQuery() const
{
    return d->sparql;
}

/*!
    Returns the current internal position of the query. The first
    record is at position zero. If the position is invalid, the
    function returns QSparql::BeforeFirstRow or
    QSparql::AfterLastRow, which are special negative values.

    \sa previous() next() first() last() seek()
*/

int QSparqlResult::pos() const
{
    return d->idx;
}

/*!
    Returns true if the result is positioned on a valid row (that
    is, the result is not positioned before the first or after the
    last row); otherwise returns false.

    \sa pos()
*/

bool QSparqlResult::isValid() const
{
    return d->idx != QSparql::BeforeFirstRow && d->idx != QSparql::AfterLastRow;
}

void QSparqlResult::waitForFinished()
{
}

bool QSparqlResult::isFinished() const
{
    return false;
}

/*!
  Retrieves the record at position \a index, if available, and
  positions the query on the retrieved record. The first record is at
  position 0. Note that the query must be in an \l{finished()}
  {finished} state and isSelect() must return true before calling this
  function.

  If \a relative is false (the default), the following rules apply:

  \list

  \o If \a index is negative, the result is positioned before the
  first record and false is returned.

  \o Otherwise, an attempt is made to move to the record at position
  \a index. If the record at position \a index could not be retrieved,
  the result is positioned after the last record and false is
  returned. If the record is successfully retrieved, true is returned.

  \endlist

  If \a relative is true, the following rules apply:

  \list

  \o If the result is currently positioned before the first record or
  on the first record, and \a index is negative, there is no change,
  and false is returned.

  \o If the result is currently located after the last record, and \a
  index is positive, there is no change, and false is returned.

  \o If the result is currently located somewhere in the middle, and
  the relative offset \a index moves the result below zero, the result
  is positioned before the first record and false is returned.

  \o Otherwise, an attempt is made to move to the record \a index
  records ahead of the current record (or \a index records behind the
  current record if \a index is negative). If the record at offset \a
  index could not be retrieved, the result is positioned after the
  last record if \a index >= 0, (or before the first record if \a
  index is negative), and false is returned. If the record is
  successfully retrieved, true is returned.

  \endlist

  \sa next() previous() first() last() pos() isFinished() isValid()
*/

bool QSparqlResult::seek(int index)
{
    if (!isFinished())
        return false;
    int actualIdx;
    if (index < 0) {
        setPos(QSparql::BeforeFirstRow);
        return false;
    }
    actualIdx = index;
    // let drivers optimize
    if (actualIdx == (pos() + 1) && pos() != QSparql::BeforeFirstRow) {
        if (!fetchNext()) {
            setPos(QSparql::AfterLastRow);
            return false;
        }
        return true;
    }
    if (actualIdx == (pos() - 1)) {
        if (!fetchPrevious()) {
            setPos(QSparql::BeforeFirstRow);
            return false;
        }
        return true;
    }
    if (!fetch(actualIdx)) {
        setPos(QSparql::AfterLastRow);
        return false;
    }
    return true;
}

/*!

  Retrieves the next record in the result, if available, and positions
  the query on the retrieved record. Note that the result must be in
  the \l{isFinished()}{finished} state and isSelect() must return true
  before calling this function or it will do nothing and return false.

  The following rules apply:

  \list

  \o If the result is currently located before the first record,
  e.g. immediately after a query is executed, an attempt is made to
  retrieve the first record.

  \o If the result is currently located after the last record, there
  is no change and false is returned.

  \o If the result is located somewhere in the middle, an attempt is
  made to retrieve the next record.

  \endlist

  If the record could not be retrieved, the result is positioned after
  the last record and false is returned. If the record is successfully
  retrieved, true is returned.

  \sa previous() first() last() seek() pos() isFinished() isValid()
*/
bool QSparqlResult::next()
{
    if (!isFinished())
        return false;
    bool b = false;
    switch (pos()) {
    case QSparql::BeforeFirstRow:
        b = fetchFirst();
        return b;
    case QSparql::AfterLastRow:
        return false;
    default:
        if (!fetchNext()) {
            setPos(QSparql::AfterLastRow);
            return false;
        }
        return true;
    }
}

/*!

  Retrieves the previous record in the result, if available, and
  positions the query on the retrieved record. Note that the result
  must be in the \l{isFinished()}{finished} state before calling this
  function or it will do nothing and return false.

  The following rules apply:

  \list

  \o If the result is currently located before the first record, there
  is no change and false is returned.

  \o If the result is currently located after the last record, an
  attempt is made to retrieve the last record.

  \o If the result is somewhere in the middle, an attempt is made to
  retrieve the previous record.

  \endlist

  If the record could not be retrieved, the result is positioned
  before the first record and false is returned. If the record is
  successfully retrieved, true is returned.

  \sa next() first() last() seek() pos() isFinished() isValid()
*/

bool QSparqlResult::previous()
{
    if (!isFinished())
        return false;

    bool b = false;
    switch (pos()) {
    case QSparql::BeforeFirstRow:
        return false;
    case QSparql::AfterLastRow:
        b = fetchLast();
        return b;
    default:
        if (!fetchPrevious()) {
            setPos(QSparql::BeforeFirstRow);
            return false;
        }
        return true;
    }
}

/*!
  Retrieves the first record in the result, if available, and
  positions the query on the retrieved record. Note that the result
  must be in the \l{isFinished()}{finished} state before calling this
  function or it will do nothing and return false.  Returns true if
  successful. If unsuccessful the query position is set to an invalid
  position and false is returned.

  \sa next() previous() last() seek() pos() isFinished() isValid()
 */

bool QSparqlResult::first()
{
    if (!isFinished())
        return false;
    return fetchFirst();
}

/*!

  Retrieves the last record in the result, if available, and positions
  the query on the retrieved record. Note that the result must be in
  the \l{isFinished()}{finished} state before calling this function or
  it will do nothing and return false.  Returns true if successful. If
  unsuccessful the query position is set to an invalid position and
  false is returned.

  \sa next() previous() first() seek() pos() isFinished() isValid()
*/

bool QSparqlResult::last()
{
    if (!isFinished())
        return false;
    return fetchLast();
}

/*!
  \fn int QSparqlResult::size() const

  Returns the size of the result (number of rows returned), or -1 if
  the size cannot be determined or if the database does not support
  reporting information about query sizes. If the query is not
  finished (isFinished() returns false), -1 is returned.

  \sa isFinished() QSparqlDriver::hasFeature()
*/

QVariant QSparqlResult::value(int i) const
{
    if (!isValid())
        return QVariant();
    return data(i);
}

/*!
    This function is provided for derived classes to set the
    internal (zero-based) row position to \a index.

    \sa pos()
*/

void QSparqlResult::setPos(int index)
{
    d->idx = index;
}


/*!
    This function is provided for derived classes to set the last
    error to \a error.

    \sa lastError()
*/

void QSparqlResult::setLastError(const QSparqlError &error)
{
    d->error = error;
}


/*!
    Returns true if there is an error associated with the result.
*/

bool QSparqlResult::hasError() const
{
    return d->error.isValid();
}

/*!
    Returns the last error associated with the result.
*/

QSparqlError QSparqlResult::lastError() const
{
    return d->error;
}

/*!
    \fn void QSparqlResult::finished()

    This signal is emitted when the QSparqlResult has finished retrieving its
    data or when there was an error.
*/

/*!
    \fn void QSparqlResult::dataReady(int totalCount)

    This signal is emitted when a query has fetched data. The \a
    totalCount is the row count of the data set after the new data has
    arrived.
*/

/*!
    \fn QVariant QSparqlResult::data(int index) const

    Returns the data for field \a index in the current row as
    a QVariant. This function is only called if the result is in
    a finished state and is positioned on a valid record and \a index is
    non-negative. Derived classes must reimplement this function and
    return the value of field \a index, or QVariant() if it cannot be
    determined.
*/

/*!
    \fn bool QSparqlResult::fetch(int index)

    Positions the result to an arbitrary (zero-based) row \a index.

    This function is only called if the result is in a finished state.
    Derived classes must reimplement this function and position the
    result to the row \a index, and call setPos() with an appropriate
    value. Return true to indicate success, or false to signify
    failure.

    \sa isFinished(), fetchFirst(), fetchLast(), fetchNext(), fetchPrevious()
*/

/*!
    \fn bool QSparqlResult::fetchFirst()

    Positions the result to the first record (row 0) in the result.

    This function is only called if the result is in a finished state.
    Derived classes must reimplement this function and position the
    result to the first record, and call setPos() with an appropriate
    value. Return true to indicate success, or false to signify
    failure.

    \sa fetch(), fetchLast()
*/

/*!
    \fn bool QSparqlResult::fetchLast()

    Positions the result to the last record (last row) in the result.

    This function is only called if the result is in an finished state.
    Derived classes must reimplement this function and position the
    result to the last record, and call setPos() with an appropriate
    value. Return true to indicate success, or false to signify
    failure.

    \sa fetch(), fetchFirst()
*/

/*!
    Positions the result to the next available record (row) in the
    result.

    This function is only called if the result is in a finished
    state. The default implementation calls fetch() with the next
    index. Derived classes can reimplement this function and position
    the result to the next record in some other way, and call setPos()
    with an appropriate value. Return true to indicate success, or
    false to signify failure.

    \sa fetch(), fetchPrevious()
*/

bool QSparqlResult::fetchNext()
{
    return fetch(pos() + 1);
}

/*!
    Positions the result to the previous record (row) in the result.

    This function is only called if the result is in a finished state.
    The default implementation calls fetch() with the previous index.
    Derived classes can reimplement this function and position the
    result to the next record in some other way, and call setPos()
    with an appropriate value. Return true to indicate success, or
    false to signify failure.
*/

bool QSparqlResult::fetchPrevious()
{
    return fetch(pos() - 1);
}

/*!
    Returns the current record if the query is finished; otherwise
    returns an empty QSparqlBindingSet.

    The default implementation always returns an empty QSparqlBindingSet.

    \sa isFinished()
*/
QSparqlBindingSet QSparqlResult::bindingSet() const
{
    return QSparqlBindingSet();
}

QT_END_NAMESPACE
