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

#ifndef QSPARQL_ENDPOINT_H
#define QSPARQL_ENDPOINT_H

#include <QtSparql/private/qsparqldriver_p.h>
#include <QtSparql/qsparqlresult.h>

#if defined (Q_OS_WIN32)
#include <QtCore/qt_windows.h>
#endif

#ifdef QT_PLUGIN
#define Q_EXPORT_SPARQLDRIVER_ENDPOINT
#else
#define Q_EXPORT_SPARQLDRIVER_ENDPOINT Q_SPARQL_EXPORT
#endif

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class EndpointDriverPrivate;
class EndpointResultPrivate;
class EndpointDriver;

class EndpointResult : public QSparqlResult
{
    Q_OBJECT
    friend class EndpointResultPrivate;
public:
    explicit EndpointResult(EndpointDriverPrivate* p);
    ~EndpointResult();

    QVariant handle() const;
    // TODO: this should be removed
    bool exec(const QString& query, QSparqlQuery::StatementType type, const QString& prefixes);

    QSparqlBinding binding(int field) const;
    QVariant value(int field) const;
    int size() const;
    QSparqlResultRow current() const;

    void waitForFinished();
    bool isFinished() const;

protected:
    void cleanup();

private:
    EndpointResultPrivate* d;
};

class Q_EXPORT_SPARQLDRIVER_ENDPOINT EndpointDriver : public QSparqlDriver
{
    Q_OBJECT
public:
    explicit EndpointDriver(QObject *parent=0);
//    explicit EndpointDriver(ENDPOINT *con, QObject * parent=0);
    ~EndpointDriver();
    bool hasFeature(QSparqlConnection::Feature f) const;
    bool open(const QSparqlConnectionOptions& options);
    void close();
    EndpointResult* createResult() const;
    EndpointResult* exec(const QString& query, QSparqlQuery::StatementType type);

private:
    EndpointDriverPrivate* d;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSPARQL_ENDPOINT_H
