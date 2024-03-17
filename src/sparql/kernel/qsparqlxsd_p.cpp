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

#include "qsparqlxsd_p.h"

namespace XSD {

QUrl Integer()
{
    return QUrl(QStringLiteral("http://www.w3.org/2001/XMLSchema#integer"));
}

QUrl Decimal()
{
    return QUrl(QStringLiteral("http://www.w3.org/2001/XMLSchema#decimal"));
}

QUrl Date()
{
    return QUrl(QStringLiteral("http://www.w3.org/2001/XMLSchema#date"));
}

QUrl Time()
{
    return QUrl(QStringLiteral("http://www.w3.org/2001/XMLSchema#time"));
}

QUrl DateTime()
{
    return QUrl(QStringLiteral("http://www.w3.org/2001/XMLSchema#dateTime"));
}

QUrl Int()
{
    return QUrl(QStringLiteral("http://www.w3.org/2001/XMLSchema#int"));
}

QUrl NonNegativeInteger()
{
    return QUrl(QStringLiteral("http://www.w3.org/2001/XMLSchema#nonNegativeInteger"));
}

QUrl UnsignedInt()
{
    return QUrl(QStringLiteral("http://www.w3.org/2001/XMLSchema#unsignedInt"));
}

QUrl Short()
{
    return QUrl(QStringLiteral("http://www.w3.org/2001/XMLSchema#short"));
}

QUrl Long()
{
    return QUrl(QStringLiteral("http://www.w3.org/2001/XMLSchema#long"));
}

QUrl UnsignedLong()
{
    return QUrl(QStringLiteral("http://www.w3.org/2001/XMLSchema#unsignedLong"));
}

QUrl Boolean()
{
    return QUrl(QStringLiteral("http://www.w3.org/2001/XMLSchema#boolean"));
}

QUrl Double()
{
    return QUrl(QStringLiteral("http://www.w3.org/2001/XMLSchema#double"));
}

QUrl Float()
{
    return QUrl(QStringLiteral("http://www.w3.org/2001/XMLSchema#float"));
}

QUrl String()
{
    return QUrl(QStringLiteral("http://www.w3.org/2001/XMLSchema#string"));
}

QUrl Base64Binary()
{
    return QUrl(QStringLiteral("http://www.w3.org/2001/XMLSchema#base64Binary"));
}

}
