/****************************************************************************
**
** Copyright (C) 2010-2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (ivan.frade@nokia.com)
**
** This file is part of the test suite of the QtSparql module (not yet part of the Qt Toolkit).
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

#ifndef UPDATETESTER_H
#define UPDATETESTER_H

#include <QtCore/qobject.h>
#include <QtCore/qset.h>

class QSignalMapper;
class QSparqlConnection;
class QSparqlResult;

class UpdateTester : public QObject
{
    Q_OBJECT
    QSparqlConnection *connection;
    QSparqlConnection *ownConnection;
    QList<QSparqlResult*> resultList;
    QSet<QSparqlResult*> pendingResults;
    QSignalMapper* updateFinishedMapper;
    QSignalMapper* deleteFinishedMapper;
    int numInserts;
    int numDeletes;
    int id;
    bool isFinished;
    int validateUpdateResultAttempts;
    int validateDeleteResultAttempts;

public:
    UpdateTester(int id);
    void setParameters(int numInserts, int numDeletes);
    void setConnection(QSparqlConnection *connection);
    void waitForFinished();
    void startInThread(QThread* thread);

Q_SIGNALS:
    void updatesComplete();
    void validateUpdateComplete();
    void deletionsComplete();
    void validateDeletionComplete();
    void finished();

public Q_SLOTS:
    void start();
    void run();
    void runInThread();
    void cleanup();

private Q_SLOTS:
    void initResources();
    void startUpdates();
    void startValidateUpdate();
    void validateUpdateResult();
    void startDeletions();
    void startValidateDeletion();
    void validateDeleteResult();
    void onUpdateFinished(QObject* mappedResult);
    void onDeleteFinished(QObject* mappedResult);
    void finish();

private:
    static QString selectTemplate();
    void appendPendingResult(QSparqlResult* result, QSignalMapper* signalMapper);
    void checkIsPendingResult(QObject* mappedResult) const;
    bool removePendingResultWasLast(QObject* mappedResult);
    void doValidateUpdateResult(bool* retry);
    void doValidateDeleteResult(bool* retry);
};

#endif // UPDATETESTER_H
