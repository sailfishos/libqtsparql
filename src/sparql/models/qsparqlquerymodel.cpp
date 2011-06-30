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

#include "qsparqlquerymodel.h"
#include "qsparqlquerymodel_p.h"

#include <qdebug.h>
#include "private/qsparqldriver_p.h"
#include <qsparqlbinding.h>
#include <qsparqlresult.h>
#include <QRegExp>
QT_BEGIN_NAMESPACE

// #define QSPARQL_PREFETCH 255

void QSparqlQueryModelPrivate::addData(int totalResults)
{
    if (newQuery) {
        beginQuery(totalResults);
        newQuery = false;
    } else {
        prefetch(totalResults);
    }
}

void QSparqlQueryModelPrivate::queryFinished()
{
    if (result->hasError())
        q->setLastError(result->lastError());

    Q_EMIT q->finished();
}

void QSparqlQueryModelPrivate::beginQuery(int totalResults)
{
    // This function will only be called when result is a valid
    // pointer.
    result->first();
    QSparqlResultRow newResultRow = result->current();
    bool columnsChanged = (newResultRow != resultRow);
    // bool hasQuerySize = connection->hasFeature(QSparqlConnection::QuerySize);
    bool hasNewData = (newResultRow != QSparqlResultRow()) || !result->hasError();

    if (colOffsets.size() != newResultRow.count() || columnsChanged)
        initColOffsets(newResultRow.count());

    resultRow = newResultRow;
    atEnd = false;

    if (columnsChanged && hasNewData)
        q->reset();

    QModelIndex newBottom;
    newBottom = q->createIndex(totalResults - 1, resultRow.count() - 1);
    q->beginInsertRows(QModelIndex(), 0, qMax(0, newBottom.row()));
    bottom = q->createIndex(totalResults - 1, columnsChanged ? 0 : resultRow.count() - 1);
    q->endInsertRows();
    bottom = newBottom;

    q->queryChange();
}

void QSparqlQueryModelPrivate::prefetch(int limit)
{
    if (atEnd || limit <= bottom.row() || bottom.column() == -1 || !result)
        return;

    QModelIndex newBottom;

    result->setPos(limit - 1);
    newBottom = q->createIndex(limit - 1, bottom.column());

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

void QSparqlQueryModelPrivate::findRoleNames()
{
    QString queryString = query.preparedQueryText();
    roleNames.clear();
    QList<QString> uniqueNames;
    QStringList stringList = queryString.split(QLatin1Char(' '));
    bool doIgnore = false;
    bool processFunction = false;
    bool functionReady = false;
    int parenthesesCount = 0;
    QString storeFunction;
    Q_FOREACH(QString word, stringList) {
            // process string-joins correctly
            // string-join ( (?role1, ?role2) , arg2 )
            // so ?role1 will be the correct property name, ignore role2
            if (processFunction || word.contains(QLatin1String("string-join"))) {
                processFunction = true;
                // get the entire function first
                storeFunction += word;
                if (word.contains(QLatin1Char('('))) {
                    parenthesesCount += word.count(QLatin1Char('('));
                }
                if (word.contains(QLatin1Char(')'))) {
                    parenthesesCount-= word.count(QLatin1Char(')'));
                    if(parenthesesCount==0) {
                        functionReady = true;
                    }
                }
                if (functionReady) {
                    // qDebug() << "Stored function = " << storeFunction;
                    // we only want the first one
                    QString::SectionFlag flag = QString::SectionIncludeLeadingSep;
                    QRegExp match(QLatin1String("[?]|[$]"));
                    word = storeFunction.section(match,1, 1, flag);
                    processFunction = false;
                    functionReady = false;
                    // fall through and process normally
                }
            }

            if (!processFunction) {
                if (!doIgnore && (word.at(0) == QLatin1Char('?') || word.at(0) == QLatin1Char('$'))
                && (!word.contains(QLatin1Char(';')) && !word.contains(QLatin1Char(':')))) {
                    //remove the ? or $
                    word.remove(0,1);
                    // select ?u{?u a ... is valid, need to make sure
                    // this is dealt with
                    if (word.contains(QLatin1Char('{'))) {
                        word = word.split(QLatin1Char('{')).at(0);
                    }
                    QRegExp cleanUp(QLatin1String("[,]|[{]|[}]|[.]"));
                    word.replace(cleanUp,QLatin1String(""));
                    if (!uniqueNames.contains(word)) {
                        uniqueNames.append(word);
                    }
                }
                // look for parentheses to avoid incorrectly adding things like ?foo in
                // 'nie:url( ?foo ) AS ?bar' as a role name
                if (word.contains(QLatin1Char('(')) || word.contains(QLatin1Char('{'))) {
                    doIgnore = true;
                    parenthesesCount += word.count(QLatin1Char('('));
                    parenthesesCount += word.count(QLatin1Char('{'));
                }
                if (word.contains(QLatin1Char(')')) || word.contains(QLatin1Char('}'))) {
                    parenthesesCount -= word.count(QLatin1Char('('));
                    parenthesesCount -= word.count(QLatin1Char('{'));
                    if (parenthesesCount == 0) {
                        doIgnore = false;
                    }
                }
            }
    }

    int roleCounter = Qt::UserRole + 1;
    Q_FOREACH(QString word, uniqueNames) {
        roleNames[roleCounter++] = word.toLatin1();
    }
    q->setRoleNames(roleNames);
    qDebug() << "unique = " << roleNames;
}

/*!
    \class QSparqlQueryModel
    \brief The QSparqlQueryModel class provides a read-only data model
    for SPARQL result sets.

    QSparqlQueryModel is a high-level interface for executing SPARQL
    statements and traversing the result set. It is built on top of
    the lower-level QSparqlQuery and can be used to provide data to
    view classes such as QTableView.

    QSparqlQueryModel can also be used to access a connection
    programmatically, without binding it to a view.

    \sa QSparqlQuery
*/

/*!
    \fn void QSparqlQueryModel::finished()

    This signal is emitted when the QSparqlResult, used by the model, has
    finished retrieving its data or when there was an error.
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

/*! \fn int QSparqlQueryModel::rowCount(const QModelIndex &parent) const

    If the connection supports returning the size of a query
    (see QSparqlConnection::hasFeature()), the number of rows of the current
    query is returned. Otherwise, returns the number of rows
    currently cached on the client.

    \a parent should always be an invalid QModelIndex.

    \sa canFetchMore(), QSparqlConnection::hasFeature()
 */
int QSparqlQueryModel::rowCount(const QModelIndex &index) const
{
    return index.isValid() ? 0 : d->bottom.row() + 1;
}

/*!
 *  Returns the number of columns, which is the number of variables
 *  in the select part of the query
 */
int QSparqlQueryModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : d->resultRow.count();
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

    int userRole = 0;
    if (role >= Qt::UserRole+1) {
        userRole = role;
        role = Qt::DisplayRole;
    }

    QVariant v;
    if (role & ~(Qt::DisplayRole | Qt::EditRole))
        return v;

    QModelIndex dItem;
    // if we have a userRole we need to set the correct column
    if (userRole) {
        int columnOffset = userRole - (Qt::UserRole + 1);
        QModelIndex newItem = index(item.row(), item.column()+columnOffset, item.parent());
        dItem = indexInQuery(newItem);
    } else {
        dItem = indexInQuery(item);
    }

    if (dItem.row() > d->bottom.row())
        const_cast<QSparqlQueryModelPrivate *>(d)->prefetch(dItem.row());

    if (!d->result->setPos(dItem.row())) {
        d->error = d->result->lastError();
        return v;
    }

    return d->result->binding(dItem.column()).value();
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
        if (role == Qt::DisplayRole && d->resultRow.count() > section)
            return d->resultRow.variableName(section);
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
    Executes the query \a query for the given connection connection \a
    connection. If no connection is specified, the default connection is used.

    lastError() can be used to retrieve verbose information if there
    was an error setting the query.

    \sa query(), queryChange(), lastError()
*/
void QSparqlQueryModel::setQuery(const QSparqlQuery &query, QSparqlConnection &connection)
{
    d->query = query;
    d->findRoleNames();
    bool mustClearModel = d->bottom.isValid();
    if (mustClearModel) {
        d->atEnd = true;
        beginRemoveRows(QModelIndex(), 0, qMax(d->bottom.row(), 0));
        d->bottom = QModelIndex();
        endRemoveRows();
    }

    d->connection = &connection;
    delete d->result;
    d->result = connection.exec(query);
    d->newQuery = true;
    connect(d->result, SIGNAL(finished()), d, SLOT(queryFinished()));
    connect(d->result, SIGNAL(dataReady(int)), d, SLOT(addData(int)));

}

/*!
    Clears the model and releases any acquired resources. After this
    function, the model is not usable until setQuery() has been called.

    \sa setQuery()
*/
void QSparqlQueryModel::clear()
{
    d->error = QSparqlError();
    d->atEnd = true;

    delete d->result; // TODO: is this ok?
    d->result = 0;

    // TODO: or should we just delete d; d = 0;

    d->resultRow.clear();
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

    if (orientation != Qt::Horizontal || section < 0)
        return false;

    if (d->headers.size() <= section)
        d->headers.resize(qMax(section + 1, 16));
    d->headers[section][role] = value;
    Q_EMIT headerDataChanged(orientation, section, section);
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

    \sa QSparqlResultRow::isEmpty()
*/
QSparqlResultRow QSparqlQueryModel::resultRow(int row) const
{
    if (!d->result)
        return QSparqlResultRow();

    if (row < 0)
        return d->resultRow;

    if (!d->result->setPos(row))
        return d->resultRow;

    return d->result->current();
}

/*! \overload

    Returns an empty record containing information about the fields
    of the current query.

    If the model is not initialized, an empty record will be
    returned.

    \sa QSparqlResultRow::isEmpty()
 */
QSparqlResultRow QSparqlQueryModel::resultRow() const
{
    return d->resultRow;
}

/*!
    Inserts \a count columns into the model at position \a column. The
    \a parent parameter must always be an invalid QModelIndex, since
    the model does not support parent-child relationships.

    Returns true if \a column is within bounds; otherwise returns false.

    By default, inserted columns are empty. To fill them with data,
    reimplement data() and handle any inserted column separately.

    \sa removeColumns()
*/
bool QSparqlQueryModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    if (count <= 0 || parent.isValid() || column < 0 || column > d->resultRow.count())
        return false;

    beginInsertColumns(parent, column, column + count - 1);
    for (int c = 0; c < count; ++c) {
        QSparqlBinding binding;
        d->resultRow.append(binding);
        // d->resultRow.insert(column, binding);
        if (d->colOffsets.size() < d->resultRow.count()) {
            int nVal = d->colOffsets.isEmpty() ? 0 : d->colOffsets[d->colOffsets.size() - 1];
            d->colOffsets.append(nVal);
            Q_ASSERT(d->colOffsets.size() >= d->resultRow.count());
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
    if (count <= 0 || parent.isValid() || column < 0 || column >= d->resultRow.count())
        return false;

    beginRemoveColumns(parent, column, column + count - 1);

    int i;
//    for (i = 0; i < count; ++i)
//        d->resultRow.remove(column);
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
    if (item.column() < 0 || item.column() >= d->resultRow.count())
        return QModelIndex();
    return createIndex(item.row(), item.column() - d->colOffsets[item.column()],
                       item.internalPointer());
}

QT_END_NAMESPACE
