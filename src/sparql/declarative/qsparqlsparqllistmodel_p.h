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

class SparqlConnection;

class Q_SPARQL_EXPORT SparqlListModel : public QSparqlQueryModel,
                                        public QDeclarativeParserStatus
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QSparqlQueryModel)
    Q_ENUMS(Status)
    Q_PROPERTY(QString query READ getQuery WRITE setQueryQML)
    Q_PROPERTY(SparqlConnection* connection READ getConnection WRITE setConnection)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_CLASSINFO("DefaultProperty", "query")
    Q_INTERFACES(QDeclarativeParserStatus)

public:
    SparqlListModel();
    void classBegin();
    void componentComplete();

    Q_INVOKABLE QString errorString() const;
    Q_INVOKABLE QVariant get(int rowNumber);
    Q_INVOKABLE void reload();

    enum Status { Null, Ready, Loading, Error };


Q_SIGNALS:
    void countChanged();
    void statusChanged(SparqlListModel::Status);

private Q_SLOTS:
    void onFinished();
    void onStarted();
    void onConnectionComplete();

private:
    SparqlConnection *connection;
    QString queryString;
    QString lastErrorMessage;
    Status modelStatus;

    void changeStatus(SparqlListModel::Status status);
    // property methods
    Status status();
    void setConnection(SparqlConnection* connection);
    SparqlConnection* getConnection();
    void setQueryQML(QString query);
    QString getQuery() const;
};

QT_END_NAMESPACE

QT_END_HEADER
