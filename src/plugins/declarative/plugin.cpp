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

#include <QQmlExtensionPlugin>
#include <QtQml/qqml.h>

#include <private/qsparqlresultslist_p.h>
#include <declarativesparqllistmodel.h>
#include <declarativesparqlconnection.h>
#include <declarativesparqlconnectionoptions.h>

#include <qsparqlquerymodel.h>


QT_BEGIN_NAMESPACE

class SparqlPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.nemomobile.QtSparql")

public:
    void registerTypes(const char *uri)
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("QtSparql"));
        qmlRegisterType<DeclarativeSparqlListModel>(uri, 1, 0, "SparqlListModel");
        qmlRegisterType<DeclarativeSparqlConnection>(uri, 1, 0, "SparqlConnection");
        qmlRegisterType<DeclarativeSparqlConnectionOptions>(uri, 1, 0, "SparqlConnectionOptions");
        qmlRegisterType<QSparqlResultsList>(uri, 0, 1, "SparqlResultsList");
    }
};

QT_END_NAMESPACE

#include "plugin.moc"
