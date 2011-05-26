/****************************************************************************
**
** Copyright (C) 2010-2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (ivan.frade@nokia.com)
**
** This file is part of QtSparqlTracker library.
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

#include <QtGui/QApplication>
#include <QtDeclarative>
#include "TrackerLiveQuery"
#include <QtSparql>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QString query("select ?urn ?subject "
                       "{ ?urn a nmo:Email ; "
                       "nmo:messageSubject ?subject . "
                       "} "
                       "order by ?e");

    QSparqlConnection connection("QTRACKER_DIRECT");

    // Create a query model

    QSparqlQueryModel model;
    model.setQuery(QSparqlQuery(query), connection);


    // Initilise QML
    QDeclarativeView viewQml;
    QDeclarativeContext *ctxt = viewQml.rootContext();

    // Now set the context property for the ListView model to the liveQuery model
    // access the values in qml using urn and subject
    ctxt->setContextProperty("contactModel", &model);
    viewQml.setSource(QUrl::fromLocalFile("qml/liveQml/main.qml"));
    viewQml.show();

    return app.exec();
}

/*

How to insert example data into tracker:

tracker-sparql -qu "insert {<email0> a nmo:Email ; nmo:messageSubject \"foo\" .}"
tracker-sparql -qu "delete {<email0> a rdfs:Resource . }"

*/
