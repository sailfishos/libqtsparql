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

#ifndef QSPARQL_TRACKER_DIRECT_SYNC_RESULT_P_H
#define QSPARQL_TRACKER_DIRECT_SYNC_RESULT_P_H

#include <tracker-sparql.h>
#include "qsparql_tracker_direct_result_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QTrackerDirectDriverPrivate;
class QSparqlQueryOptions;

// A sync and forward-only Result class. The instance of this class is retreved
// with QTrackerDirectDriver::syncExec().
class QTrackerDirectSyncResult : public QTrackerDirectResult
{
    Q_OBJECT
public:
    explicit QTrackerDirectSyncResult(QTrackerDirectDriverPrivate* p,
                                      const QString& query,
                                      QSparqlQuery::StatementType type,
                                      const QSparqlQueryOptions& options);
    ~QTrackerDirectSyncResult();

    // Implementation of the QSparqlResult interface
    virtual bool next();

    virtual QSparqlResultRow current() const;
    virtual QSparqlBinding binding(int i) const;
    virtual QVariant value(int i) const;
    virtual QString stringValue(int i) const;

    virtual bool isFinished() const;
    virtual bool hasFeature(QSparqlResult::Feature feature) const;
    virtual void waitForFinished();

public Q_SLOTS:
    virtual void exec();

private:
    TrackerSparqlCursor* cursor;
    mutable int n_columns;
    bool isAsync;
    const QSparqlQueryOptions* options;

    Q_INVOKABLE void startFetcher();

    virtual void stopAndWait();
    virtual void run();
    void runQuery();
    void updateQuery();
    void selectQuery();
    void terminate();
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSPARQL_TRACKER_DIRECT_SYNC_RESULT_P_H
