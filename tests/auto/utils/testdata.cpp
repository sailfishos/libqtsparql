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

#include "testdata.h"
#include <QtGlobal>
#include <QtSparql>

namespace {

class TestDataImpl : public TestData {

public:
    TestDataImpl(const QSparqlQuery& cleanupQuery, const QSparqlQuery& selectQuery);
    ~TestDataImpl();
    void setOK();
    bool isOK() const;
    QSparqlQuery selectQuery() const;
private:
    QSparqlQuery cleanupQuery;
    QSparqlQuery selectQuery_;
    bool ok;
};

}  // namespace

TestData* TestData::createTrackerTestData(int testDataAmount, const QString& testSuiteTag, const QString& testCaseTag)
{
    const int insertBatchSize = 200;
    QSparqlConnection conn("QTRACKER_DIRECT");
    const QString insertQueryTemplate =
            "<urn:music:%1> a nmm:MusicPiece, nfo:FileDataObject, nfo:Audio;"
                "nie:isLogicalPartOf %3 ;"
                "nie:isLogicalPartOf %4 ;"
                "tracker:available          true ;"
                "nie:byteSize               \"0\" ;"
                "nie:url                    \"file://music/Song_%1.mp3\" ;"
                "nfo:belongsToContainer     <file://music/> ;"
                "nie:title                  \"Song %1\" ;"
                "nie:mimeType               \"audio/mpeg\" ;"
                "nie:contentCreated         \"2000-01-01T01:01:01Z\" ;"
                "nie:isLogicalPartOf        <urn:album:%2> ;"
                "nco:contributor            <urn:artist:%2> ;"
                "nfo:fileLastAccessed       \"2000-01-01T01:01:01Z\" ;"
                "nfo:fileSize               \"0\" ;"
                "nfo:fileName               \"Song_%1.mp3\" ;"
                "nfo:fileLastModified       \"2000-01-01T01:01:01Z\" ;"
                "nfo:codec                  \"MPEG\" ;"
                "nfo:averageBitrate         \"16\" ;"
                "nfo:genre                  \"Genre %2\" ;"
                "nfo:channels               \"2\" ;"
                "nfo:sampleRate             \"44100.0\" ;"
                "nmm:musicAlbum             <urn:album:%2> ;"
                "nmm:musicAlbumDisc         <urn:album-disk:%2> ;"
                "nmm:performer              <urn:artist:%2> ;"
                "nfo:duration               \"1\" ;"
                "nmm:trackNumber            \"%1\" .";

    const QSparqlQuery cleanupQuery(
        QString("delete { "
            "?u a rdfs:Resource . } "
            "where { "
            "?u nie:isLogicalPartOf %1 . "
            "?u nie:isLogicalPartOf %2 . "
            "} "
            "delete {"
            "%2 a rdfs:Resource . "
            "}").arg(testSuiteTag).arg(testCaseTag),
        QSparqlQuery::DeleteStatement);

    const QSparqlQuery selectQuery(
        QString("select tracker:id(?musicPiece) ?title ?performer ?album ?duration ?created "
            "{ "
            "?musicPiece a nmm:MusicPiece; "
            "nie:isLogicalPartOf %1; "
            "nie:isLogicalPartOf %2; "
            "nie:title ?title; "
            "nmm:performer ?performer; "
            "nmm:musicAlbum ?album; "
            "nfo:duration ?duration; "
            "nie:contentCreated ?created. "
            "} order by ?title ?created")
                .arg(testSuiteTag)
                .arg(testCaseTag));

    TestDataImpl* testData = new TestDataImpl(cleanupQuery, selectQuery);

    QString insertTagsQuery = QString("insert { %1 a nie:InformationElement. "
                                               "%2 a nie:InformationElement. "
                                               "%2 nie:isLogicalPartOf %1. }")
                                        .arg(testSuiteTag).arg(testCaseTag);
    QScopedPointer<QSparqlResult> r(conn.syncExec(QSparqlQuery(insertTagsQuery, QSparqlQuery::InsertStatement)));
    if (r.isNull() || r->hasError())
        return testData;
    r.reset();

    for (int item = 1; item <= testDataAmount; ) {
        QString insertQuery = "insert { ";
        int itemDozen = 10;
        const int batchEnd = item + insertBatchSize;
        for (; item < batchEnd && item <= testDataAmount; ++item) {
            insertQuery.append( insertQueryTemplate
                                    .arg(item)
                                    .arg(itemDozen)
                                    .arg(testSuiteTag)
                                    .arg(testCaseTag) );
            if (item % 10 == 0) itemDozen += 10;
        }
        insertQuery.append(" }");
        QScopedPointer<QSparqlResult> r(conn.syncExec(QSparqlQuery(insertQuery, QSparqlQuery::InsertStatement)));
        if (r.isNull() || r->hasError())
            return testData;
    }

    testData->setOK();
    return testData;
}

TestDataImpl::TestDataImpl(const QSparqlQuery& cleanupQuery, const QSparqlQuery& selectQuery)
    : cleanupQuery(cleanupQuery), selectQuery_(selectQuery), ok(false)
{
}

TestDataImpl::~TestDataImpl()
{
    QSparqlConnection conn("QTRACKER_DIRECT");
    conn.syncExec(cleanupQuery);
}

void TestDataImpl::setOK()
{
    ok = true;
}

bool TestDataImpl::isOK() const
{
    return ok;
}

QSparqlQuery TestDataImpl::selectQuery() const
{
    return selectQuery_;
}

