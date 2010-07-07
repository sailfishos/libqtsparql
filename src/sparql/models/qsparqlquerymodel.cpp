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

#include "qsparqlquerymodel.h"

#include <qdebug.h>
#include "private/qsparqldriver_p.h"
#include <qsparqlbinding.h>
#include <qsparqlresult.h>

#include "qsparqlquerymodel_p.h"

QT_BEGIN_NAMESPACE

#define QSPARQL_PREFETCH 255

void QSparqlQueryModelPrivate::queryFinished()
{
    // This function will only be called when result is a valid
    // pointer.
    result->first();
    QSparqlBindingSet newBindingSet = result->bindingSet();
    bool columnsChanged = (newBindingSet != result->bindingSet());
    columnsChanged = true;
    bool hasQuerySize = connection->hasFeature(QSparqlConnection::QuerySize);
    bool hasNewData = (newBindingSet != QSparqlBindingSet()) || !result->hasError();

    if (colOffsets.size() != newBindingSet.count() || columnsChanged)
        initColOffsets(newBindingSet.count());

    
    bool mustClearModel = bottom.isValid();
    if (mustClearModel) {
        atEnd = true;
        q->beginRemoveRows(QModelIndex(), 0, qMax(bottom.row(), 0));
        bottom = QModelIndex();
    }

    bindingSet = newBindingSet;

    if (mustClearModel)
        q->endRemoveRows();

    atEnd = false;

    if (columnsChanged && hasNewData)
        q->reset();

//    if (!result->isActive()) {
//        atEnd = true;
//        bottom = QModelIndex();
//        error = result->error();
//        return;
//    }
    QModelIndex newBottom;
    if (hasQuerySize && result->size() > 0) {
        newBottom = q->createIndex(result->size() - 1, bindingSet.count() - 1);
        q->beginInsertRows(QModelIndex(), 0, qMax(0, newBottom.row()));
        bottom = q->createIndex(result->size() - 1, columnsChanged ? 0 : bindingSet.count() - 1);
        atEnd = true;
        q->endInsertRows();
    } else {
        newBottom = q->createIndex(-1, bindingSet.count() - 1);
    }
    bottom = newBottom;

    q->queryChange();

    // fetchMore does the rowsInserted stuff for incremental models
    q->fetchMore();
}

void QSparqlQueryModelPrivate::prefetch(int limit)
{
    if (atEnd || limit <= bottom.row() || bottom.column() == -1 || !result)
        return;

    QModelIndex newBottom;
    const int oldBottomRow = qMax(bottom.row(), 0);

    // try to seek directly
    if (result->seek(limit)) {
        newBottom = q->createIndex(limit, bottom.column());
    } else {
        // have to seek back to our old position for MS Access
        int i = oldBottomRow;
        if (result->seek(i)) {
            while (result->next())
                ++i;
            newBottom = q->createIndex(i, bottom.column());
        } else {
            // empty or invalid query
            newBottom = q->createIndex(-1, bottom.column());
        }
        atEnd = true; // this is the end.
    }
    if (newBottom.row() >= 0 && newBottom.row() > bottom.row()) {
        q->beginInsertRows(QModelIndex(), bottom.row() + 1, newBottom.row());
        bottom = newBottom;
        q->endInsertRows();
    } else {
        bottom = newBottom;
    }

}

QSparqlQueryModelPrivate::~QSparqlQueryModelPrivate()
{
}

void QSparqlQueryModelPrivate::initColOffsets(int size)
{
    colOffsets.resize(size);
    memset(colOffsets.data(), 0, colOffsets.size() * sizeof(int));
}

/*!
    \class QSparqlQueryModel
    \brief The QSparqlQueryModel class provides a read-only data model
    for SPARQL result sets.

    \ingroup connection

    QSparqlQueryModel is a high-level interface for executing SPARQL
    statements and traversing the result set. It is built on top of
    the lower-level QSparqlQuery and can be used to provide data to
    view classes such as QTableView. For example:

    \snippet doc/src/snippets/sqlconnection/sqldatabase.cpp 16

    We set the model's query, then we set up the labels displayed in
    the view header.

    QSparqlQueryModel can also be used to access a connection
    programmatically, without binding it to a view:

    \snippet doc/src/snippets/sqlconnection/sqldatabase.cpp 21

    The code snippet above extracts the \c salary field from record 4 in
    the result set of the query \c{SELECT * from employee}. Assuming
    that \c salary is column 2, we can rewrite the last line as follows:

    \snippet doc/src/snippets/sqlconnection/sqldatabase.cpp 22

    The model is read-only by default. To make it read-write, you
    must subclass it and reimplement setData() and flags(). Another
    option is to use QSparqlTableModel, which provides a read-write
    model based on a single connection table.

    The \l{sql/querymodel} example illustrates how to use
    QSparqlQueryModel to display the result of a query. It also shows
    how to subclass QSparqlQueryModel to customize the contents of the
    data before showing it to the user, and how to create a
    read-write model based on QSparqlQueryModel.

    If the connection doesn't return the amount of selected rows in
    a query, the model will fetch rows incrementally.
    See fetchMore() for more information.

    \sa QSparqlQuery,
        {Model/View Programming}, {Query Model Example}
*/

/*!
    Creates an empty QSparqlQueryModel with the given \a parent.
 */
QSparqlQueryModel::QSparqlQueryModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    d = new QSparqlQueryModelPrivate(this);
}

/*!
    Destroys the object and frees any allocated resources.

    \sa clear()
*/
QSparqlQueryModel::~QSparqlQueryModel()
{
    delete d;
}

/*!
    Fetches more rows from a connection.
    This only affects connections that don't report back the size of a query
    (see QSparqlConnection::hasFeature()).

    To force fetching of the entire connection, you can use the following:

    \snippet doc/src/snippets/code/src_sql_models_qsparqlquerymodel.cpp 0

    \a parent should always be an invalid QModelIndex.

    \sa canFetchMore()
*/
void QSparqlQueryModel::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid())
        return;
    d->prefetch(qMax(d->bottom.row(), 0) + QSPARQL_PREFETCH);
}

/*!
    Returns true if it is possible to read more rows from the connection.
    This only affects connections that don't report back the size of a query
    (see QSparqlDriver::hasFeature()).

    \a parent should always be an invalid QModelIndex.

    \sa fetchMore()
 */
bool QSparqlQueryModel::canFetchMore(const QModelIndex &parent) const
{
    return (!parent.isValid() && !d->atEnd);
}

/*! \fn int QSparqlQueryModel::rowCount(const QModelIndex &parent) const

    If the connection supports returning the size of a query
    (see QSparqlConnection::hasFeature()), the amount of rows of the current
    query is returned. Otherwise, returns the amount of rows
    currently cached on the client.

    \a parent should always be an invalid QModelIndex.

    \sa canFetchMore(), QSparqlConnection::hasFeature()
 */
int QSparqlQueryModel::rowCount(const QModelIndex &index) const
{
    return index.isValid() ? 0 : d->bottom.row() + 1;
}

/*! \reimp
 */
int QSparqlQueryModel::columnCount(const QModelIndex &index) const
{
    return index.isValid() ? 0 : d->bindingSet.count();
}

/*!
    Returns the value for the specified \a item and \a role.

    If \a item is out of bounds or if an error occurred, an invalid
    QVariant is returned.

    \sa lastError()
*/
QVariant QSparqlQueryModel::data(const QModelIndex &item, int role) const
{
    if (!item.isValid() || !d->result)
        return QVariant();

    QVariant v;
    if (role & ~(Qt::DisplayRole | Qt::EditRole))
        return v;

//    if (!d->bindingSet.isGenerated(item.column()))
//        return v;
    QModelIndex dItem = indexInQuery(item);
    if (dItem.row() > d->bottom.row())
        const_cast<QSparqlQueryModelPrivate *>(d)->prefetch(dItem.row());

    if (!d->result->seek(dItem.row())) {
        d->error = d->result->lastError();
        return v;
    }

    return d->result->value(dItem.column());
}

/*!
    Returns the header data for the given \a role in the \a section
    of the header with the specified \a orientation.
*/
QVariant QSparqlQueryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        QVariant val = d->headers.value(section).value(role);
        if (role == Qt::DisplayRole && !val.isValid())
            val = d->headers.value(section).value(Qt::EditRole);
        if (val.isValid())
            return val;
        if (role == Qt::DisplayRole && d->bindingSet.count() > section)
            return d->bindingSet.variableName(section);
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

/*!
    This virtual function is called whenever the query changes. The
    default implementation does nothing.

    query() returns the new query.

    \sa query(), setQuery()
 */
void QSparqlQueryModel::queryChange()
{
    // do nothing
}

/*!
    Resets the model and sets the data provider to be the given \a
    query. Note that the query must be active and must not be
    isForwardOnly().

    lastError() can be used to retrieve verbose information if there
    was an error setting the query.

    \sa query(), QSparqlQuery::isActive(), QSparqlQuery::setForwardOnly(), lastError()
*/
void QSparqlQueryModel::setQuery(const QSparqlQuery &query)
{
    // FIXME: what's this function?
}

/*! \overload

    Executes the query \a query for the given connection connection \a
    db. If no connection is specified, the default connection is used.

    lastError() can be used to retrieve verbose information if there
    was an error setting the query.

    Example:
    \snippet doc/src/snippets/code/src_sql_models_qsparqlquerymodel.cpp 1

    \sa query(), queryChange(), lastError()
*/
void QSparqlQueryModel::setQuery(const QSparqlQuery &query, const QSparqlConnection &connection)
{
    // FIXME: the old result needs to be deleted after the new results are displayed
    // so not here..
//    if (d->result != 0)
//        delete d->result;
    d->connection = &connection;
    d->result = connection.exec(query);
    connect(d->result, SIGNAL(finished()), d, SLOT(queryFinished()));
}

/*!
    Clears the model and releases any acquired resource. After this
    function, the model is not usable until the setQuery() has been called.

    \sa setQuery()
*/
void QSparqlQueryModel::clear()
{
    d->error = QSparqlError();
    d->atEnd = true;

    delete d->result; // TODO: is this ok?
    d->result = 0;

    // TODO: or should we just delete d; d = 0;

    d->bindingSet.clear();
    d->colOffsets.clear();
    d->bottom = QModelIndex();
    d->headers.clear();
}

/*!
    Sets the caption for a horizontal header for the specified \a role to
    \a value. This is useful if the model is used to
    display data in a view (e.g., QTableView).

    Returns true if \a orientation is Qt::Horizontal and
    the \a section refers to a valid section; otherwise returns
    false.

    Note that this function cannot be used to modify values in the
    connection since the model is read-only.

    \sa data()
 */
bool QSparqlQueryModel::setHeaderData(int section, Qt::Orientation orientation,
                                   const QVariant &value, int role)
{
    if (orientation != Qt::Horizontal || section < 0 || columnCount() <= section)
        return false;

    if (d->headers.size() <= section)
        d->headers.resize(qMax(section + 1, 16));
    d->headers[section][role] = value;
    emit headerDataChanged(orientation, section, section);
    return true;
}

/*!
    Returns the QSparqlQuery associated with this model.

    \sa setQuery()
*/
QSparqlQuery QSparqlQueryModel::query() const
{
    return d->query;
}

/*!
    Returns information about the last error that occurred on the
    connection.

    \sa query()
*/
QSparqlError QSparqlQueryModel::lastError() const
{
    return d->error;
}

/*!
   Protected function which allows derived classes to set the value of
   the last error that occurred on the connection to \a error.

   \sa lastError()
*/
void QSparqlQueryModel::setLastError(const QSparqlError &error)
{
    d->error = error;
}

/*!
    Returns the record containing information about the fields of the
    current query. If \a row is the index of a valid row, the record
    will be populated with values from that row.

    If the model is not initialized, an empty record will be
    returned.

    \sa QSparqlBindingSet::isEmpty()
*/
QSparqlBindingSet QSparqlQueryModel::bindingSet(int row) const
{
    if (!d->result)
        return QSparqlBindingSet();

    if (row < 0)
        return d->bindingSet;

    if (!d->result->seek(row))
        return d->bindingSet;

    QSparqlBindingSet bindingSet = d->result->bindingSet();
    for (int i = 0; i < bindingSet.count(); ++i)
        bindingSet.setValue(i, data(createIndex(row, i), Qt::EditRole));
    return bindingSet;
}

/*! \overload

    Returns an empty record containing information about the fields
    of the current query.

    If the model is not initialized, an empty record will be
    returned.

    \sa QSparqlBindingSet::isEmpty()
 */
QSparqlBindingSet QSparqlQueryModel::bindingSet() const
{
    return d->bindingSet;
}

/*!
    Inserts \a count columns into the model at position \a column. The
    \a parent parameter must always be an invalid QModelIndex, since
    the model does not support parent-child relationships.

    Returns true if \a column is within bounds; otherwise returns false.

    By default, inserted columns are empty. To fill them with data,
    reimplement data() and handle any inserted column separately:

    \snippet doc/src/snippets/sqlconnection/sqldatabase.cpp 23

    \sa removeColumns()
*/
bool QSparqlQueryModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    if (count <= 0 || parent.isValid() || column < 0 || column > d->bindingSet.count())
        return false;

    beginInsertColumns(parent, column, column + count - 1);
    for (int c = 0; c < count; ++c) {
        QSparqlBinding field;
//        field.setReadOnly(true);
//        field.setGenerated(false);
        d->bindingSet.insert(column, field);
        if (d->colOffsets.size() < d->bindingSet.count()) {
            int nVal = d->colOffsets.isEmpty() ? 0 : d->colOffsets[d->colOffsets.size() - 1];
            d->colOffsets.append(nVal);
            Q_ASSERT(d->colOffsets.size() >= d->bindingSet.count());
        }
        for (int i = column + 1; i < d->colOffsets.count(); ++i)
            ++d->colOffsets[i];
    }
    endInsertColumns();
    return true;
}

/*!
    Removes \a count columns from the model starting from position \a
    column. The \a parent parameter must always be an invalid
    QModelIndex, since the model does not support parent-child
    relationships.

    Removing columns effectively hides them. It does not affect the
    underlying QSparqlQuery.

    Returns true if the columns were removed; otherwise returns false.
 */
bool QSparqlQueryModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    if (count <= 0 || parent.isValid() || column < 0 || column >= d->bindingSet.count())
        return false;

    beginRemoveColumns(parent, column, column + count - 1);

    int i;
    for (i = 0; i < count; ++i)
        d->bindingSet.remove(column);
    for (i = column; i < d->colOffsets.count(); ++i)
        d->colOffsets[i] -= count;

    endRemoveColumns();
    return true;
}

/*!
    Returns the index of the value in the connection result set for the
    given \a item in the model.

    The return value is identical to \a item if no columns or rows
    have been inserted, removed, or moved around.

    Returns an invalid model index if \a item is out of bounds or if
    \a item does not point to a value in the result set.

    \sa QSparqlTableModel::indexInQuery(), insertColumns(), removeColumns()
*/
QModelIndex QSparqlQueryModel::indexInQuery(const QModelIndex &item) const
{
    if (item.column() < 0 || item.column() >= d->bindingSet.count())
        return QModelIndex();
    return createIndex(item.row(), item.column() - d->colOffsets[item.column()],
                       item.internalPointer());
}

QT_END_NAMESPACE
