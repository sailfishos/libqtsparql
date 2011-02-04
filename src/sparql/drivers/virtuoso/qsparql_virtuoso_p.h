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

#ifndef QSPARQL_VIRTUOSO_H
#define QSPARQL_VIRTUOSO_H

#include <QtSparql/private/qsparqldriver_p.h>
#include <QtSparql/qsparqlresult.h>

#if defined (Q_OS_WIN32)
#include <QtCore/qt_windows.h>
#endif

#ifdef QT_PLUGIN
#define Q_EXPORT_SPARQLDRIVER_VIRTUOSO
#else
#define Q_EXPORT_SPARQLDRIVER_VIRTUOSO Q_SPARQL_EXPORT
#endif

#ifdef Q_OS_UNIX
#define HAVE_LONG_LONG 1 // force UnixODBC NOT to fall back to a struct for BIGINTs
#endif

#if defined(Q_CC_BOR)
// workaround for Borland to make sure that SQLBIGINT is defined
#  define _MSC_VER 900
#endif
#include <sql.h>
#if defined(Q_CC_BOR)
#  undef _MSC_VER
#endif

//#ifndef Q_ODBC_VERSION_2
//#include <sqlucode.h>
//#endif

#include <sqlext.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QVirtuosoResultPrivate;
class QVirtuosoAsyncResultPrivate;
class QVirtuosoDriverPrivate;
class QVirtuosoDriver;

class QVirtuosoResult : public QSparqlResult
{
    Q_OBJECT
public:
    QVirtuosoResult(const QVirtuosoDriver * db, QVirtuosoDriverPrivate* p,
                    const QString& query, QSparqlQuery::StatementType type,
                    const QString& prefixes);
    virtual ~QVirtuosoResult();

    QVariant handle() const;
    virtual bool runQuery();

    bool next();
    QSparqlBinding binding(int field) const;
    QVariant value(int field) const;
    QSparqlResultRow current() const;
    int size() const;

    void waitForFinished();
    bool isFinished() const;

    bool hasFeature(QSparqlResult::Feature feature) const;
    virtual void terminate() {}
private:
    QVirtuosoResultPrivate *d;
};

class QVirtuosoAsyncResult : public QVirtuosoResult
{
    Q_OBJECT
    friend class QVirtuosoFetcherPrivate;
public:
    QVirtuosoAsyncResult(const QVirtuosoDriver * db, QVirtuosoDriverPrivate* p,
                    const QString& query, QSparqlQuery::StatementType type,
                    const QString& prefixes);
    virtual ~QVirtuosoAsyncResult();

    Q_INVOKABLE void startFetcher();
    bool runQuery();

    bool next();
    QSparqlBinding binding(int field) const;
    QVariant value(int field) const;
    QSparqlResultRow current() const;
    int size() const;

    void waitForFinished();
    bool isFinished() const;

    bool hasFeature(QSparqlResult::Feature feature) const;
    void terminate();
private:
    bool fetchNextResult();
    bool fetchBoolResult();
    bool fetchGraphResult();
    QVirtuosoAsyncResultPrivate *d;
};

class Q_EXPORT_SPARQLDRIVER_VIRTUOSO QVirtuosoDriver : public QSparqlDriver
{
    Q_OBJECT
public:
    explicit QVirtuosoDriver(QObject *parent=0);
    QVirtuosoDriver(SQLHANDLE env, SQLHANDLE con, QObject * parent=0);
    virtual ~QVirtuosoDriver();
    bool hasFeature(QSparqlConnection::Feature f) const;
    void close();
    QVariant handle() const;
    bool open(const QSparqlConnectionOptions& options);
    QVirtuosoAsyncResult* exec(const QString& query, QSparqlQuery::StatementType type);
    QVirtuosoResult* syncExec(const QString& query, QSparqlQuery::StatementType type);

protected:
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

private:
    void init();
    bool endTrans();
    void cleanup();
    QVirtuosoDriverPrivate* d;
    friend class QVirtuosoResultPrivate;
    friend class QVirtuosoAsyncResultPrivate;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSPARQL_VIRTUOSO_H
