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

#include "qsparqldriverplugin_p.h"

QT_BEGIN_NAMESPACE


/*  FIXME: ! removed from this line on purpose, enable this doc when Driver is public
    \class QSparqlDriverPlugin
    \brief The QSparqlDriverPlugin class provides an abstract base for custom QSparqlDriver plugins.

    \ingroup plugins

    The SPARQL driver plugin is a simple plugin interface that makes it
    easy to create your own SPARQL driver plugins that can be loaded
    dynamically by Qt.

    Writing a SPARQL plugin is achieved by subclassing this base class,
    reimplementing the pure virtual functions keys() and create(), and
    exporting the class with the Q_EXPORT_PLUGIN2() macro. See the SPARQL
    plugins that come with Qt for example implementations (in the
    \c{plugins/src/sparqldrivers} subdirectory of the source
    distribution).

    \sa {How to Create Qt Plugins}
*/

/*!
    \fn QStringList QSparqlDriverPlugin::keys() const

    Returns the list of drivers (keys) this plugin supports.

    These keys are usually the class names of the custom drivers that
    are implemented in the plugin.

    \sa create()
*/

/*!
    \fn QSparqlDriver *QSparqlDriverPlugin::create(const QString& key)

    Creates and returns a QSparqlDriver object for the driver called \a
    key. The driver key is usually the class name of the required
    driver. Keys are case sensitive.

    \sa keys()
*/

/*!
    Constructs a SPARQL driver plugin and sets the parent to \a parent.
    This is invoked automatically by the Q_EXPORT_PLUGIN2() macro.
*/

QSparqlDriverPlugin::QSparqlDriverPlugin(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys the SPARQL driver plugin.

    You never have to call this explicitly. Qt destroys a plugin
    automatically when it is no longer used.
*/
QSparqlDriverPlugin::~QSparqlDriverPlugin()
{
}

QT_END_NAMESPACE
