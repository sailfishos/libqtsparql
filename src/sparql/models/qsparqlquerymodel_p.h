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

#ifndef QSPARQLQUERYMODEL_P_H
#define QSPARQLQUERYMODEL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qsparql*model.h .  This header file may change from version to version
// without notice, or even be removed.
//
// We mean it.
//

#include <qsparqlerror.h>
#include <qsparqlquery.h>
#include <qsparqlresultrow.h>

#include <QtCore/qhash.h>
#include <QtCore/qvarlengtharray.h>
#include <QtCore/qvector.h>
#include <QtCore/qabstractitemmodel.h>
#include "qsparqlquerymodel.h"

QT_BEGIN_NAMESPACE

class QSparqlQueryModel;
class QSparqlResult;
class QSparqlConnection;

class QSparqlQueryModelPrivate : public QObject
{
    Q_OBJECT
public:
    QSparqlQueryModelPrivate(QSparqlQueryModel* q_)
        : q(q_), result(0), connection(0), atEnd(false), newQuery(false) {}
    ~QSparqlQueryModelPrivate();
    void prefetch(int);
    void initColOffsets(int size);

    QSparqlQueryModel* q;
    mutable QSparqlQuery query;
    mutable QSparqlError error;
    mutable QSparqlResult *result;
    const QSparqlConnection* connection;
    QModelIndex bottom;
    QSparqlResultRow resultRow;
    uint atEnd : 1;
    QVector<QHash<int, QVariant> > headers;
    QVarLengthArray<int, 56> colOffsets; // used to calculate indexInQuery of columns
    bool newQuery;
    void beginQuery(int totalResults);
    void findRoleNames();
    QHash<int, QByteArray> roleNames;

public Q_SLOTS:
    void addData(int totalResults);
    void queryFinished();
};

QT_END_NAMESPACE

#endif // QSPARQLQUERYMODEL_P_H
