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

#include "qsparqlquery.h"

#include "qsparqlresultrow.h"
#include "qsparqlbinding.h"

//#define QT_DEBUG_SQL

#include "qatomic.h"
#include "qvector.h"
#include "qmap.h"
#include "qdebug.h"

QT_BEGIN_NAMESPACE

// This struct stores a "placholder name" - "its position in the query string"
// pair.  If the same placeholder occurs multiple times, multiple QHolders are
// created for it.
struct QHolder {
    QHolder(const QString& hldr = QString(), int index = -1):
        holderName(hldr), holderPos(index) {}
    bool operator==(const QHolder& h) const
        { return h.holderPos == holderPos && h.holderName == holderName; }
    bool operator!=(const QHolder& h) const
        { return h.holderPos != holderPos || h.holderName != holderName; }

    QString holderName;
    int holderPos;
};

struct QSparqlQueryPrivate
{
    QSparqlQueryPrivate(const QString& q = QString(),
                        QSparqlQuery::StatementType t = QSparqlQuery::SelectStatement)
        : query(q), type(t)
    {
        findPlaceholders();
    }

    ~QSparqlQueryPrivate();

    void findPlaceholders();

    QAtomicInt ref;
    QString query;
    QSparqlQuery::StatementType type;

    QVector<QSparqlBinding> values; // bound values
    typedef QHash<QString, int> IndexMap;
    IndexMap indexes; // placeholder names -> indexes in the 'values' vector
    typedef QVector<QHolder> QHolderVector;
    QHolderVector holders;
};

static bool qIsAlnum(QChar ch)
{
    uint u = uint(ch.unicode());
    // matches [a-zA-Z0-9_]
    return u - 'a' < 26 || u - 'A' < 26 || u - '0' < 10 || u == '_';
}

/*!
\internal
*/
QSparqlQueryPrivate::~QSparqlQueryPrivate()
{
}

/*!
    \class QSparqlQuery
    \brief The QSparqlQuery class provides a means of executing and
    manipulating SPARQL statements.

    \ingroup database
    \ingroup shared
    \inmodule QtSparql

    QSparqlQuery encapsulates the functionality involved in creating
    SPARQL queries which are executed on a \l QSparqlConnection.

    It can also be used to execute commands specific to an RDF store which
    are not standard SPARQL.

    To execute the QSparqlQuery, pass it to QSparqlConnection::exec().

    QSparqlQuery supports binding of parameter values to placeholders;
    ?: and $: are used as placeholder markers.

    You can retrieve the values of all the fields in a single variable
    (a map) using boundValues().

    \section bindingapproaches Approaches to Binding Values

    QSparqlQuery supports replacing placeholders (marked with ?: or $:) with strings.

    \dontinclude contacts/main.cpp
    \skip QSparqlQuery nameQuery(
    \until nameFamily
    \skipline bindValue(

    When replacing strings, the quotes surrounding strings are inserted
    automatically. Quotes (" and ') inside the strings are escaped.

    \sa QSparqlConnection, QSparqlQueryModel, QSparqlResult, QSparqlResultRow, QSparqlBinding, QVariant
*/

/*!
    \enum QSparqlQuery::StatementType

    This enum contains a list of SPARQL statement (or clause) types the
    driver can create.

    \value SelectStatement A SPARQL \c SELECT statement
    \value AskStatement  A SPARQL \c ASK statement
    \value ConstructStatement A SPARQL \c CONSTRUCT statement
    \value DescribeStatement A SPARQL \c DESCRIBE statement
    \value InsertStatement A SPARQL \c INSERT statement
    \value DeleteStatement A SPARQL \c DELETE statement

*/

/*!
    Constructs a QSparqlQuery object which uses the QSparqlResult \a result
    to communicate with a database.
*/

QSparqlQuery::QSparqlQuery(const QString& query, StatementType type)
{
    d = new QSparqlQueryPrivate(query, type);
    d->ref.ref();
}

/*!
    Destroys the object and frees any allocated resources.
*/

QSparqlQuery::~QSparqlQuery()
{
    if (!d->ref.deref())
        delete d;
}

/*!
    Constructs a copy of \a other.
*/


QSparqlQuery::QSparqlQuery(const QSparqlQuery& other)
{
    d = other.d;
    d->ref.ref();
}

/*!
    Assigns \a other to this object.
*/

QSparqlQuery& QSparqlQuery::operator=(const QSparqlQuery& other)
{
    qAtomicAssign(d, other.d);
    return *this;
}

/*!
    Returns the statement type.
*/
QSparqlQuery::StatementType QSparqlQuery::type() const
{
    return d->type;
}

/*!
    Sets the statement type to \a type.
*/
void QSparqlQuery::setType(StatementType type)
{
    d->type = type;
}

/*!
    Returns the query text without replacements.
*/
QString QSparqlQuery::query() const
{
    return d->query;
}

/*!
    Sets the query text to \a query. The bound placeholders are not affected.
*/
void QSparqlQuery::setQuery(const QString& query)
{
    d->query = query;
    d->findPlaceholders();
}

/*!
    Finds the placeholders from the query, creates holders for them and inserts
    them into d->holders.
*/

void QSparqlQueryPrivate::findPlaceholders()
{
    holders.clear();
    int n = query.size();

    QChar quoteChar = QChar::Null;
    int i = 0;

    while (i < n) {
        QChar ch = query.at(i);
        QChar prev = (i > 0 ? query.at(i - 1) : QLatin1Char(' '));
        if (ch == QLatin1Char(':') && (prev == QLatin1Char('?') || prev == QLatin1Char('$')) && quoteChar == QChar::Null
                && (i < n - 1 && qIsAlnum(query.at(i + 1)))) {
            int pos = i + 2;
            while (pos < n && qIsAlnum(query.at(pos)))
                ++pos;

            holders.append(QHolder(query.mid(i + 1, pos - i - 1), i - 1));
            i = pos;
        } else {
            if (ch == QLatin1Char('\'') || ch == QLatin1Char('"')) {
                if (quoteChar == QChar::Null) {
                    quoteChar = ch;
                } else if (quoteChar == ch) {
                    quoteChar = QChar::Null;
                }
            }

            ++i;
        }
    }
}

/*!
    Replaces the placeholders with the bound values and returns the resulting
    query.
*/

QString QSparqlQuery::preparedQueryText() const
{
    QString result(d->query);
    int i;
    QString holder;
    int ix;
    // The holders are stored in order and iterated in the reverse order; this
    // way the indices of the earlier holder remain valid when we replace a
    // holder in the string.
    for (i = d->holders.count() - 1; i >= 0; --i) {
        holder = d->holders.at(i).holderName;
        ix = d->indexes.value(holder, -1);
        if (ix == -1) {
            qWarning() << "QSparql: Placeholder" << holder << "not replaced";
            continue;
        }
        // Recycling the value through QSparqlBinding will do the
        // escaping.
        QSparqlBinding binding = d->values.value(ix);
        result = result.replace(d->holders.at(i).holderPos,
                                holder.length() + 2, binding.toString());
    }
    return result;
}

/*!
  Set the placeholder \a placeholder to be bound to value \a val in the
  query. Note that the placeholder mark (\c{?:} or \c{$:}) must not be included
  when specifying the placeholder name.

  \sa bindValue(), boundValue() boundValues()
*/
void QSparqlQuery::bindValue(const QString& placeholder, const QVariant& val)
{
    bindValue(QSparqlBinding(placeholder, val));
}


/*!
  Set the placeholder \a binding.name() to be bound to value \a binding.value()
  in the query. Note that the placeholder mark (\c{?:} or \c{$:}) must not be
  included when specifying the placeholder name.
*/
void QSparqlQuery::bindValue(const QSparqlBinding& binding)
{
    // indexes: QString (placeholder) -> int
    // values: QVector<QSparqlBinding>
    QString placeholder = binding.name();
    int idx = d->indexes.value(placeholder, -1);
    if (idx >= 0) {
        // this placeholder has been assigned an index
        if (d->values.count() <= idx)
            d->values.resize(idx + 1);
        d->values[idx] = binding;
    } else {
        // this placeholder hasn't been assigned an index; assign it now
        d->values.append(binding);
        idx = d->values.count() - 1;
        d->indexes[placeholder] = idx;
    }
}

/*!
  Iterates through the variable name - value pairs from the \a bindings and
  adds them as bindings.
*/
void QSparqlQuery::bindValues(const QSparqlResultRow& bindings)
{
    for (int i = 0; i < bindings.count(); ++i)
        bindValue(bindings.binding(i));
}

/*!
  Clears all bound values.
*/
void QSparqlQuery::unbindValues()
{
    d->values.clear();
}

/*!
  Returns the value for the \a placeholder.

  \sa boundValues() bindValue()
*/
QVariant QSparqlQuery::boundValue(const QString& placeholder) const
{
    int idx = d->indexes.value(placeholder, -1);
    // if index is out of bounds, value() returns a default constructed value
    return d->values.value(idx).value();
}

/*!
  Returns a map of the bound values.

  With named binding, the bound values can be examined in the
  following ways:

  \snippet doc/src/snippets/sparqlconnection/sparqlconnection.cpp 14

  \sa boundValue() bindValue()
*/
QMap<QString, QSparqlBinding> QSparqlQuery::boundValues() const
{
    QMap<QString, QSparqlBinding> map;

    QHash<QString, int>::const_iterator it = d->indexes.constBegin();
    while (it != d->indexes.constEnd()) {
        map[it.key()] = d->values.at(it.value());
        ++it;
    }
    return map;
}

QT_END_NAMESPACE
