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

#include <private/qsparqldriver_p.h>
#include <qsparqlerror.h>
#include <qsparqlresult.h>
#include <qsparqlbinding.h>

#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

class QSparqlConnectionOptions;

class QSparqlNullResult : public QSparqlResult
{
    Q_OBJECT
public:
    inline explicit QSparqlNullResult()
    {
        setLastError(QSparqlError(QLatin1String("Driver not loaded"),
                                  QSparqlError::ConnectionError));
    }
    inline QSparqlResultRow current() const { return QSparqlResultRow(); }
    inline QSparqlBinding binding(int) const { return QSparqlBinding(); }
    inline QVariant value(int) const { return QVariant(); }
    inline void waitForFinished() { }
    inline bool isFinished() const { return true; }
protected:
    inline bool fetch(int) { return false; }
    inline bool isNull(int) const { return false; }
    inline int size() const { return -1; }

    inline void setAt(int) {}
    inline void setQuery(const QString&) {}
    inline void setSelect(bool) {}
    inline void setForwardOnly(bool) {}
};

class QSparqlNullDriver : public QSparqlDriver
{
    Q_OBJECT
public:
    inline QSparqlNullDriver()
    {
        setLastError(QSparqlError(QLatin1String("Driver not loaded"),
                                  QSparqlError::ConnectionError));
    }
    inline bool hasFeature(QSparqlConnection::Feature) const { return false; }
    inline bool hasError() const { return false; }
    inline bool open(const QSparqlConnectionOptions&)
        { return false; }
    inline void close() {}
    inline QSparqlResult* exec(const QString&, QSparqlQuery::StatementType, const QSparqlQueryOptions&)
        { return new QSparqlNullResult(); }

protected:
    inline void setOpen(bool) {}
    inline void setOpenError(bool) {}
    inline void setLastError(const QSparqlError&) {}
};

QT_END_NAMESPACE

#include "moc_qsparqlnulldriver_p.cpp"

#endif // QSPARQLNULLDRIVER_P_H
