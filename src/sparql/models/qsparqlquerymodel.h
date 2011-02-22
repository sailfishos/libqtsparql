/***************************************************************************/
/**
** @copyright Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
**
** @license Commercial Qt/LGPL 2.1 with Nokia exception/GPL 3.0
**
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

#ifndef QSPARQLQUERYMODEL_H
#define QSPARQLQUERYMODEL_H

#include <QtCore/qabstractitemmodel.h>
#include <QtSparql/qsparqlconnection.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Sparql)

class QSparqlQueryModelPrivate;
class QSparqlError;
class QSparqlResultRow;
class QSparqlQuery;

class Q_SPARQL_EXPORT QSparqlQueryModel: public QAbstractTableModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QSparqlQueryModel)

public:
    explicit QSparqlQueryModel(QObject *parent = 0);
    virtual ~QSparqlQueryModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QSparqlResultRow resultRow(int row) const;
    QSparqlResultRow resultRow() const;

    QVariant data(const QModelIndex &item, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value,
                       int role = Qt::EditRole);

    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex());
    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex());

    void setQuery(const QSparqlQuery &query, QSparqlConnection &conn);
    QSparqlQuery query() const;

    virtual void clear(); // FIXME: do we need this?

    QSparqlError lastError() const;

Q_SIGNALS:
    void finished();

protected:
    virtual void queryChange();

    QModelIndex indexInQuery(const QModelIndex &item) const;
    void setLastError(const QSparqlError &error);
private:
    QSparqlQueryModelPrivate* d;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSPARQLQUERYMODEL_H
