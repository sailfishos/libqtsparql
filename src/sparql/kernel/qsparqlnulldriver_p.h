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

#ifndef QSPARQLNULLDRIVER_P_H
#define QSPARQLNULLDRIVER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  This header file may
// change from version to version without notice, or even be
// removed.
//
// We mean it.
//

#include "QtCore/qvariant.h"
#include "QtSparql/private/qsparqldriver_p.h"
#include "QtSparql/qsparqlerror.h"
#include "QtSparql/qsparqlresult.h"

QT_BEGIN_NAMESPACE

class QSparqlConnectionOptions;

class QSparqlNullResult : public QSparqlResult
{
public:
    inline explicit QSparqlNullResult()
    {
        setLastError(QSparqlError(QLatin1String("Driver not loaded"),
                                  QSparqlError::ConnectionError));
    }
    inline void waitForFinished() { }
    inline bool isFinished() { return true; }
protected:
    inline bool reset (const QString&) { return false; }
    inline bool fetch(int) { return false; }
    inline bool fetchFirst() { return false; }
    inline bool fetchLast() { return false; }
    inline QVariant data(int) const { return QVariant(); }
    inline bool isNull(int) const { return false; }
    inline int size() const { return -1; }
    inline int numRowsAffected() { return 0; }

    inline void setAt(int) {}
    inline void setActive(bool) {}
    inline void setQuery(const QString&) {}
    inline void setSelect(bool) {}
    inline void setForwardOnly(bool) {}

    inline bool prepare(const QString&) { return false; }
    inline bool savePrepare(const QString&) { return false; }
    inline void bindValue(int, const QVariant&, QSparql::ParamType) {}
    inline void bindValue(const QString&, const QVariant&, QSparql::ParamType) {}
};

class QSparqlNullDriver : public QSparqlDriver
{
public:
    inline QSparqlNullDriver()
    {
        setLastError(QSparqlError(QLatin1String("Driver not loaded"),
                                  QSparqlError::ConnectionError));
    }
    inline bool hasFeature(QSparqlConnection::Feature) const { return false; }
    inline bool open(const QSparqlConnectionOptions&)
        { return false; }
    inline void close() {}
    inline QSparqlResult* exec(const QString&, QSparqlQuery::StatementType)
        { return new QSparqlNullResult(); }

protected:
    inline void setOpen(bool) {}
    inline void setOpenError(bool) {}
    inline void setLastError(const QSparqlError&) {}
};

QT_END_NAMESPACE

#endif // QSPARQLNULLDRIVER_P_H
