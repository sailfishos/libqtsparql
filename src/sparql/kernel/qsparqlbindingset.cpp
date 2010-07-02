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

#include "qsparqlbindingset.h"

#include "qdebug.h"
#include "qstringlist.h"
#include "qatomic.h"
#include "qsparqlbinding.h"
#include "qstring.h"
#include "qvector.h"

QT_BEGIN_NAMESPACE

class QSparqlBindingSetPrivate
{
public:
    QSparqlBindingSetPrivate();
    QSparqlBindingSetPrivate(const QSparqlBindingSetPrivate &other);

    inline bool contains(int index) { return index >= 0 && index < bindings.count(); }

    QVector<QSparqlBinding> bindings;
    QAtomicInt ref;
};

QSparqlBindingSetPrivate::QSparqlBindingSetPrivate()
{
    ref = 1;
}

QSparqlBindingSetPrivate::QSparqlBindingSetPrivate(const QSparqlBindingSetPrivate &other): bindings(other.bindings)
{
    ref = 1;
}

/*!
    \class QSparqlBindingSet
    \brief The QSparqlBindingSet class encapsulates a database binding set.

    \ingroup database
    \ingroup shared
    \inmodule QtSparql

    The QSparqlBindingSet class encapsulates the functionality and
    characteristics of a database binding set (usually a row in a table or
    view within the database). QSparqlBindingSet supports adding and
    removing bindings as well as setting and retrieving binding values.

    The values of a binding set's bindings' can be set by name or position
    with setValue(); if you want to set a binding to null use
    setNull(). To find the position of a binding by name use indexOf(),
    and to find the name of a binding at a particular position use
    bindingName(). Use binding() to retrieve a QSparqlBinding object for a
    given binding. Use contains() to see if the binding set contains a
    particular binding name.

    A binding set can have bindings added with append() or insert(), replaced
    with replace(), and removed with remove(). All the bindings can be
    removed with clear(). The number of bindings is given by count();
    all their values can be cleared (to null) using clearValues().

    \sa QSparqlBinding, QSparqlQuery::bindingSet()
*/


/*!
    Constructs an empty binding set.

    \sa isEmpty(), append(), insert()
*/

QSparqlBindingSet::QSparqlBindingSet()
{
    d = new QSparqlBindingSetPrivate();
}

/*!
    Constructs a copy of \a other.

    QSparqlBindingSet is \l{implicitly shared}. This means you can make copies
    of a binding set in \l{constant time}.
*/

QSparqlBindingSet::QSparqlBindingSet(const QSparqlBindingSet& other)
{
    d = other.d;
    d->ref.ref();
}

/*!
    Sets the binding set equal to \a other.

    QSparqlBindingSet is \l{implicitly shared}. This means you can make copies
    of a binding set in \l{constant time}.
*/

QSparqlBindingSet& QSparqlBindingSet::operator=(const QSparqlBindingSet& other)
{
    qAtomicAssign(d, other.d);
    return *this;
}

/*!
    Destroys the object and frees any allocated resources.
*/

QSparqlBindingSet::~QSparqlBindingSet()
{
    if (!d->ref.deref())
        delete d;
}

/*!
    \fn bool QSparqlBindingSet::operator!=(const QSparqlBindingSet &other) const

    Returns true if this object is not identical to \a other;
    otherwise returns false.

    \sa operator==()
*/

/*!
    Returns true if this object is identical to \a other (i.e., has
    the same bindings in the same order); otherwise returns false.

    \sa operator!=()
*/
bool QSparqlBindingSet::operator==(const QSparqlBindingSet &other) const
{
    return d->bindings == other.d->bindings;
}

/*!
    Returns the value of the binding located at position \a index in
    the binding set. If \a index is out of bounds, an invalid QVariant
    is returned.

    \sa bindingName() isNull()
*/

QVariant QSparqlBindingSet::value(int index) const
{
    return d->bindings.value(index).value();
}

/*!
    \overload

    Returns the value of the binding called \a name in the binding set. If
    binding \a name does not exist an invalid variant is returned.

    \sa indexOf()
*/

QVariant QSparqlBindingSet::value(const QString& name) const
{
    return value(indexOf(name));
}

/*!
    Returns the name of the binding at position \a index. If the binding
    does not exist, an empty string is returned.

    \sa indexOf()
*/

QString QSparqlBindingSet::variableName(int index) const
{
    return d->bindings.value(index).name();
}

/*!
    Returns the position of the binding called \a name within the
    binding set, or -1 if it cannot be found. Field names are not
    case-sensitive. If more than one binding matches, the first one is
    returned.

    \sa bindingName()
*/

int QSparqlBindingSet::indexOf(const QString& name) const
{
    QString nm = name.toUpper();
    for (int i = 0; i < count(); ++i) {
        if (d->bindings.at(i).name().toUpper() == nm) // TODO: case-insensitive comparison
            return i;
    }
    return -1;
}

/*!
    Returns the binding at position \a index. If the position is out of
    range, an empty binding is returned.
 */
QSparqlBinding QSparqlBindingSet::binding(int index) const
{
    return d->bindings.value(index);
}

/*! \overload
    Returns the binding called \a name.
 */
QSparqlBinding QSparqlBindingSet::binding(const QString &name) const
{
    return binding(indexOf(name));
}


/*!
    Append a copy of binding \a binding to the end of the binding set.

    \sa insert() replace() remove()
*/

void QSparqlBindingSet::append(const QSparqlBinding& binding)
{
    detach();
    d->bindings.append(binding);
}

/*!
    Inserts the binding \a binding at position \a pos in the binding set.

    \sa append() replace() remove()
 */
void QSparqlBindingSet::insert(int pos, const QSparqlBinding& binding)
{
   detach();
   d->bindings.insert(pos, binding);
}

/*!
    Replaces the binding at position \a pos with the given \a binding. If
    \a pos is out of range, nothing happens.

    \sa append() insert() remove()
*/

void QSparqlBindingSet::replace(int pos, const QSparqlBinding& binding)
{
    if (!d->contains(pos))
        return;

    detach();
    d->bindings[pos] = binding;
}

/*!
    Removes the binding at position \a pos. If \a pos is out of range,
    nothing happens.

    \sa append() insert() replace()
*/

void QSparqlBindingSet::remove(int pos)
{
    if (!d->contains(pos))
        return;

    detach();
    d->bindings.remove(pos);
}

/*!
    Removes all the binding set's bindings.

    \sa clearValues() isEmpty()
*/

void QSparqlBindingSet::clear()
{
    detach();
    d->bindings.clear();
}

/*!
    Returns true if there are no bindings in the binding set; otherwise
    returns false.

    \sa append() insert() clear()
*/

bool QSparqlBindingSet::isEmpty() const
{
    return d->bindings.isEmpty();
}


/*!
    Returns true if there is a binding in the binding set called \a name;
    otherwise returns false.
*/

bool QSparqlBindingSet::contains(const QString& name) const
{
    return indexOf(name) >= 0;
}

/*!
    Clears the value of all bindings in the binding set and sets each binding
    to null.

    \sa setValue()
*/

void QSparqlBindingSet::clearValues()
{
    detach();
    int count = d->bindings.count();
    for (int i = 0; i < count; ++i)
        d->bindings[i].clear();
}

/*!
    Returns the number of bindings in the binding set.

    \sa isEmpty()
*/

int QSparqlBindingSet::count() const
{
    return d->bindings.count();
}

/*!
    Sets the value of the binding at position \a index to \a val. If the
    binding does not exist, nothing happens.

    \sa setNull()
*/

void QSparqlBindingSet::setValue(int index, const QVariant& val)
{
    if (!d->contains(index))
        return;
    detach();
    d->bindings[index].setValue(val);
}


/*!
    \overload

    Sets the value of the binding called \a name to \a val. If the binding
    does not exist, nothing happens.
*/

void QSparqlBindingSet::setValue(const QString& name, const QVariant& val)
{
    setValue(indexOf(name), val);
}


/*! \internal
*/
void QSparqlBindingSet::detach()
{
    qAtomicDetach(d);
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QSparqlBindingSet &r)
{
    dbg << "QSparqlBindingSet(" << r.count() << ')';
    for (int i = 0; i < r.count(); ++i)
        dbg << '\n' << QString::fromLatin1("%1:").arg(i, 2) << r.binding(i) << r.value(i).toString();
    return dbg;
}
#endif

QT_END_NAMESPACE
