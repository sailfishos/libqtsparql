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

#include "qsparqlsparqlconnection_p.h"
#include "qsparqlsparqlconnectionoptions_p.h"

#include <QSparqlResult>
#include <QSparqlResultRow>
#include <QSparqlError>

SparqlConnection::SparqlConnection()
  : asyncResult(0)
  , lastResult(0)  // To avoid "result" property having undefined value in QML
  , options(0)
  , connectionStatus(Null)
{
}

void SparqlConnection::classBegin()
{
    changeStatus(Loading);
}

void SparqlConnection::componentComplete()
{
    // we will create the connection once the component has finished reading, that way
    // we know if any connection options have been set
    if (options) {
        qmlConstructor(driverName, *options);
    } else {
        qmlConstructor(driverName);
    }

    // check connection opening when ok
    if (isValid()) {
        changeStatus(Ready);
    } else {
        lastErrorMessage = QLatin1String("Failed to open connection");
        changeStatus(Error);
    }
    Q_EMIT onCompleted();
}

QString SparqlConnection::errorString() const
{
    if (connectionStatus != Error)
        return QString();
    return lastErrorMessage;
}

QVariant SparqlConnection::select(QString queryString, bool async)
{
    QSparqlQuery query(queryString);
    return runQuery(query, async);
}

QVariant SparqlConnection::select(QString queryString, QVariant boundValues, bool async)
{
    QSparqlQuery query(queryString);
    if (bindValues(&query, boundValues))
        return runQuery(query, async);
    return -1;
}

QVariant SparqlConnection::ask(QString queryString, bool async)
{
    QSparqlQuery query(queryString, QSparqlQuery::AskStatement);
    return runQuery(query, async);
}

QVariant SparqlConnection::ask(QString queryString, QVariant boundValues, bool async)
{
    QSparqlQuery query(queryString, QSparqlQuery::AskStatement);
    if (bindValues(&query, boundValues))
        return runQuery(query, async);
    return -1;
}

QVariant SparqlConnection::update(QString queryString, bool async)
{
    // inserts and deletes are both update queries, and run in the same
    // way, so it doesn't matter if this is an insert or delete statement
    QSparqlQuery query(queryString, QSparqlQuery::InsertStatement);
    return runQuery(query, async);
}

QVariant SparqlConnection::update(QString queryString, QVariant boundValues, bool async)
{
    QSparqlQuery query(queryString, QSparqlQuery::InsertStatement);
    if (bindValues(&query, boundValues))
        return runQuery(query, async);
    return -1;
}

QVariant SparqlConnection::construct(QString queryString, bool async)
{
    QSparqlQuery query(queryString, QSparqlQuery::ConstructStatement);
    return runQuery(query, async);
}

QVariant SparqlConnection::construct(QString queryString, QVariant boundValues, bool async)
{
    QSparqlQuery query(queryString, QSparqlQuery::ConstructStatement);
    if (bindValues(&query, boundValues))
        return runQuery(query, async);
    return -1;
}

bool SparqlConnection::bindValues(QSparqlQuery *query, QVariant boundValues)
{
    if (!boundValues.convert(QVariant::Map)) {
        lastErrorMessage = QLatin1String("Invalid bound values hashmap");
        changeStatus(Error);
        return false;
    }

    QMap<QString, QVariant> boundPairs = boundValues.toMap();

    QMapIterator<QString, QVariant> i(boundPairs);
    while (i.hasNext()) {
        i.next();
        query->bindValue(i.key(), i.value());
    }
    return true;
}

QVariant SparqlConnection::runQuery(QSparqlQuery query, bool async)
{
    if (!isValid()) {
        return -1;
    }
    changeStatus(Ready);
    // clear the last result
    lastResult.clear();
    if (async) {
        // in case of previous active async result
        delete asyncResult;
        asyncResult = exec(query);
        connect(asyncResult, SIGNAL(finished()), this, SLOT(onResultFinished()));
    } else {
        QSparqlResult *result = syncExec(query);
        return resultToVariant(result);
    }
    return 0;
}

QVariant SparqlConnection::getResult()
{
    return lastResult;
}

void SparqlConnection::onResultFinished()
{
    resultToVariant(asyncResult);
    asyncResult = 0;
}

QVariant SparqlConnection::resultToVariant(QSparqlResult *result)
{
    // check for a result error
    if (result->hasError()) {
        lastErrorMessage = result->lastError().message();
        changeStatus(Error);
        delete result;
        return -1;
    }
    // check for ask query first, just return a bool
    // if it is
    if (result->isBool()) {
        // next past the result to avoid warning
        result->next();
        result->next();
        lastResult = result->boolValue();
    } else {
        QVariantList resultList;
        QVector<QString> bindingNames;
        while(result->next()) {
            // only read the binding names once
            if (result->pos() == 0) {
                QSparqlResultRow row = result->current();
                for (int i=0; i<row.count(); i++) {
                    bindingNames.append(row.binding(i).name());
                }
            }
            QVariantMap resultHash;
            for (int i=0; i<bindingNames.size(); i++) {
                resultHash.insert(bindingNames.at(i), result->value(i));
            }
            resultList.append(resultHash);
        }
        lastResult = resultList;
    }
    result->deleteLater();
    Q_EMIT resultReady(lastResult);
    return lastResult;
}

void SparqlConnection::changeStatus(SparqlConnection::Status status)
{
    if (connectionStatus != status) {
        connectionStatus = status;
        Q_EMIT statusChanged(connectionStatus);
    }
}
// property set/get methods

SparqlConnection::Status SparqlConnection::status()
{
    return connectionStatus;
}

void SparqlConnection::setOptions(SparqlConnectionOptions* options)
{
    if (options)
    {
       this->options = options;
    }
}

SparqlConnectionOptions* SparqlConnection::getOptions()
{
    return options;
}

void SparqlConnection::setDriver(QString driverName)
{
    this->driverName = driverName;
}

QString SparqlConnection::getDriver()
{
    return driverName;
}
