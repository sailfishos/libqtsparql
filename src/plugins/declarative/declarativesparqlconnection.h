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

#ifndef DECLARATIVESPARQLCONNECTION_H
#define DECLARATIVESPARQLCONNECTION_H

#include <qsparqlquerymodel.h>
#include "declarativesparqlconnectionoptions.h"

#include <QtCore/qglobal.h>
#include <QtCore/qdebug.h>

#include <QtQml/qqml.h>
#include <QQmlParserStatus>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Sparql)

class DeclarativeSparqlQuery;
class DeclarativeSparqlConnectionOptions;

class Q_SPARQL_EXPORT DeclarativeSparqlConnection : public QSparqlConnection, public QQmlParserStatus
{
    Q_OBJECT
    Q_ENUMS(Status)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString driver READ getDriver WRITE setDriver)
    Q_PROPERTY(QVariant result READ getResult NOTIFY resultReady)
    Q_PROPERTY(DeclarativeSparqlConnectionOptions * options READ getOptions WRITE setOptions)
    Q_CLASSINFO("DefaultProperty", "driver")
    Q_INTERFACES(QQmlParserStatus)

public:
    enum Status {
        Null,
        Ready,
        Loading,
        Error
    };

    DeclarativeSparqlConnection();
    ~DeclarativeSparqlConnection() {}

    void classBegin();
    void componentComplete();

    Q_INVOKABLE QVariant select(QString query, bool async = false);
    Q_INVOKABLE QVariant select(QString query, QVariant boundValues, bool async = false);
    Q_INVOKABLE QVariant ask(QString query, bool async = false);
    Q_INVOKABLE QVariant ask(QString query, QVariant boundValues, bool async = false);
    Q_INVOKABLE QVariant update(QString query, bool async = false);
    Q_INVOKABLE QVariant update(QString query, QVariant boundValues, bool async = false);
    Q_INVOKABLE QVariant construct(QString query, bool async = false);
    Q_INVOKABLE QVariant construct(QString query, QVariant boundValues, bool async = false);
    Q_INVOKABLE QString errorString() const;

    void setOptions(DeclarativeSparqlConnectionOptions* options);
    DeclarativeSparqlConnectionOptions* getOptions();

    void setDriver(QString driverName);
    QString getDriver();

    Status status();
    QVariant getResult();

Q_SIGNALS:
    void statusChanged();
    void resultReady();
    void onCompleted();

private Q_SLOTS:
    void onResultFinished();

private:
    QVariant resultToVariant(QSparqlResult *result);
    QVariant runQuery(QSparqlQuery query, bool async);
    void changeStatus(DeclarativeSparqlConnection::Status);
    bool bindValues(QSparqlQuery *query, QVariant boundValues);

    QString driverName;
    QString lastErrorMessage;
    QSparqlResult *asyncResult; // for async queries
    QVariant lastResult;
    DeclarativeSparqlConnectionOptions *options;
    Status connectionStatus;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif
