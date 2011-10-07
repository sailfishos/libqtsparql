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

QString makeTable(QHash<QString, QStringList> &results, QString fileName,
                  QString testName1, QString testName2)
{
    if(results[fileName+testName1].count()<2 || results[fileName+testName2].count()<2)
        return QString("No valid data<br>");
    int test1Total = results[fileName+testName1].at(2).toInt();
    int test2Total = results[fileName+testName2].at(2).toInt();
    int fasterTime, slowerTime;
    if(test1Total<test2Total)
    {
        fasterTime=test1Total;
        slowerTime=test2Total;
    }
    else
    {
        fasterTime=test2Total;
        slowerTime=test1Total;
    }
    bool isFirstFaster=(test1Total < test2Total ? true:false);
    QString table("<table style=\"border: 1px solid blue;\"><tr><td style=\"width:80px;\"></td>"
        "<td class=\"%1\">%3</td><td class=\"%2\">%4</td></tr><tr><td>median<br>mean<br>total</td>"
        "<td class=\"%1\">%5</td><td class=\"%2\">%6</td><td class=\"w\">%7 is faster by <b>%8%</b></td>"
        "</tr></table><br />");
    return table.arg(QString(isFirstFaster?"g":"w")).arg(QString((isFirstFaster?"w":"g"))).
    arg(testName1).arg(testName2).arg(results[fileName+testName1].join("<br>")).
    arg(results[fileName+testName2].join("<br>")).arg((isFirstFaster?testName1:testName2)).
    arg(((slowerTime-fasterTime)*100)/slowerTime);
}

QString makeTableCombined(QHash<QString, QStringList> &results, QString fileName,
                  QStringList testNames1, QStringList testNames2)
{
    for(int i=0; i<testNames1.count(); i++)
        if(results[fileName+testNames1[i]].count()<2 || results[fileName+testNames2[i]].count()<2)
            return QString("No valid data<br>");
    int test1Total = 0;
    for(int i=0; i<testNames1.count(); i++)
        test1Total+= results[fileName+testNames1[i]].at(2).toInt();
    int test2Total = 0;
    for(int i=0; i<testNames2.count(); i++)
        test2Total+= results[fileName+testNames2[i]].at(2).toInt();
    int fasterTime, slowerTime;
    if(test1Total<test2Total)
    {
        fasterTime=test1Total;
        slowerTime=test2Total;
    }
    else
    {
        fasterTime=test2Total;
        slowerTime=test1Total;
    }
    bool isFirstFaster=(test1Total < test2Total ? true:false);
    QString table("<table style=\"border: 1px solid blue;\"><tr><td style=\"width:80px;\"></td>"
    "<td class=\"%1\">%3</td><td class=\"%2\">%4</td></tr><tr><td>total</td>"
    "<td class=\"%1\">%5</td><td class=\"%2\">%6</td><td class=\"w\">%7 is faster by <b>%8%</b>"
    "</td></tr></table><br />");
    return table.arg(QString(isFirstFaster?"g":"w")).arg((isFirstFaster?"w":"g")).arg(testNames1.join(" + ")).
    arg(testNames2.join(" + ")).arg(test1Total).arg(test2Total).arg((isFirstFaster?testNames1:testNames2)
    .join(" + ")).arg(((slowerTime-fasterTime)*100)/slowerTime);
}
int main(int argc, char *argv[])
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);
    QString dirPath = QDir::homePath() + QDir::separator();
    QDir myDir(dirPath);
    QStringList fileList = myDir.entryList(QStringList() << "benchmark-*.xml");
    QString pageOutput;
    pageOutput = "<html>"
    "<head>"
    "<title>Benchmark result comparison</title>"
    "</head>"
    "<style>body, table {font-size:12px;}"
    "td.row1{ background-color:#FFFFFF;}"
    "td.row2{ background-color:#E8E8E8;}"
    "sup.t1{color:red;}"
    "sup.t2{color:green;}"
    ".b{color:#006699;}"
    ".o{color:#CC6633;}"
    "td.g{background-color:#99FF66; width:250px;}"
    "td.w{background-color:#FFFFFF; width:250px;}"
    "</style>"
    "<body><h1>Report generated by Benchmark Comparison Tool (part of QSparql test library)</h1>";
    QHash<QString, QStringList> results;
    QHash<QString, QString> tracker_ver;
    QHash<QString, QString> qsparql_ver;
    QStringList testNames;
    QString pageOutput2(pageOutput);

    //lets iterate through files, read them and parse
    for(int dirIterator=0; dirIterator < fileList.count(); dirIterator++)
    {
        QString filename(dirPath+fileList.at(dirIterator));
        QDomDocument doc("xmlResult");
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly))
        {
            qDebug() << "Couldn't open file "<< filename;
            pageOutput.append("Error: Couldn't open file "+filename);
        }
        else
        if (!doc.setContent(&file)) {
            file.close();
            qDebug() << "Couldn't set file content for  QDomDocument "<< filename;
            pageOutput.append("Couldn't set file content for  QDomDocument  "+filename);
        }
        else
        {
            file.close();

            QDomNode asset = doc.elementsByTagName("benchmark").at(0).firstChild();
            QDomNode assetIterator = asset.firstChild();
            QString tracker, qsparql, created;
            for(QDomNode assetIterator = asset.firstChild(); !assetIterator.isNull();
                                                assetIterator = assetIterator.nextSibling())
            {
                QString tagName = assetIterator.toElement().tagName();
                if(tagName == "tracker")
                    tracker = assetIterator.toElement().text();
                else if(tagName == "qsparql")
                    qsparql = assetIterator.toElement().text();
                else if(tagName == "created")
                    created = assetIterator.toElement().text();
            }

            //QString description = assetIterator.toElement().text();
            tracker_ver[fileList.at(dirIterator)]=tracker;
            qsparql_ver[fileList.at(dirIterator)]=qsparql;
            QDomNodeList testList = doc.elementsByTagName("benchmark").at(0).lastChild().
                                    toElement().elementsByTagName("test");
            for(int i=0; i< testList.count(); i++)
            {
                QString name = testList.at(i).toElement().attribute("name");
                if(!testNames.contains(name))
                    testNames << name;
                QDomNode median = testList.at(i).toElement().firstChild();
                QString medianValue = median.toElement().attribute("value");
                QDomNode mean = median.nextSibling();
                QString meanValue = mean.toElement().attribute("value");
                QDomNode total = mean.nextSibling();
                QString totalValue = total.toElement().attribute("value");
                QStringList runResults;
                runResults << medianValue << meanValue << totalValue;
                results[fileList.at(dirIterator)+name] = runResults;
            }
        }
    }

    //overall comparison html page
    pageOutput.append("<br /><table><tr><td>Test name</td>");
    for(int dirIterator=0; dirIterator < fileList.count(); dirIterator++)
    {
        QStringList nameparts = fileList.at(dirIterator).split(".");
        pageOutput.append("<td>" + nameparts[0] + "<br>" +nameparts[1] + "<br><span class=\"b\">"+
                          qsparql_ver[fileList.at(dirIterator)]+"</span><br>"+
                          "<span class=\"o\">"+tracker_ver[fileList.at(dirIterator)]+"</span>" +
                          "<br /><a href=\"tests-comparison.html#" + nameparts[0]+ "." + nameparts[1] +
                          "\">tests comaprison</a></td>");
    }
    pageOutput.append("</tr>\n");
    QStringList previousResult;
    for(int testIterator=0; testIterator < testNames.count(); testIterator++)
    {
        previousResult.clear();
        pageOutput.append(QString("<tr><td class=\"row%1").arg(testIterator%2+1)+
                          "\" style=\"width:230px;\"><b>"+testNames[testIterator].remove(".xml")+
                          "</b><br /><small>median<br />mean<br />total</small></td>");
        for(int dirIterator=0; dirIterator < fileList.count(); dirIterator++)
        {
            pageOutput.append("<td class=\"row"+QString("%1").arg(testIterator%2+1)+"\">");
            for(int partResultIterator=0; partResultIterator < results[fileList.at(dirIterator)+
                                            testNames[testIterator]].count(); partResultIterator++)
            {
                int currentValue=results[fileList.at(dirIterator)+testNames[testIterator]].
                                                            at(partResultIterator).toInt();
                pageOutput.append(QString("%1").arg(currentValue));
                if(previousResult.count() == results[fileList.at(dirIterator)+
                   testNames[testIterator]].count() && previousResult.count()-1 == partResultIterator)
                {
                    int previousValue=previousResult[partResultIterator].toInt();
                    int diff = (previousValue?(((previousValue-currentValue)*100)/previousValue):100);
                    pageOutput.append(QString("  <sup class=\"t%2\">%1%</sup>").arg(diff*-1).
                                                                        arg(diff<0?"1":"2"));
                }
                pageOutput.append("<br />");
            }
            pageOutput.append("</td>\n");
            previousResult = results[fileList.at(dirIterator)+testNames[testIterator]];
        }
        pageOutput.append("</tr>\n\n");
    }
    pageOutput.append("</tr></table>");
    pageOutput.append("</body></html>");

    //between-tests-comparison html page
    pageOutput2.append("<h2>In-between tests comaprison</h2><a href=\"index.html\">Back to the mainpage</a><br><br>");
    for(int dirIterator=0; dirIterator < fileList.count(); dirIterator++)
    {
        QStringList nameparts = fileList.at(dirIterator).split(".");
        pageOutput2.append("<br /><a name=\"" + nameparts[0]+ "." + nameparts[1] + "\"></a>" +
        "<hr><h4>"+ nameparts[0]+ "." + nameparts[1] +"</h4>" + "<span class=\"b\">"+
        qsparql_ver[fileList.at(dirIterator)]+"</span><br>"+
        "<span class=\"o\">"+tracker_ver[fileList.at(dirIterator)]+"</span><br /><br />" +
        makeTable(results, fileList.at(dirIterator), "read-music-Async-fin", "read-music-Sync-fin")+
        makeTable(results, fileList.at(dirIterator), "read-music-Async-read", "read-music-Sync-read")+
        makeTable(results, fileList.at(dirIterator), "read-music-Async-ForwardOnly-fin", "read-music-Async-fin")+
        makeTable(results, fileList.at(dirIterator), "read-music-Async-ForwardOnly-read", "read-music-Async-read")+
        makeTableCombined(results, fileList.at(dirIterator), QStringList() << "read-music-Async-fin"
                            << "read-music-Async-read", QStringList() << "read-music-Sync-fin" << "read-music-Sync-read")+
        makeTableCombined(results, fileList.at(dirIterator), QStringList() << "read-music-Async-fin"
                            << "read-music-Async-read", QStringList() << "read-music-Async-ForwardOnly-fin"
                            << "read-music-Async-ForwardOnly-read"));
    }
    pageOutput2.append("</body></html>");

    QFile data(dirPath+"index.html");
    if (data.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&data);
        out << pageOutput;
        qDebug() << "Report saved in " << dirPath;
        data.close();
    }
    else
        qDebug() << "Couldn't save report in " << dirPath << "Check writing permissions!";
    data.setFileName(dirPath+"tests-comparison.html");
    if (data.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&data);
        out << pageOutput2;
        qDebug() << "Report saved in " << dirPath;
    }
    else
        qDebug() << "Couldn't save report in " << dirPath << "Check writing permissions!";
    return 0;
}