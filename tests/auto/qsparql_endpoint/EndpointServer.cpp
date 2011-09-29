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

#include "EndpointServer.h"
#include <QTextStream>
#include <QTcpSocket>
#include <QStringList>

EndpointServer::EndpointServer(int _port) : port(_port), disabled(false)
{
    if(!listen(QHostAddress::Any, port))
    {
        disabled=true;
        qDebug() << "Can't bind server to port "<< port;
    }
    else
    {
        qDebug() << "Starting fake endpoint server";
        while(!disabled)
            loop.processEvents();
    }
}
    
EndpointServer::~EndpointServer()
{
    stop();
}

void EndpointServer::stop()
{
    disabled=true;
    qDebug() << "Shutting down fake endpoint www server";
    close();
}

void EndpointServer::incomingConnection(int socket)
{
    if (disabled)
        return;

    QTcpSocket* s = new QTcpSocket(this);
    connect(s, SIGNAL(readyRead()), this, SLOT(readClient()));
    connect(s, SIGNAL(disconnected()), this, SLOT(discardClient()));
    s->setSocketDescriptor(socket);
    
    qDebug() << "New Connection";
}

QString EndpointServer::sparqlData(QString url)
{
    if(url.contains("select", Qt::CaseInsensitive))
    {
        return QString("<head>"
        "   <variable name=\"book\"/>"
        "   <variable name=\"who\"/>"
        "</head>"
        "<results distinct=\"false\" ordered=\"false\">"
        "<result>"
        "    <binding name=\"book\"><uri>http://www.example/book/book5</uri></binding>"
        "    <binding name=\"who\"><bnode>r29392923r2922</bnode></binding>"
        "</result>"
        "<result>"
        "    <binding name=\"book\"><uri>http://www.example/book/book6</uri></binding>"
        "    <binding name=\"who\"><bnode>r8484882r49593</bnode></binding>"
        "</result>"
        "</results>");
    }
    return QString();
}

void EndpointServer::readClient()
{
    if (disabled)
        return;

    // This slot is called when the client sent data to the server. The
    // server looks if it was a get request and sends a very simple HTML
    // document back.
    QTcpSocket* socket = (QTcpSocket*)sender();
    if (socket->canReadLine()) {
        QStringList tokens = QString(socket->readLine()).split(QRegExp("[ \r\n][ \r\n]*"));
        if (tokens[0] == "GET") {
            QString url = tokens[1];
            qDebug() << url;
            QTextStream os(socket);
            os.setAutoDetectUnicode(true);
            os << "HTTP/1.0 200 Ok\r\n"
            "Content-Type: text/html; charset=\"utf-8\"\r\n"
            "\r\n"
            "<?xml version=\"1.0\"?>"
            "<sparql xmlns=\"http://www.w3.org/2005/sparql-results#\">"
            + sparqlData(url) +
            "</sparql>\n";
            socket->close();

            if (socket->state() == QTcpSocket::UnconnectedState) {
                delete socket;
                qDebug() << "Connection closed";
            }
        }
    }
}

void EndpointServer::discardClient()
{
    QTcpSocket* socket = (QTcpSocket*)sender();
    socket->deleteLater();
    
    qDebug() << "Connection closed";
}

bool EndpointServer::isRunning() const
{
    return !disabled;
}

void EndpointServer::pause()
{
    disabled=true;
}

bool EndpointServer::resume()
{
    if(isListening())
    {
        disabled=false;
        return true;
    }
    else
    {
        qDebug() << "Can't resume server as there was problem with binding on port " << port;
        return false;
    }
}
