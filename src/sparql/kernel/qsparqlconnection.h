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

#ifndef QSPARQLCONNECTION_H
#define QSPARQLCONNECTION_H

#include <QtCore/qstring.h>
#include <QtSparql/qsparql.h>
#include <QtSparql/qsparqlconnectionoptions.h>
#include <QtSparql/qsparqlbinding.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Sparql)

class QSparqlError;
class QSparqlQuery;
class QSparqlResult;
class QSparqlConnectionPrivate;

class Q_SPARQL_EXPORT QSparqlConnection : public QObject
{
    Q_OBJECT
public:
    enum Feature {  QuerySize, DefaultGraph,
                    AskQueries, ConstructQueries, UpdateQueries,
                    SyncExec, AsyncExec };
    // TODO: QuerySize should be removed (API break).

    explicit QSparqlConnection(QObject* parent = 0);
    QSparqlConnection(const QString& type,
                      const QSparqlConnectionOptions& options = QSparqlConnectionOptions(),
                      QObject* parent = 0);
    ~QSparqlConnection();

    QSparqlResult* exec(const QSparqlQuery& query);
    QSparqlResult* syncExec(const QSparqlQuery& query);

    bool isValid() const;
    QString driverName() const;
    bool hasFeature(Feature feature) const;

    void addPrefix(const QString& prefix, const QUrl& uri);
    void clearPrefixes();

    QUrl createUrn() const;
    QSparqlBinding createUrn(const QString& name) const;

    static QStringList drivers();

private:
    friend class QSparqlConnectionPrivate;
    QSparqlConnectionPrivate *d; // replace with Q_DECLARE_PRIVATE when Qt publishes it
};
// TODO: make "validness" of a connection a QObject property

#ifndef QT_NO_DEBUG_STREAM
Q_SPARQL_EXPORT QDebug operator<<(QDebug, const QSparqlConnection &);
#endif

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSPARQLCONNECTION_H
