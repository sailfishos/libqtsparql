/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (ivan.frade@nokia.com)
**
** This file is part of the examples of the QtSparql module (not yet part of the Qt Toolkit).
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
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
** If you have questions regarding the use of this file, please contact
** Nokia at ivan.frade@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QDebug>
#include <QtSparql>
#include <QtGui/QApplication>
#include <QtGui/QLayout>
#include <QtGui/QWidget>
#include <QtGui/QPushButton>

#include "sparqlquerytext.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.addLibraryPath("../../../../qsparql/plugins");

    QSparqlConnectionOptions options;
    options.setHostName("dbpedia.org");
    QSparqlConnection connection("QSPARQL_ENDPOINT", options);

    QWidget mainWidget;
    SparqlQueryText queryText(connection);
    QPushButton button(QLatin1String("Run Query"));
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(&button);
    layout->addWidget(&queryText);
    mainWidget.setLayout(layout);
    mainWidget.resize(600, 300);
    
    QObject::connect(&button, SIGNAL(clicked()), &queryText, SLOT(runQuery()));
    mainWidget.show();
    
    app.exec();
    
    return 0;
}
