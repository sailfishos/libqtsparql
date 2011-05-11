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

#ifndef QSPARQLXSD_H
#define QSPARQLXSD_H

#include "qsparql.h"

#include <QtCore/qurl.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Sparql)

namespace XSD {
#if defined XSD_INTEGER || defined XSD_ALL
Q_GLOBAL_STATIC_WITH_ARGS(QUrl, Integer,
                          (QLatin1String("http://www.w3.org/2001/XMLSchema#integer")))
#endif

#if defined XSD_DATE || defined XSD_ALL
Q_GLOBAL_STATIC_WITH_ARGS(QUrl, Decimal,
                          (QLatin1String("http://www.w3.org/2001/XMLSchema#decimal")))

Q_GLOBAL_STATIC_WITH_ARGS(QUrl, Date,
                          (QLatin1String("http://www.w3.org/2001/XMLSchema#date")))

Q_GLOBAL_STATIC_WITH_ARGS(QUrl, Time,
                          (QLatin1String("http://www.w3.org/2001/XMLSchema#time")))

Q_GLOBAL_STATIC_WITH_ARGS(QUrl, DateTime,
                          (QLatin1String("http://www.w3.org/2001/XMLSchema#dateTime")))
#endif

#ifdef XSD_ALL
Q_GLOBAL_STATIC_WITH_ARGS(QUrl, Int,
                          (QLatin1String("http://www.w3.org/2001/XMLSchema#int")))


Q_GLOBAL_STATIC_WITH_ARGS(QUrl, NonNegativeInteger,
                          (QLatin1String("http://www.w3.org/2001/XMLSchema#nonNegativeInteger")))

Q_GLOBAL_STATIC_WITH_ARGS(QUrl, UnsignedInt,
                          (QLatin1String("http://www.w3.org/2001/XMLSchema#unsignedInt")))

Q_GLOBAL_STATIC_WITH_ARGS(QUrl, Short,
                          (QLatin1String("http://www.w3.org/2001/XMLSchema#short")))

Q_GLOBAL_STATIC_WITH_ARGS(QUrl, Long,
                          (QLatin1String("http://www.w3.org/2001/XMLSchema#long")))

Q_GLOBAL_STATIC_WITH_ARGS(QUrl, UnsignedLong,
                          (QLatin1String("http://www.w3.org/2001/XMLSchema#unsignedLong")))

Q_GLOBAL_STATIC_WITH_ARGS(QUrl, Boolean,
                          (QLatin1String("http://www.w3.org/2001/XMLSchema#boolean")))

Q_GLOBAL_STATIC_WITH_ARGS(QUrl, Double,
                          (QLatin1String("http://www.w3.org/2001/XMLSchema#double")))

Q_GLOBAL_STATIC_WITH_ARGS(QUrl, Float,
                          (QLatin1String("http://www.w3.org/2001/XMLSchema#float")))

Q_GLOBAL_STATIC_WITH_ARGS(QUrl, String,
                          (QLatin1String("http://www.w3.org/2001/XMLSchema#string")))

Q_GLOBAL_STATIC_WITH_ARGS(QUrl, Base64Binary,
                          (QLatin1String("http://www.w3.org/2001/XMLSchema#base64Binary")))
#endif
}

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSPARQLXSD_H
