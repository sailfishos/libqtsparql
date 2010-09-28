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

#include <QtCore/QDebug>
#include <QtDeclarative/QDeclarativeExtensionPlugin>
#include <QtDeclarative/qdeclarative.h>

#include "qsparqlresultslist.h"

QSparqlResultsList::QSparqlResultsList(QObject *parent) :
    QAbstractListModel(parent),
    m_connection(0), m_result(0), m_options(0)
{
}

int
QSparqlResultsList::rowCount(const QModelIndex &) const
{
    return m_result->size();
}

QVariant
QSparqlResultsList::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    m_result->setPos(index.row());
    QSparqlResultRow row = m_result->current();
    int i = role - (Qt::UserRole + 1);

    if (i >= row.count())
        return row.binding(i - row.count()).toString();
    else
        return row.value(i);
}

void
QSparqlResultsList::reload()
{
    if (m_options == 0 || m_query.isEmpty())
        return;

    if (m_result != 0)
        delete m_result;

    /* Create and run the sparql query */
    m_connection = new QSparqlConnection(m_options->driverName(), m_options->options());
    m_result = m_connection->exec(QSparqlQuery(m_query));
    connect(m_result, SIGNAL(finished()), this, SLOT(queryFinished()));
}

void QSparqlResultsList::queryFinished()
{
    QHash<int, QByteArray> roleNames;
    roleNames = QAbstractItemModel::roleNames();

    if (m_result->first()) {
        QSparqlResultRow resultRow = m_result->current();

        // Create two set of declarative variables from the variable names used
        // in the select statement
        // 'foo' is just a literal like 1234, but '$foo' is "1234"^^xsd:integer
        // 'bar' is a string 'http://www.w3.org/2002/07/owl#sameAs', but '$bar'
        // is a uri <http://www.w3.org/2002/07/owl#sameAs>
        for (int i = 0; i < resultRow.count(); i++) {
            roleNames.insert((Qt::UserRole + 1) + i, resultRow.binding(i).name().toLatin1());
        }

        for (int i = 0; i < resultRow.count(); i++) {
            roleNames.insert((Qt::UserRole + 1) + i + resultRow.count(), QByteArray("$") + resultRow.binding(i).name().toLatin1());
        }

        setRoleNames(roleNames);
    }

    reset();
}

QSparqlConnectionOptionsWrapper* QSparqlResultsList::options() const
{
    return m_options;
}

void QSparqlResultsList::setOptions(QSparqlConnectionOptionsWrapper *options)
{
    m_options = options;
    reload();
}

QString QSparqlResultsList::query() const
{
    return m_query;
}

void
QSparqlResultsList::setQuery(const QString &query)
{
    m_query = query;
    reload();
}

class SparqlPlugin : public QDeclarativeExtensionPlugin
{
public:
    void registerTypes(const char *uri)
    {
        qmlRegisterType<QSparqlResultsList>(uri, 0, 1, "SparqlResultsList");
        qmlRegisterType<QSparqlConnectionOptionsWrapper>(uri, 0, 1, "SparqlConnectionOptions");
    }
};

Q_EXPORT_PLUGIN2(sparqlplugin, SparqlPlugin);