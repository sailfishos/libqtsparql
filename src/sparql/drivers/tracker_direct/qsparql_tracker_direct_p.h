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

#ifndef QSPARQL_TRACKER_DIRECT_H
#define QSPARQL_TRACKER_DIRECT_H

#include <QtSparql/private/qsparqldriver_p.h>
#include <QtSparql/qsparqlresult.h>
#include <QtSparql/qsparqlquery.h>

#include <QtDBus/QtDBus>

#if defined (Q_OS_WIN32)
#include <QtCore/qt_windows.h>
#endif

#ifdef QT_PLUGIN
#define Q_EXPORT_SPARQLDRIVER_TRACKER_DIRECT
#else
#define Q_EXPORT_SPARQLDRIVER_TRACKER_DIRECT Q_SPARQL_EXPORT
#endif

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QTrackerDirectDriverPrivate;
class QTrackerDirectResultPrivate;
class QTrackerDirectDriver;

class Q_EXPORT_SPARQLDRIVER_TRACKER_DIRECT QTrackerDirectResult : public QSparqlResult
{
    Q_OBJECT
    friend class QTrackerDirectDriver;
    friend class QTrackerDirectResultPrivate; // for emitting signals
public:
    explicit QTrackerDirectResult(QTrackerDirectDriverPrivate* p,
                                  const QString& query,
                                  QSparqlQuery::StatementType type);
    ~QTrackerDirectResult();

    Q_INVOKABLE void startFetcher();
    bool runQuery();

    // Implementation of the QSparqlResult interface
    virtual void waitForFinished();
    virtual bool isFinished() const;

    virtual QSparqlResultRow current() const;
    virtual QSparqlBinding binding(int i) const;
    virtual QVariant value(int i) const;
    virtual int size() const;

protected:
    void cleanup();

private:
    void terminate();
    bool fetchNextResult();
    bool fetchBoolResult();

    QTrackerDirectResultPrivate* d;
    friend class QTrackerDirectFetcherPrivate;
};

class Q_EXPORT_SPARQLDRIVER_TRACKER_DIRECT QTrackerDirectDriver : public QSparqlDriver
{
    Q_OBJECT
public:
    explicit QTrackerDirectDriver(QObject *parent=0);
    ~QTrackerDirectDriver();

    // Implementation of the QSparqlDriver interface
    bool hasFeature(QSparqlConnection::Feature f) const;
    bool open(const QSparqlConnectionOptions& options);
    void close();
    QTrackerDirectResult* exec(const QString& query,
                         QSparqlQuery::StatementType type);
private:
    QTrackerDirectDriverPrivate* d;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSPARQL_TRACKER_DIRECT_H
