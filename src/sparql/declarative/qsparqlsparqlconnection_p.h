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

#include <qsparqlquerymodel.h>

#include <QtCore/qglobal.h>
#include <QtCore/qdebug.h>

#ifdef QT_VERSION_5
#include <QtQml/qqml.h>
#include <QQmlParserStatus>
#define QDeclarativeParserStatus QQmlParserStatus
#else
#include <QtDeclarative/qdeclarative.h>
#include <QDeclarativeParserStatus>
#endif

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Sparql)

class SparqlQuery;
class SparqlConnectionOptions;

class Q_SPARQL_EXPORT SparqlConnection : public QSparqlConnection,
                                         public QDeclarativeParserStatus
{
    Q_OBJECT
    Q_ENUMS(Status)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString driver WRITE setDriver READ getDriver)
    Q_PROPERTY(QVariant result READ getResult NOTIFY resultReady)
    Q_PROPERTY(SparqlConnectionOptions * options WRITE setOptions READ getOptions)
    Q_CLASSINFO("DefaultProperty", "driver")
    Q_INTERFACES(QDeclarativeParserStatus)

public:
    SparqlConnection();
    ~SparqlConnection() {}

    enum Status { Null, Ready, Loading, Error };

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

Q_SIGNALS:
    void statusChanged(SparqlConnection::Status);
    void resultReady(QVariant);
    void onCompleted();

private Q_SLOTS:
    void onResultFinished();

private:
    QString driverName;
    QString lastErrorMessage;
    QSparqlResult *asyncResult; // for async queries
    QVariant lastResult;
    SparqlConnectionOptions *options;
    Status connectionStatus;

    QVariant resultToVariant(QSparqlResult *result);
    QVariant runQuery(QSparqlQuery query, bool async);
    QVariant getResult();
    void changeStatus(SparqlConnection::Status);
    bool bindValues(QSparqlQuery *query, QVariant boundValues);
    // property methods
    void setOptions(SparqlConnectionOptions* options);
    SparqlConnectionOptions* getOptions();
    void setDriver(QString driverName);
    QString getDriver();
    Status status();
};

QT_END_NAMESPACE

QT_END_HEADER
