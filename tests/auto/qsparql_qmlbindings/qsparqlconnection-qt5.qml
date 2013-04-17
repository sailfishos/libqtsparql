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

import QtQuick 2.0
import QtSparql 1.0

Rectangle {
    id: rootComp
    property string setDriver: "QTRACKER_DIRECT"
    property string queryString: sparqlQueryString
    property int portNumber: setPortNumber
    property string host: setHost
    property int connectionStatus: 0
    property int resultReadyCalled: 0
    property variant resultFromSlot: 0

    signal resultReadySignal();

    SparqlConnection {
        objectName: "connectionWithOptions"
        id: sparqlConnection
        driver: setDriver
        options: SparqlConnectionOptions { id:connectionOptions; hostName: host; port: portNumber }
        onStatusChanged: connectionStatus = sparqlConnection.status
        onResultReady: resultReadySignal()
    }

    function runSelectQuery()
    {
       return sparqlConnection.select(queryString);
    }

    function runAskQuery(query)
    {
        return sparqlConnection.ask(query);
    }

    function runAskQueryAsync(query)
    {
        sparqlConnection.ask(query, true);
    }

    function runConstructQuery()
    {
        // will result in error, since construct queries are not
        // supported by tracker
        return sparqlConnection.construct(queryString);
    }

    function runSelectQueryAsync()
    {
        sparqlConnection.resultReady.connect(asyncResultReady);
        sparqlConnection.select(queryString, true);
    }

    function asyncResultReady(result)
    {
        resultFromSlot = result;
        resultReadySignal();
    }

    // returns from the result property
    function returnResults()
    {
        return sparqlConnection.result;
    }

    function returnResultsFromSlot()
    {
        return resultFromSlot;
    }

    function returnConnectionOptions() {
        var hash = {};
        for (var prop in connectionOptions) {
            hash[prop] = connectionOptions[prop];
        }
        return hash;
    }

    function insertContact() {
        var insertQuery = "insert { <qmlInsert> a nco:PersonContact; "+
                          "nie:isLogicalPartOf <qsparql-qml-tests>; "+
                          "nco:nameGiven 'QML INSERT' }";
        sparqlConnection.update(insertQuery);
    }

    function insertBoundContact() {
        var insertQuery = "insert { <qmlInsert-bound> a nco:PersonContact; "+
                          "nie:isLogicalPartOf <qsparql-qml-tests>; "+
                          "nco:nameGiven ?:nameGiven; "+
                          "nco:nameFamily ?:nameFamily }";
        var boundValues = { "nameFamily":"QML Family", "nameGiven":"QML Insert" };
        sparqlConnection.update(insertQuery, boundValues);
    }

    function deleteBoundContact() {
        var deleteQuery = "delete { ?u a nco:PersonContact } "+
                          "where { ?u a nco:PersonContact; nco:nameGiven ?:ng; nco:nameFamily ?:nf } ";
        var boundValues = { "ng":"QML Insert", "nf":"QML Family" };
        sparqlConnection.update(deleteQuery, boundValues);
    }

    function selectBoundContact() {
        var selectQuery = "select ?u { ?u a nco:PersonContact; "+
                          "nie:isLogicalPartOf <qsparql-qml-tests>; "+
                          "nco:nameGiven ?:nameGiven; nco:nameFamily ?:nameFamily}";
        var boundValues = { "nameFamily":"QML Family", "nameGiven":"QML Insert" };
        return sparqlConnection.select(selectQuery, boundValues);
    }

    function askBoundContact() {
        var askQuery = "ask { ?u a nco:PersonContact; "+
                       "nco:nameGiven ?:ng;"+
                       "nco:nameFamily ?:nf .}";
        var boundValues = { "ng":"QML Insert", "nf":"QML Family" };
        return sparqlConnection.ask(askQuery, boundValues);
    }

    function insertContactAsync() {
        var insertQuery = "insert { <qmlInsert> a nco:PersonContact; "+
                          "nie:isLogicalPartOf <qsparql-qml-tests>; "+
                          "nco:nameGiven 'QML INSERT' }";
        sparqlConnection.update(insertQuery, true);
    }

    function deleteContact() {
        var deleteQuery = "delete { <qmlInsert> a nco:PersonContact } "+
                          "where { <qmlInsert> a nco:PersonContact } ";

        sparqlConnection.update(deleteQuery);
    }

    function deleteContactAsync() {
        var deleteQuery = "delete { <qmlInsert> a nco:PersonContact } "+
                          "where { <qmlInsert> a nco:PersonContact } ";

        sparqlConnection.update(deleteQuery, true);
    }

    function getStatus()
    {
        // return the property value here
        // that way we can also check to make sure
        // the notify for Status is working
        return connectionStatus
    }

    function getLastError()
    {
        return sparqlConnection.errorString();
    }
}

