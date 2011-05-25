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

#ifndef QSPARQL_TRACKER_DIRECT_DRIVER_P_H
#define QSPARQL_TRACKER_DIRECT_DRIVER_P_H

#include <tracker-sparql.h>

#include <QtSparql/private/qsparqldriver_p.h>
#include <QtSparql/qsparqlquery.h>
#include <QtSparql/qsparqlqueryoptions.h>
#include <QtSparql/qsparqlerror.h>

#include <QtCore/qlist.h>
#include <QtCore/qpointer.h>
#include <QtCore/qmutex.h>
#include <QThreadPool>

class QSparqlResult;

#ifdef QT_PLUGIN
#define Q_EXPORT_SPARQLDRIVER_TRACKER_DIRECT
#else
#define Q_EXPORT_SPARQLDRIVER_TRACKER_DIRECT Q_SPARQL_EXPORT
#endif

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QTrackerDirectDriverPrivate;
class QTrackerDirectDriver;
class QTrackerDirectResult;

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
    QSparqlResult* exec(const QString& query,
                         QSparqlQuery::StatementType type,
                         const QSparqlQueryOptions& options);

Q_SIGNALS:
    void opened();

private:
    QSparqlResult* asyncExec(const QString& query,
                            QSparqlQuery::StatementType type,
                            const QSparqlQueryOptions& options);
    QSparqlResult* syncExec(const QString& query,
                            QSparqlQuery::StatementType type,
                            const QSparqlQueryOptions& options);

private:
    friend class QTrackerDirectDriverPrivate;
    QTrackerDirectDriverPrivate* d;
};

class QTrackerDirectDriverPrivate {
public:
    QTrackerDirectDriverPrivate(QTrackerDirectDriver *driver);
    ~QTrackerDirectDriverPrivate();

    void setOpen(bool open);
    void emitOpened();
    void addActiveResult(QTrackerDirectResult* result);
    void onConnectionOpen(QObject* object,  const char* method, const char* slot);

    TrackerSparqlConnection *connection;
    int dataReadyInterval;
    // This mutex is for ensuring that only one thread at a time
    // is using the connection to make tracker queries. This mutex
    // probably isn't needed as a TrackerSparqlConnection is
    // already thread safe.
    QMutex connectionMutex;
    QTrackerDirectDriver *driver;
    bool asyncOpenCalled;
    QString error;
    QList<QPointer<QTrackerDirectResult> > activeResults;

    QThreadPool threadPool;
};

QVariant readVariant(TrackerSparqlCursor* cursor, int col);
QSparqlError::ErrorType errorCodeToType(gint code);
gint qSparqlPriorityToGlib(QSparqlQueryOptions::Priority priority);

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSPARQL_TRACKER_DIRECT_DRIVER_P_H
