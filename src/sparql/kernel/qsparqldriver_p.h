/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtSparql module of the Qt Toolkit.
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
** Nokia at qt-info@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

// TODO: publish this header file when the Driver interface has stabilized
#ifndef QSPARQLDRIVER_H
#define QSPARQLDRIVER_H

#include <QtSparql/qsparqlconnection.h>
#include <QtSparql/qsparqlconnectionoptions.h>
#include <QtSparql/qsparqlquery.h>

#include <QtCore/qobject.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Sparql)

class QSparqlDriverPrivate;
class QSparqlError;
class QSparqlResult;
class QVariant;

class Q_SPARQL_EXPORT QSparqlDriver : public QObject
{
    Q_OBJECT
    friend class QSparqlConnection;
public:
    explicit QSparqlDriver(QObject *parent=0);
    ~QSparqlDriver();
    virtual bool isOpen() const;
    bool isOpenError() const;

    virtual bool beginTransaction();
    virtual bool commitTransaction();
    virtual bool rollbackTransaction();

    QSparqlError lastError() const;

    virtual QVariant handle() const;
    virtual bool hasFeature(QSparqlConnection::Feature f) const = 0;
    virtual void close() = 0;
    virtual QSparqlResult* exec(const QString& query, QSparqlQuery::StatementType) = 0;

    virtual bool open(const QSparqlConnectionOptions& options = QSparqlConnectionOptions()) = 0;

protected:
    virtual void setOpen(bool o);
    virtual void setOpenError(bool e);
    virtual void setLastError(const QSparqlError& e);

private:
    Q_DISABLE_COPY(QSparqlDriver)
    QSparqlDriverPrivate* d;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSPARQLDRIVER_H