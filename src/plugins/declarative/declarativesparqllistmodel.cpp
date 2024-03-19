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

#include "declarativesparqllistmodel.h"
#include "declarativesparqlconnection.h"
#include <QtSparql>

DeclarativeSparqlListModel::DeclarativeSparqlListModel()
    : connection(0)
    , modelStatus(Null)
{
    connect(this, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SIGNAL(countChanged()));
    connect(this, SIGNAL(finished()), this, SLOT(onFinished()));
    connect(this, SIGNAL(started()), this, SLOT(onStarted()));
}

void DeclarativeSparqlListModel::classBegin()
{
}

void DeclarativeSparqlListModel::componentComplete()
{
    changeStatus(Loading);
    // we will create the connection once the component has finished reading, that way
    // we know if any connection options have been set
}

void DeclarativeSparqlListModel::setQueryProperty(const QString &query)
{
    if (query == queryString) {
        return;
    }

    queryString = query;
    // if the query property changes, after the binding has been
    // initilised, call reload
    if (modelStatus != Null)
        reload();
}

QString DeclarativeSparqlListModel::getQuery() const
{
    return queryString;
}

QVariant DeclarativeSparqlListModel::get(int rowNumber)
{
    QVariantMap map;
    QSparqlResultRow row = resultRow(rowNumber);
    for (int i=0; i<row.count(); i++) {
        map.insert(row.binding(i).name(), row.value(i));
    }
    return map;
}

void DeclarativeSparqlListModel::reload()
{
    changeStatus(Loading);
    setQuery(QSparqlQuery(queryString), *connection);
}

void DeclarativeSparqlListModel::setConnection(DeclarativeSparqlConnection* connection)
{
    if (connection) {
        this->connection = connection;
        connect(connection, SIGNAL(onCompleted()), this, SLOT(onConnectionComplete()));
    }
}

void DeclarativeSparqlListModel::onConnectionComplete()
{
    if (connection && connection->isValid()) {
        setQuery(QSparqlQuery(queryString), *connection);
    } else {
        lastErrorMessage = QLatin1String("Error opening connection");
        changeStatus(Error);
    }
}

DeclarativeSparqlConnection* DeclarativeSparqlListModel::getConnection()
{
    return connection;
}

DeclarativeSparqlListModel::Status DeclarativeSparqlListModel::status()
{
    return modelStatus;
}

QString DeclarativeSparqlListModel::errorString() const
{
    return lastErrorMessage;
}

void DeclarativeSparqlListModel::onStarted()
{
    changeStatus(Loading);
}

void DeclarativeSparqlListModel::onFinished()
{
    if (lastError().type() == QSparqlError::NoError) {
        changeStatus(Ready);
    } else {
        lastErrorMessage = lastError().message();
        changeStatus(Error);
    }
}

void DeclarativeSparqlListModel::changeStatus(DeclarativeSparqlListModel::Status status)
{
    if (modelStatus != status) {
        modelStatus = status;
        Q_EMIT statusChanged();
    }
}
