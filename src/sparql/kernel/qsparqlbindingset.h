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

#ifndef QSPARQLBINDINGSET_H
#define QSPARQLBINDINGSET_H

#include "qsparql.h"

#include <QtCore/qstring.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Sparql)

class QSparqlBinding;
class QStringList;
class QVariant;
class QSparqlBindingSetPrivate;

class Q_SPARQL_EXPORT QSparqlBindingSet
{
public:
    QSparqlBindingSet();
    QSparqlBindingSet(const QSparqlBindingSet& other);
    QSparqlBindingSet& operator=(const QSparqlBindingSet& other);
    ~QSparqlBindingSet();

    bool operator==(const QSparqlBindingSet &other) const;
    inline bool operator!=(const QSparqlBindingSet &other) const { return !operator==(other); }

    QVariant value(int i) const;
    QVariant value(const QString& name) const;
    void setValue(int i, const QVariant& val);
    void setValue(const QString& name, const QVariant& val);

    int indexOf(const QString &name) const;
    QString variableName(int i) const;

    QSparqlBinding binding(int i) const;
    QSparqlBinding binding(const QString &name) const;

    void append(const QSparqlBinding& binding);
    void replace(int pos, const QSparqlBinding& binding);
    void insert(int pos, const QSparqlBinding& binding);
    void remove(int pos);

    bool isEmpty() const;
    bool contains(const QString& name) const;
    void clear();
    void clearValues();
    int count() const;

private:
    void detach();
    QSparqlBindingSetPrivate* d;
};

#ifndef QT_NO_DEBUG_STREAM
Q_SPARQL_EXPORT QDebug operator<<(QDebug, const QSparqlBindingSet &);
#endif

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSPARQLBINDINGSET_H
