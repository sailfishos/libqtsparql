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

#include "qsparqlconnectionoptionswrapper_p.h"

QSparqlConnectionOptions QSparqlConnectionOptionsWrapper::options() const
{
    return opts;
}

QString QSparqlConnectionOptionsWrapper::databaseName() const
{
    return opts.databaseName();
}

void QSparqlConnectionOptionsWrapper::setDatabaseName(const QString &databaseName)
{
    opts.setDatabaseName(databaseName);
}

QString QSparqlConnectionOptionsWrapper::userName() const
{
    return opts.hostName();
}

void QSparqlConnectionOptionsWrapper::setUserName(const QString &userName)
{
    opts.setUserName(userName);
}

QString QSparqlConnectionOptionsWrapper::password() const
{
    return opts.password();
}

void QSparqlConnectionOptionsWrapper::setPassword(const QString &password)
{
    opts.setPassword(password);
}

QString QSparqlConnectionOptionsWrapper::hostName() const
{
    return opts.hostName();
}

void QSparqlConnectionOptionsWrapper::setHostName(const QString &hostName)
{
    opts.setHostName(hostName);
}

QString QSparqlConnectionOptionsWrapper::path() const
{
    return opts.path();
}

void QSparqlConnectionOptionsWrapper::setPath(const QString &path)
{
    opts.setPath(path);
}

QString QSparqlConnectionOptionsWrapper::driverName() const
{
    return driver;
}

void QSparqlConnectionOptionsWrapper::setDriverName(const QString &name)
{
    driver = name;
}

int QSparqlConnectionOptionsWrapper::port() const
{
    return opts.port();
}

void QSparqlConnectionOptionsWrapper::setPort(int port)
{
    opts.setPort(port);
}
