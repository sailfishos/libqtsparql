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

#ifndef QSPARQLRESOURCE_H
#define QSPARQLRESOURCE_H

#include <QtCore/qstring.h>
#include <QtCore/qurl.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Sparql)

class QSparqlResourcePrivate;

class Q_SPARQL_EXPORT QSparqlResource
{
public:
    QSparqlResource();
    QSparqlResource(const QSparqlResource&);
    QSparqlResource(const QString& pathOrIdentifier, const QUrl& type = QUrl());
    QSparqlResource(const QUrl& uri, const QUrl& type = QUrl());
    virtual ~QSparqlResource();

    QSparqlResource& operator=(const QSparqlResource& other);
    QSparqlResource& operator=(const QUrl& uri);

    QUrl resourceUri() const;
    QUrl resourceType() const;

    QList<QUrl> types() const;
    void setTypes(const QList<QUrl>& types);
    void addType(const QUrl& type);
    bool hasType(const QUrl& typeUri) const;

    QString className() const;

    QHash<QUrl, QVariant> properties() const;
    bool hasProperty(const QUrl& uri) const;
    QVariant property(const QUrl& uri) const;
    void setProperty(const QUrl& uri, const QVariant& value);
    void setProperty(const QString& uri, const QVariant& value);
    void addProperty(const QUrl& uri, const QVariant& value);
    void removeProperty(const QUrl& uri);
    void removeProperty(const QUrl& uri, const QVariant& value);

    bool operator==(const QSparqlResource&) const;
    bool operator!=(const QSparqlResource&) const;

    void save();
    
    static QList<QSparqlResource> find(const QUrl& type, const QString& query);
private:
    void detach();
    QSparqlResourcePrivate* d;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSPARQLRESOURCE_H
