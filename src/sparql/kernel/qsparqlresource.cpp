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

#include "qsparqlresource.h"

#include "qdebug.h"
#include "qstringlist.h"
#include "qatomic.h"
#include "qstring.h"
#include "qvector.h"
#include "qvariant.h"

QT_BEGIN_NAMESPACE

class QSparqlResourcePrivate
{
public:
    QSparqlResourcePrivate();
    QSparqlResourcePrivate(const QSparqlResourcePrivate &other);
    QAtomicInt ref;
    
    QUrl uri;
    QUrl mainType;
    QList<QUrl> types;
    
    QHash<QUrl, QVariant> properties;
};

QSparqlResourcePrivate::QSparqlResourcePrivate()
{
    ref = 1;
}

QSparqlResourcePrivate::QSparqlResourcePrivate(const QSparqlResourcePrivate &other)
{
    ref = 1;
}


/*!
    \class QSparqlResource
    \brief The QSparqlResource class encapsulates an RDFS resource.

    \ingroup database
    \ingroup shared
    \inmodule QtSparql

    The QSparqlResource class encapsulates the functionality and
    characteristics of an RDFS resource. 
*/

QSparqlResource::QSparqlResource()
{
    d = new QSparqlResourcePrivate();
}

/*!
    Constructs a copy of \a other.

    QSparqlResource is \l{implicitly shared}. This means you can make copies
    of a record in \l{constant time}.
*/
QSparqlResource::QSparqlResource(const QSparqlResource& other)
{
    d = other.d;
    d->ref.ref();
}

QSparqlResource::QSparqlResource(const QString& pathOrIdentifier, const QUrl& type)
{
    d = new QSparqlResourcePrivate();
    d->uri = QUrl(pathOrIdentifier);
    d->mainType = type;
}

QSparqlResource::QSparqlResource(const QUrl& uri, const QUrl& type)
{
    d = new QSparqlResourcePrivate();
    d->uri = uri;
    d->mainType = type;
}

QSparqlResource::~QSparqlResource()
{
    if (!d->ref.deref())
        delete d;
}

QSparqlResource& QSparqlResource::operator=(const QSparqlResource& other)
{
    qAtomicAssign(d, other.d);
    return *this;
}

QSparqlResource& QSparqlResource::operator=(const QUrl& uri)
{
}

QUrl QSparqlResource::resourceUri() const
{
    return d->uri;
}

QUrl QSparqlResource::resourceType() const
{
    return d->mainType;
}

QList<QUrl> QSparqlResource::types() const
{
    return d->types;
}

void QSparqlResource::setTypes(const QList<QUrl>& types)
{
    d->types = types;
}

void QSparqlResource::addType(const QUrl& type)
{
    d->types << type;
}

bool QSparqlResource::hasType(const QUrl& typeUri) const
{
    return d->types.contains(typeUri);
}

QString QSparqlResource::className() const
{
}

QHash<QUrl, QVariant> QSparqlResource::properties() const
{
    return d->properties;
}

bool QSparqlResource::hasProperty(const QUrl& uri) const
{
    return d->properties.contains(uri);
}

QVariant QSparqlResource::property(const QUrl& uri) const
{
    if (d->properties.contains(uri)) {
        return d->properties[uri];
    } else {
        return QVariant();
    }
}

void QSparqlResource::setProperty(const QUrl& uri, const QVariant& value)
{
    d->properties[uri] = value;
}

void QSparqlResource::setProperty(const QString& uri, const QVariant& value)
{
    d->properties[QUrl(uri)] = value;
}

void QSparqlResource::addProperty(const QUrl& uri, const QVariant& value)
{
    // TODO: Append 'value' to a list of property values
    d->properties[uri] = value;
}

void QSparqlResource::removeProperty(const QUrl& uri)
{
    d->properties.remove(uri);
}

void QSparqlResource::removeProperty(const QUrl& uri, const QVariant& value)
{
    // TODO: If there is a list of property values, just remove 'value'
    d->properties.remove(uri);
}

bool QSparqlResource::operator==(const QSparqlResource& other) const
{
    return d->uri == other.d->uri;
}

bool QSparqlResource::operator!=(const QSparqlResource& other) const
{
    return d->uri != other.d->uri;
}

/*!
  Save the instance in the data store

*/

void QSparqlResource::save()
{
}

/*!
  Find all instances of the URI 'type' in the store

*/

QList<QSparqlResource> QSparqlResource::find(const QUrl& type, const QString& query)
{
    return QList<QSparqlResource>();
}

QT_END_NAMESPACE
