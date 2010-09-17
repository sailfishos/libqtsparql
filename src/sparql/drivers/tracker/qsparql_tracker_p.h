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

#ifndef QSPARQL_TRACKER_P_H
#define QSPARQL_TRACKER_P_H

#include "qsparql_tracker_signals.h"

#include <qsparqlquery.h>

#include <QObject>
#include <QVector>
#include <QStringList>
#include <QDBusConnection>
#include <QDBusArgument>

class QDBusInterface;
class QDBusPendingCallWatcher;
class QString;
class QTrackerResult;
class QTrackerDriver;

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QTrackerDriverPrivate {
public:
    QTrackerDriverPrivate();
    ~QTrackerDriverPrivate();
    QDBusConnection connection;
    QDBusInterface* iface;

    QString service;
    QString basePath;

    QString searchInterface;
    QString searchPath;

    QString resourcesInterface;
    QString resourcesPath;

    QString resourcesClassPath;
    QString resourcesClassInterface;
};

class QTrackerResultPrivate : public QObject {
    Q_OBJECT
public:
    QTrackerResultPrivate(QTrackerResult* res,
                          QSparqlQuery::StatementType tp);

    ~QTrackerResultPrivate();
    QDBusPendingCallWatcher* watcher;
    QVector<QStringList> data;
    QSparqlQuery::StatementType type;
    void setCall(QDBusPendingCall& call);

private slots:
    void onDBusCallFinished();
private:
    QTrackerResult* q; // public part
};

QDBusArgument& operator<<(QDBusArgument& argument,
                          const QTrackerChangeNotifier::Quad &t);
const QDBusArgument& operator>>(const QDBusArgument& argument,
                                QTrackerChangeNotifier::Quad &t);

class QTrackerChangeNotifierPrivate : public QObject
{
    Q_OBJECT
public:
    QTrackerChangeNotifierPrivate(const QString& className,
                                  QDBusConnection c,
                                  QTrackerChangeNotifier* q);
public slots:
    void changed(QString className,
                 QList<QTrackerChangeNotifier::Quad> deleted,
                 QList<QTrackerChangeNotifier::Quad> inserted);
public:
    QTrackerChangeNotifier* q; // public part
    QString className; // which class is watched
    QDBusConnection connection;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSPARQL_TRACKER_H
