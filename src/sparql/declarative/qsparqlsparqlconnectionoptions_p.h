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
#include <QtCore/qstring.h>

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

class Q_SPARQL_EXPORT SparqlConnectionOptions : public QObject,
                                                public QDeclarativeParserStatus,
                                                public QSparqlConnectionOptions
{
    Q_OBJECT
    Q_PROPERTY(QString databaseName READ databaseName WRITE setDatabaseName)
    Q_PROPERTY(QString userName READ userName WRITE setUserName)
    Q_PROPERTY(QString password READ password WRITE setPassword)
    Q_PROPERTY(QString hostName READ hostName WRITE setHostName)
    Q_PROPERTY(QString path READ path WRITE setPath)
    Q_PROPERTY(int port READ port WRITE setPort)
    Q_PROPERTY(QString driverName READ driverName WRITE setDriverName)
    Q_INTERFACES(QDeclarativeParserStatus)
public:
    SparqlConnectionOptions() {}
    void classBegin() {}
    void componentComplete() {}

    void setDriverName(const QString& name)
    {
        driver = name;
    }

    QString driverName() const
    {
        return driver;
    }
private:
    QString driver;
};

QT_END_NAMESPACE

QT_END_HEADER
