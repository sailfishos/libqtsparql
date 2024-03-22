/***************************************************************************/
/**
** @copyright Copyright (C) 2010-2011 Nokia Corporation and/or its subsidiary(-ies).
**
** @license Commercial Qt/LGPL 2.1 with Nokia exception/GPL 3.0
**
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

#ifndef QSPARQLXSD_H
#define QSPARQLXSD_H

#include "qsparql.h"

#include <QtCore/qurl.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Sparql)

namespace XSD {
// exporting for plugins
Q_SPARQL_EXPORT QUrl Integer();
Q_SPARQL_EXPORT QUrl Decimal();
Q_SPARQL_EXPORT QUrl Date();
Q_SPARQL_EXPORT QUrl Time();
Q_SPARQL_EXPORT QUrl DateTime();
Q_SPARQL_EXPORT QUrl Int();
Q_SPARQL_EXPORT QUrl NonNegativeInteger();
Q_SPARQL_EXPORT QUrl UnsignedInt();
Q_SPARQL_EXPORT QUrl Short();
Q_SPARQL_EXPORT QUrl Long();
Q_SPARQL_EXPORT QUrl UnsignedLong();
Q_SPARQL_EXPORT QUrl Boolean();
Q_SPARQL_EXPORT QUrl Double();
Q_SPARQL_EXPORT QUrl Float();
Q_SPARQL_EXPORT QUrl String();
Q_SPARQL_EXPORT QUrl Base64Binary();
}

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSPARQLXSD_H
