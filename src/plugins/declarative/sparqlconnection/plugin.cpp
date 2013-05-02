/****************************************************************************
**
** Copyright (C) 2010-2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (ivan.frade@nokia.com)
**
** This file is part of the plugins of the Qt Toolkit.
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

#include <QtCore/qglobal.h>
#include <QtCore/qdebug.h>

#ifdef QT_VERSION_5
#include <QQmlExtensionPlugin>
#include <QtQml/qqml.h>
#else
#include <QtDeclarative/QDeclarativeExtensionPlugin>
#include <QtDeclarative/qdeclarative.h>
#endif

#include <qsparqlquerymodel.h>

#include <private/qsparqlsparqlconnection_p.h>
#include <private/qsparqlsparqlconnectionoptions_p.h>

QT_BEGIN_NAMESPACE

class SparqlConnectionPlugin
#ifdef QT_VERSION_5
    : public QQmlExtensionPlugin
#else
    : public QDeclarativeExtensionPlugin
#endif
{
    Q_OBJECT
#ifdef QT_VERSION_5
    Q_PLUGIN_METADATA(IID "org.nemomobile.QtSparql.SparqlConnectionInterface")
#endif

public:
    void registerTypes(const char *uri)
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("QtSparql"));
        qmlRegisterType<SparqlConnection>(uri, 1, 0, "SparqlConnection");
        qmlRegisterType<SparqlConnectionOptions>(uri, 1, 0, "SparqlConnectionOptions");
    }
};

#ifndef QT_VERSION_5
Q_EXPORT_PLUGIN2(sparqlconnection, SparqlConnectionPlugin);
#endif

QT_END_NAMESPACE

#include "plugin.moc"
