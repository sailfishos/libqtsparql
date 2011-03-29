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

#include "qsparqlresultrow.h"

#include "qdebug.h"
#include "qstringlist.h"
#include "qatomic.h"
#include "qsparqlbinding.h"
#include "qstring.h"
#include "qvector.h"

QT_BEGIN_NAMESPACE

class QSparqlResultRowPrivate
{
public:
    QSparqlResultRowPrivate();
    QSparqlResultRowPrivate(const QSparqlResultRowPrivate &other);

    inline bool contains(int index) { return index >= 0 && index < bindings.count(); }

    QVector<QSparqlBinding> bindings;
    QAtomicInt ref;
};

QSparqlResultRowPrivate::QSparqlResultRowPrivate()
{
    ref = 1;
}

QSparqlResultRowPrivate::QSparqlResultRowPrivate(const QSparqlResultRowPrivate &other): bindings(other.bindings)
{
    ref = 1;
}

/*!
    \class QSparqlResultRow

    \brief The QSparqlResultRow class encapsulates a row in the results of a
    query.

    \ingroup database
    \ingroup shared
    \inmodule QtSparql

    The QSparqlResultRow class encapsulates the functionality and
    characteristics of a row of results returned by a QSparqlQuery.  A
    QSparqlResultRow is a set of (name, value) pairs (bindings).
    QSparqlResultRow supports adding and removing bindings as well as setting
    and retrieving binding values.

    The values of a QSparqlResultRow can be set by name or position with
    setValue(); if you want to set a binding to null use setValue() with
    QVariant().  To find the position of a binding by name use indexOf(), and to
    find the name of a binding at a particular position use bindingName().  Use
    binding() to retrieve a QSparqlBinding object for a given binding. Use
    contains() to see if the QSparqlResultRow contains a particular binding
    name.

    A QSparqlResultRow can have bindings added with append() or insert(),
    replaced with replace(), and removed with remove(). All the bindings can be
    removed with clear(). The number of bindings is given by count(); all their
    values can be cleared (to null) using clearValues().

    \sa QSparqlBinding, QSparqlResult
*/


/*!
    Constructs an empty result row.

    \sa isEmpty(), append(), insert()
*/

QSparqlResultRow::QSparqlResultRow()
{
    d = new QSparqlResultRowPrivate();
}

/*!
    Constructs a copy of \a other.

    QSparqlResultRow is \l{implicitly shared}. This means you can make copies
    of a result row in \l{constant time}.
*/

QSparqlResultRow::QSparqlResultRow(const QSparqlResultRow& other)
{
    d = other.d;
    d->ref.ref();
}

/*!
    Sets the result row equal to \a other.

    QSparqlResultRow is \l{implicitly shared}. This means you can make copies
    of a result row in \l{constant time}.
*/

QSparqlResultRow& QSparqlResultRow::operator=(const QSparqlResultRow& other)
{
    qAtomicAssign(d, other.d);
    return *this;
}

/*!
    Destroys the object and frees any allocated resources.
*/

QSparqlResultRow::~QSparqlResultRow()
{
    if (!d->ref.deref())
        delete d;
}

/*!
    \fn bool QSparqlResultRow::operator!=(const QSparqlResultRow &other) const

    Returns true if this object is not identical to \a other;
    otherwise returns false.

    \sa operator==()
*/

/*!
    Returns true if this object is identical to \a other (i.e., has
    the same bindings in the same order); otherwise returns false.

    \sa operator!=()
*/
bool QSparqlResultRow::operator==(const QSparqlResultRow &other) const
{
    return d->bindings == other.d->bindings;
}

/*!
    Returns the name of the binding at position \a index. If the binding
    does not exist, an empty string is returned.

    \sa indexOf()
*/

QString QSparqlResultRow::variableName(int index) const
{
    return d->bindings.value(index).name();
}

/*!
    Returns the position of the binding called \a name within the
    result row, or -1 if it cannot be found. Variable names are
    case-sensitive. If more than one binding matches, the index of
    the first one is returned.

    \sa bindingName()
*/

int QSparqlResultRow::indexOf(const QString& name) const
{
    for (int i = 0; i < count(); ++i) {
        if (d->bindings.at(i).name() == name)
            return i;
    }
    return -1;
}

/*!
    Returns the binding at position \a index. If the position is out of
    range, an empty binding is returned.
 */
QSparqlBinding QSparqlResultRow::binding(int index) const
{
    return d->bindings.value(index);
}

/*!
    Returns the value at the current row and position \a index. If the position
    is out of range, an empty QVariant is returned.
 */
QVariant QSparqlResultRow::value(int index) const
{
    return d->bindings.value(index).value();
}

/*! \overload
    Returns the binding called \a name.
 */
QSparqlBinding QSparqlResultRow::binding(const QString &name) const
{
    return binding(indexOf(name));
}

/*! \overload
    Returns the value of the binding called \a name.
 */
QVariant QSparqlResultRow::value(const QString &name) const
{
    return value(indexOf(name));
}


/*!
    Append a copy of binding \a binding to the end of the result row.

    \sa insert() replace() remove()
*/

void QSparqlResultRow::append(const QSparqlBinding& binding)
{
    detach();
    d->bindings.append(binding);
}

/*!
    Removes all the result row's bindings.

    \sa clearValues() isEmpty()
*/

void QSparqlResultRow::clear()
{
    detach();
    d->bindings.clear();
}

/*!
    Returns true if there are no bindings in the result row; otherwise
    returns false.

    \sa append() insert() clear()
*/

bool QSparqlResultRow::isEmpty() const
{
    return d->bindings.isEmpty();
}


/*!
    Returns true if there is a binding in the result row called \a name;
    otherwise returns false.
*/

bool QSparqlResultRow::contains(const QString& name) const
{
    return indexOf(name) >= 0;
}

/*!
    Clears the value of all bindings in the result row and sets each binding
    to null.

    \sa setValue()
*/

void QSparqlResultRow::clearValues()
{
    detach();
    int count = d->bindings.count();
    for (int i = 0; i < count; ++i)
        d->bindings[i].clear();
}

/*!
    Returns the number of bindings in the result row.

    \sa isEmpty()
*/

int QSparqlResultRow::count() const
{
    return d->bindings.count();
}


/*! \internal
*/
void QSparqlResultRow::detach()
{
    qAtomicDetach(d);
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QSparqlResultRow &r)
{
    dbg << "QSparqlResultRow(" << r.count() << ')';
    for (int i = 0; i < r.count(); ++i)
        dbg << '\n' << QString::fromLatin1("%1:").arg(i, 2) << r.binding(i) << r.binding(i).toString();
    return dbg;
}
#endif

QT_END_NAMESPACE
