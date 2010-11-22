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

#include <QtCore/QDebug>

#include "qsparqlresultslist_p.h"

/*!
    \fn void QSparqlQueryModel::finished()

    This signal is emitted when the QSparqlResult, used by the model, has
    finished retrieving its data or when there was an error.
*/

class QSparqlResultsListPrivate
{
public:
    QSparqlResultsListPrivate(QSparqlResultsList* _q) : q(_q), connection(0), result(0), options(0)
    {
    }

    QSparqlResultsList *q;
    QSparqlConnection *connection;
    QSparqlResult *result;
    QString query;
    QSparqlConnectionOptionsWrapper *options;
};

QSparqlResultsList::QSparqlResultsList(QObject *parent) :
    QAbstractListModel(parent)
{
    d = new QSparqlResultsListPrivate(this);
}

QSparqlResultsList::~QSparqlResultsList()
{
    delete d;
}

int QSparqlResultsList::rowCount(const QModelIndex &) const
{
    return d->result->size();
}

QVariant QSparqlResultsList::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    d->result->setPos(index.row());
    QSparqlResultRow row = d->result->current();
    int i = role - (Qt::UserRole + 1);

    if (i >= row.count())
        return row.binding(i - row.count()).toString();
    else
        return row.value(i);
}

void QSparqlResultsList::reload()
{
    if (d->options == 0 || d->query.isEmpty())
        return;

    if (d->result != 0) {
        if (!d->result->isFinished())
            return;
        else
            delete d->result;
    }

    delete d->connection;

    d->connection = new QSparqlConnection(d->options->driverName(), d->options->options());
    d->result = d->connection->exec(QSparqlQuery(d->query));
    connect(d->result, SIGNAL(finished()), this, SIGNAL(finished()));
    connect(d->result, SIGNAL(finished()), this, SLOT(queryFinished()));
}

void QSparqlResultsList::queryFinished()
{
    QHash<int, QByteArray> roleNames = QAbstractItemModel::roleNames();

    if (d->result->first()) {
        QSparqlResultRow resultRow = d->result->current();

        // Create two sets of declarative variables from the variable names used
        // in the select statement
        // 'foo' is just a literal like 1234, but '$foo' is "1234"^^xsd:integer
        // 'bar' is a string 'http://www.w3.org/2002/07/owl#sameAs', but '$bar'
        // is a uri <http://www.w3.org/2002/07/owl#sameAs>
        for (int i = 0; i < resultRow.count(); i++) {
            roleNames.insert((Qt::UserRole + 1) + i, resultRow.binding(i).name().toLatin1());
        }

        for (int i = 0; i < resultRow.count(); i++) {
            roleNames.insert((Qt::UserRole + 1) + i + resultRow.count(), QByteArray("$") + resultRow.binding(i).name().toLatin1());
        }

        setRoleNames(roleNames);
    }

    reset();
}

QSparqlConnectionOptionsWrapper* QSparqlResultsList::options() const
{
    return d->options;
}

void QSparqlResultsList::setOptions(QSparqlConnectionOptionsWrapper *options)
{
    d->options = options;
    reload();
}

QString QSparqlResultsList::query() const
{
    return d->query;
}

void
QSparqlResultsList::setQuery(const QString &query)
{
    d->query = query;
    reload();
}
