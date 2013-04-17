/****************************************************************************
**
** Copyright (C) 2010-2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (ivan.frade@nokia.com)
**
** This file is part of the examples of the QtSparql module (not yet part of the Qt Toolkit).
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

#include <QApplication>
#include <QTableView>
#include <QtSparql>

#include "customsparqlmodel.h"

QSparqlConnection* connection;

void initializeModel(QSparqlQueryModel *model)
{
    model->setQuery(QSparqlQuery("select ?u ?ng ?nf { ?u a nco:Contact; nco:nameGiven ?ng; nco:nameFamily ?nf . }"), *connection);
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("First name"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Last name"));
}

void createView(const QString &title, QSparqlQueryModel *model)
{
    static int offset = 0;

    QTableView *view = new QTableView;
    view->setModel(model);
    view->setWindowTitle(title);
    view->move(100 + offset, 100 + offset);
    offset += 20;
    view->show();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);


    connection = new QSparqlConnection("QTRACKER_DIRECT");
    QSparqlQueryModel model;
    
    model.setQuery(QSparqlQuery("select ?u ?ng ?nf { ?u a nco:Contact; nco:nameGiven ?ng; nco:nameFamily ?nf . }"), *connection);
    model.setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model.setHeaderData(1, Qt::Horizontal, QObject::tr("First name"));
    model.setHeaderData(2, Qt::Horizontal, QObject::tr("Last name"));

    CustomSparqlModel customModel;
    initializeModel(&customModel);

    createView(QObject::tr("Plain Query Model"), &model);
    createView(QObject::tr("Custom Query Model"), &customModel);

    return app.exec();
}
