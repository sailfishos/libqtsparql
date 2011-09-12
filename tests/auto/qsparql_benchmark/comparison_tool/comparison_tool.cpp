/****************************************************************************
**
** Copyright (C) 2010-2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (ivan.frade@nokia.com)
**
** This file is part of the test suite of the QtSparql module (not yet part of the Qt Toolkit).
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


#include <QDebug>
#include <QDir>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QDomDocument>
#include <QDomNode>
#include <QDomNodeList>

int main(int argc, char *argv[])
{
    QString dirPath = QDir::homePath() + QDir::separator();
    QDir myDir(dirPath);
    QStringList list = myDir.entryList(QStringList() << "benchmark-*.xml");
    QString pageOutput;
    pageOutput = "<html>"
    "<head>"
    "<title>Benchmark result comparison</title>"
    "</head>"
    "<style>body, table {font-size:12px;}</style>"
    "<body>";
    QHash<QString, QList <QStringList> > results;

    //lets iterate through files, read them and parse
    for(int dirIterator=0; dirIterator < list.count(); dirIterator++)
    {
        QString filename(dirPath+list.at(dirIterator));
        QDomDocument doc("xmlResult");
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly))
            qDebug() << "Couldn't open file "<< filename;
        if (!doc.setContent(&file)) {
            file.close();
            qDebug() << "Couldn't open file "<< filename;
        }
        file.close();

        QDomNode asset = doc.elementsByTagName("benchmark").at(0).firstChild();
        QString created = asset.firstChild().toElement().text();
        QString description = asset.lastChild().toElement().text();
        QDomNodeList testList = doc.elementsByTagName("benchmark").at(0).lastChild().toElement().elementsByTagName("test");
        for(int i=0; i< testList.count(); i++)
        {
            QString name = testList.at(i).toElement().attribute("name");
            QDomNode median = testList.at(i).toElement().firstChild();
            QString medianValue = median.toElement().attribute("value");
            QDomNode mean = median.nextSibling();
            QString meanValue = mean.toElement().attribute("value");
            QDomNode total = mean.nextSibling();
            QString totalValue = total.toElement().attribute("value");
            QStringList runResults;
            runResults << medianValue << meanValue << totalValue;
            results[list.at(dirIterator)].append(runResults);
        }
    }

    pageOutput.append("<table><tr>");
    QHashIterator<QString, QList<QStringList > > i(results);
    // first create column headers
    /*while (i.hasNext()) {
        i.next();
        pageOutput.append("<td>" + i.key() + "</td>");
    }*/
    i.toFront();
    // now lets print values
    while (i.hasNext()) {
        i.next();
        pageOutput.append("</tr>\n<tr>");
        QList<QStringList> values = i.value();
        for(int run=0; run < values.count(); run++)
        {
            pageOutput.append("<td>" + values.at(run).join("<br>") + "</td>");
        }
    }
    pageOutput.append("</tr></table>");
    pageOutput.append("</body></html>");
    QFile data("index.html");
    if (data.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&data);
        out << pageOutput;
    }
    return 0;
}