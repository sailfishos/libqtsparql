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

#ifndef QSPARQLNTRIPLES_P_H
#define QSPARQLNTRIPLES_P_H

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

#include <qsparqlbinding.h>
#include <qsparqlresultrow.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qstring.h>
#include <QtCore/qvector.h>

QT_BEGIN_NAMESPACE

QT_MODULE(Sparql)

class Q_SPARQL_EXPORT QSparqlNTriples {
public:
    QSparqlNTriples(QByteArray &b) : buffer(b), i(0), lineNumber(1) {}
    
    void parseError(QString message);
    void skipWhiteSpace();
    void skipComment(); 
    void skipEoln();
    QUrl parseUri();    
    QSparqlBinding parseNamedNode(QString name); 
    QString parseLanguageTag();
    QSparqlBinding parseLiteral(QString name); 
    QSparqlResultRow parseStatement();
    QVector<QSparqlResultRow> parse();

    QByteArray buffer;
    int i;
    int lineNumber;
    QVector<QSparqlResultRow> results;
};

QT_END_NAMESPACE

#endif // QSPARQLNTRIPLES_P_H
