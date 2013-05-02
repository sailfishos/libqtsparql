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

#ifndef QSPARQLRESULTSLIST_H
#define QSPARQLRESULTSLIST_H

#include <QSparqlQuery>
#include <QSparqlResult>
#include <QSparqlBinding>
#include <QSparqlConnectionOptions>
#include <QSparqlConnection>

#include <private/qsparqlsparqlconnectionoptions_p.h>

#include <QtCore/QAbstractListModel>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Sparql)

class QSparqlResultsListPrivate;

class Q_SPARQL_EXPORT QSparqlResultsList : public QAbstractListModel
{
    Q_OBJECT
    Q_ENUMS(Status)
    Q_PROPERTY(SparqlConnectionOptions * options READ options WRITE setOptions NOTIFY optionsChanged)
    Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_CLASSINFO("DefaultProperty", "query")

public:
    QSparqlResultsList(QObject *parent = 0);
    ~QSparqlResultsList();

    QVariant data(const QModelIndex &index, int role) const;
    int rowCount(const QModelIndex &parent) const;

    SparqlConnectionOptions * options() const;
    void setOptions(SparqlConnectionOptions *options);

    QString query() const;
    void setQuery(const QString &query);

    int count() const;

    enum Status { Null, Ready, Loading, Error };
    Status status() const;

    Q_INVOKABLE QString errorString() const;
    
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    virtual QHash<int, QByteArray> roleNames() const;
#endif

public Q_SLOTS:
    void reload();

Q_SIGNALS:
    void finished();
    void statusChanged(QSparqlResultsList::Status);
    void optionsChanged();
    void queryChanged();
    void countChanged();

private Q_SLOTS:
    void queryData(int totalResults);
    void queryFinished();

private:
    QSparqlResultsListPrivate* d;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSPARQLRESULTSLIST_H
