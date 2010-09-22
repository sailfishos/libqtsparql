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

#include "qvariant.h"
#include "qhash.h"
#include "qregexp.h"
#include "qsparqlerror.h"
#include "qsparqlbinding.h"
#include "qsparqlresultrow.h"
#include "qsparqlresult.h"
#include "qvector.h"
#include "qsparqldriver_p.h"
#include <QDebug>

QT_BEGIN_NAMESPACE

class QSparqlResultPrivate
{
public:
    QSparqlResultPrivate()
    : idx(QSparql::BeforeFirstRow), statementType(QSparqlQuery::SelectStatement), 
      boolValue(false)
    {}

public:
    int idx;
    QString sparql; // FIXME: needed? rdale: yes it is needed (for debugging)
    QSparqlQuery::StatementType statementType;
    QSparqlError error;
    bool boolValue;
};

/*!
    \class QSparqlResult
    \brief The QSparqlResult class provides an abstract interface for
    accessing the asynchronous results of an executed QSparqlQuery.

    \ingroup database
    \inmodule QtSparql

    When QSparqlConnection::exec() is called, it asynchronously begins
    the execution of the given query. The returned result is in an
    unfinished state so that isFinished() returns false. When the execution
    finished, QSparqlResult emits the finished() signal and sets
    itself to finished state.

    Initially, QSparqlResult is positioned on an invalid position. It
    must be navigated to a valid position (so that isValid() returns
    true) before values can be retrieved.

    Navigating the result is performed with the following functions:

    - next()
    - previous()
    - first()
    - last()
    - seek()

    Retrieving the data is performed with the following functions:
    - current()
    - binding()
    - value()
*/

/* this doc not included in doxygen...
    If you are implementing your own SPARQL driver (by subclassing
    QSparqlDriver), you will need to provide your own QSparqlResult
    subclass that implements all the pure virtual functions and other
    virtual functions that you need.

*/

/* this doc not included in doxygen...
    These functions allow the programmer to move forward, backward
    or arbitrarily through the rows returned by the query. If you
    only need to move forward through the results (e.g., by using
    next()), you can use setForwardOnly(), which will save a
    significant amount of memory overhead and improve performance on
    some databases. Once a finished query is positioned on a valid
    row, data can be retrieved using value(). All data is
    transferred from the SQL backend using QVariants.

    For example:

    \snippet doc/src/snippets/sparqlconnection/sparqlconnection.cpp 7

    To access the data returned by a query, use value(int). Each
    field in the data returned by a \c SELECT statement is accessed
    by passing the field's position in the statement, starting from
    0. This makes using \c{SELECT *} queries inadvisable because the
    order of the fields returned is indeterminate.

    For the sake of efficiency, there are no functions to access a
    field by name (unless you use prepared queries with names, as
    explained below). To convert a field name into an index, use
    resultRow().\l{QSparqlResultRow::indexOf()}{indexOf()}, for example:

    \snippet doc/src/snippets/sparqlconnection/sparqlconnection.cpp 8

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

void QSparqlResult::setQuery(const QString &query)
{
    d->sparql = query;
}

void QSparqlResult::setStatementType(QSparqlQuery::StatementType type)
{
    d->statementType = type;
}

bool QSparqlResult::isTable() const
{
    return d->statementType == QSparqlQuery::SelectStatement;
}

/*!
    Returns true if the statement is a CONSTRUCT or DESCRIBE query 
    returning a graph. Each QSparqlResultRow in a graph result hasError
    three QSParqlBinding values, named 's', 'p' and 'o' corresponding
    to triples with Subject, Predicate and Object values

    \sa isTable() isBool()
*/

bool QSparqlResult::isGraph() const
{
    return d->statementType == QSparqlQuery::ConstructStatement 
            || d->statementType == QSparqlQuery::DescribeStatement;
}

/*!
    Returns true if the statement is an ASK query returning a 
    boolean value

    \sa isTable() isGraph()
*/

/*!
    Returns true if the statement is an ASK query returning a 
    boolean value

    \sa isTable() isGraph() boolValue()
*/

bool QSparqlResult::isBool() const 
{
    return d->statementType == QSparqlQuery::AskStatement;
}

/*!
    Returns the boolean result of an ASK query

    \sa isBool() setBoolValue()
*/

bool QSparqlResult::boolValue() const
{
    return d->boolValue;
}

/*!
    Set the boolean result of an ASK query

    \sa isBool() boolValue()
*/
void QSparqlResult::setBoolValue(bool v)
{
    d->boolValue = v;
}

/*!
    Returns the current internal position of the query. The first
    row is at position zero. If the position is invalid, the
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

/*!
    Suspends the execution of the calling thread until all the query results 
    have arrived. After this function returns, isFinished() should return true, 
    indicating the result's contents are ready to be processed.

    \sa isFinished()
*/
void QSparqlResult::waitForFinished()
{
}

/*!
    Returns true if the pending query has finished processing and the result has been 
    received. If this function returns true, the hasError() and lastError() 
    methods should return valid information.

    Note that this function only changes state if you call waitForFinished(),
    or if an external event happens, which in general only happens if 
    you return to the event loop execution.

    \sa waitForFinished() lastError() error()
*/

bool QSparqlResult::isFinished() const
{
    return false;
}

/*!
  Retrieves the result row at position \a index, if available, and
  positions the query on the retrieved row. The first result row is at
  position 0. Note that the query must be in an \l{finished()}
  {finished} state and isSelect() must return true before calling this
  function.

  If \a relative is false (the default), the following rules apply:

  \list

  \o If \a index is negative, the result is positioned before the
  first row and false is returned.

  \o Otherwise, an attempt is made to move to the row at position
  \a index. If the row at position \a index could not be retrieved,
  the result is positioned after the last row and false is
  returned. If the row is successfully retrieved, true is returned.

  \endlist

  If \a relative is true, the following rules apply:

  \list

  \o If the result is currently positioned before the first row or
  on the first row, and \a index is negative, there is no change,
  and false is returned.

  \o If the result is currently located after the last row, and \a
  index is positive, there is no change, and false is returned.

  \o If the result is currently located somewhere in the middle, and
  the relative offset \a index moves the result below zero, the result
  is positioned before the first row and false is returned.

  \o Otherwise, an attempt is made to move to the row \a index
  rows ahead of the current row (or \a index rows behind the
  current row if \a index is negative). If the row at offset \a
  index could not be retrieved, the result is positioned after the
  last row if \a index >= 0, (or before the first row if \a
  index is negative), and false is returned. If the row is
  successfully retrieved, true is returned.

  \endlist

  \sa next() previous() first() last() pos() isFinished() isValid()
*/

/*
bool QSparqlResult::seek(int index)
{
//    if (!isFinished())
//        return false;
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
*/

/*!

  Retrieves the next row in the result, if available, and positions
  the query on the retrieved row. Note that the result must be in
  the \l{isFinished()}{finished} state and isSelect() must return true
  before calling this function or it will do nothing and return false.

  The following rules apply:

  \list

  \o If the result is currently located before the first row,
  e.g. immediately after a query is executed, an attempt is made to
  retrieve the first row.

  \o If the result is currently located after the last row, there
  is no change and false is returned.

  \o If the result is located somewhere in the middle, an attempt is
  made to retrieve the next row.

  \endlist

  If the row could not be retrieved, the result is positioned after
  the last row and false is returned. If the row is successfully
  retrieved, true is returned.

  \sa previous() first() last() seek() pos() isFinished() isValid()
*/
bool QSparqlResult::next()
{
    // qDebug() << "QSparqlResult::next():" << pos() << " size:" << size();

    bool b = false;
    switch (pos()) {
    case QSparql::BeforeFirstRow:
        b = first();
        return b;
    case QSparql::AfterLastRow:
        return false;
    default:
        return setPos(pos() + 1);
    }
}

/*!

  Retrieves the previous row in the result, if available, and
  positions the query on the retrieved row. Note that the result
  must be in the \l{isFinished()}{finished} state before calling this
  function or it will do nothing and return false.

  The following rules apply:

  \list

  \o If the result is currently located before the first row, there
  is no change and false is returned.

  \o If the result is currently located after the last row, an
  attempt is made to retrieve the last row.

  \o If the result is somewhere in the middle, an attempt is made to
  retrieve the previous row.

  \endlist

  If the row could not be retrieved, the result is positioned
  before the first row and false is returned. If the row is
  successfully retrieved, true is returned.

  \sa next() first() last() seek() pos() isFinished() isValid()
*/

bool QSparqlResult::previous()
{
    bool b = false;
    switch (pos()) {
    case QSparql::BeforeFirstRow:
        return false;
    case QSparql::AfterLastRow:
        b = last();
        return b;
    default:
        setPos(pos() - 1);
        if (pos() < 0) {
            setPos(QSparql::BeforeFirstRow);
            return false;
        }
        return true;
    }
}

/*!
  Retrieves the first row in the result, if available, and
  positions the query on the retrieved row. Note that the result
  must be in the \l{isFinished()}{finished} state before calling this
  function or it will do nothing and return false.  Returns true if
  successful. If unsuccessful the query position is set to an invalid
  position and false is returned.

  \sa next() previous() last() seek() pos() isFinished() isValid()
 */

bool QSparqlResult::first()
{
    if (pos() == 0)
        return true;

    setPos(0);
    return size() > 0;
}

/*!

  Retrieves the last row in the result, if available, and positions
  the query on the retrieved row. Note that the result must be in
  the \l{isFinished()}{finished} state before calling this function or
  it will do nothing and return false.  Returns true if successful. If
  unsuccessful the query position is set to an invalid position and
  false is returned.

  \sa next() previous() first() seek() pos() isFinished() isValid()
*/

bool QSparqlResult::last()
{
    setPos(size() - 1);
    return pos() >= 0;
}

/*!
  \fn int QSparqlResult::size() const

  Returns the size of the result (number of rows returned), or -1 if
  the size cannot be determined or if the database does not support
  reporting information about query sizes. If the query is not
  finished (isFinished() returns false), -1 is returned.

  \sa isFinished() QSparqlDriver::hasFeature()
*/

/*!
    Returns the value of binding \a index in the current result row.

    The binding values are numbered from left to right using the text of the
    \c SELECT statement, e.g. in

    \snippet doc/src/snippets/code/src_sparql_kernel_qsparqlquery.cpp 0

    field 0 is \c forename and field 1 is \c
    surname. Using \c{SELECT *} is not recommended because the order
    of the fields in the query is undefined.

    An invalid QVariant is returned if binding value \a index does not
    exist, if the query is inactive, or if the query is positioned on
    an invalid result row.

    \sa previous() next() first() last() seek() isValid()
*/

QSparqlBinding QSparqlResult::binding(int i) const
{
    return QSparqlBinding();
}

QVariant QSparqlResult::value(int i) const
{
    return QVariant();
}

/*!
    This function is provided for derived classes to set the
    internal (zero-based) row position to \a index.

    \sa pos()
*/

bool QSparqlResult::setPos(int pos)
{
    if (pos >= size())
        return false;

    d->idx = pos;
    return true;
}


/*!
    This function is provided for derived classes to set the last
    error to \a error.

    \sa lastError() hasError()
*/

void QSparqlResult::setLastError(const QSparqlError &error)
{
    d->error = error;
}


/*!
    Returns true if there is an error associated with the result.
    
    \sa setLastError() lastError()
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
    \fn void QSparqlResult::dataReady(int totalRows)

    This signal is emitted when a query has fetched data. The \a
    totalRows is the row count of the data set after the new data has
    arrived.
*/

/*!
    \fn QVariant QSparqlResult::data(int index) const

    Returns the data for field \a index in the current row as
    a QVariant. This function is only called if the result is in
    a finished state and is positioned on a valid row and \a index is
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

    Positions the result to the first row (row 0) in the result.

    This function is only called if the result is in a finished state.
    Derived classes must reimplement this function and position the
    result to the first row, and call setPos() with an appropriate
    value. Return true to indicate success, or false to signify
    failure.

    \sa fetch(), fetchLast()
*/

/*!
    \fn bool QSparqlResult::fetchLast()

    Positions the result to the last row (last row) in the result.

    This function is only called if the result is in an finished state.
    Derived classes must reimplement this function and position the
    result to the last row, and call setPos() with an appropriate
    value. Return true to indicate success, or false to signify
    failure.

    \sa fetch(), fetchFirst()
*/

/*!
  Returns a QSparqlResultRow containing the binding values information for the
  current query. If the query points to a valid row (isValid() returns
  true), the result row is populated.  An empty
  result row is returned when there is no result at the current position

  To retrieve just the values from a query, value() should be used since
  its index-based lookup is faster. Use QSparqlResultRow::binding() to
  retrieve the value along with meta data, such as the data type URI
  or language tag for literals.

  In the following example, a \c{SELECT * FROM} query is executed.
  Since the order of the columns is not defined, QSparqlResultRow::indexOf()
  is used to obtain the index of a column. FIXME: "select * from"?

  \snippet doc/src/snippets/code/src_sparql_kernel_qsparqlquery.cpp 1

  \sa value() pos() setPos()
*/

QSparqlResultRow QSparqlResult::current() const
{
    return QSparqlResultRow();
}

QT_END_NAMESPACE
