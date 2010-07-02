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

#ifndef QSPARQLRESULT_H
#define QSPARQLRESULT_H

#include <QtCore/qvariant.h>
#include <QtCore/qobject.h>
#include <QtSparql/qsparqlbindingset.h>
#include <QtSparql/qsparqlquery.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Sparql)

class QString;
class QSparqlError;
class QSparqlResultPrivate;

class Q_SPARQL_EXPORT QSparqlResult : public QObject
{
    Q_OBJECT
    friend class QSparqlResultPrivate;
    friend class QSparqlConnection;

public:
    virtual ~QSparqlResult();

    // Iterating the result set
    int pos() const;
    bool next();
    bool previous();
    bool first();
    bool last();
    bool seek(int index);
    virtual int size() = 0;
    bool isValid() const; // valid = positioned on a valid row
    // TODO: decide what should be the pos() of the result when the data has
    // arrived; options: 1) pos() == BeforeFirstRow (like now), 2) pos() == 0
    // (the first row), but what if there's no data? pos() == AfterLastRow ?

    // Retrieving data
    QVariant value(int i) const;
    virtual QSparqlBindingSet bindingSet() const;

    // Asynchronous operations
    virtual void waitForFinished();
    virtual bool isFinished() const;
    bool hasError() const;
    QSparqlError lastError() const;

Q_SIGNALS:
    void dataReady(int totalCount);
    void finished();

protected:
    QSparqlResult();
    QString lastQuery() const; // FIXME: needed?
    virtual void setPos(int at);
    virtual void setLastError(const QSparqlError& e);

    // The subclasses need to implement at least fecth, fetchFirst and
    // fetchLast. The default implementations of fetchNext and fetchPrevious use
    // them, but the subclasses can implement them as well.
    virtual bool fetch(int i) = 0;
    virtual bool fetchNext();
    virtual bool fetchPrevious();
    virtual bool fetchFirst() = 0;
    virtual bool fetchLast() = 0;

    // The subclasses need to implement these for retrieving the data
    virtual QVariant data(int i) const = 0;

private:
    QSparqlResultPrivate* d;

private:
    Q_DISABLE_COPY(QSparqlResult)
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSPARQLRESULT_H
