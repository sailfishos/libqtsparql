/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (ivan.frade@nokia.com)
**
** This file is part of the test suite of the QtSparql module (not yet part of the Qt Toolkit).
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

#include <QtTest/QtTest>
#include <QtSparql/QtSparql>

class tst_QSparqlBenchmark : public QObject
{
    Q_OBJECT

public:
    tst_QSparqlBenchmark();
    virtual ~tst_QSparqlBenchmark();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
//    void trackerDirectAllResources();
//    void trackerDBusAllResources();
    void queryBenchmark();
    void queryBenchmark_data();
};

tst_QSparqlBenchmark::tst_QSparqlBenchmark()
{
}

tst_QSparqlBenchmark::~tst_QSparqlBenchmark()
{
}

void tst_QSparqlBenchmark::initTestCase()
{
    // For running the test without installing the plugins. Should work in
    // normal and vpath builds.
    QCoreApplication::addLibraryPath("../../../plugins");
}

void tst_QSparqlBenchmark::cleanupTestCase()
{
}

void tst_QSparqlBenchmark::init()
{
}

void tst_QSparqlBenchmark::cleanup()
{
}

namespace {
class ResultRetriever : public QObject
{
    Q_OBJECT
public:
    ResultRetriever(QSparqlResult* r)
    : result(r)
    {
        connect(r, SIGNAL(finished()),
                SLOT(onFinished()));
    }

public slots:
    void onFinished()
    {
        // Do something silly with the data
        int total = 0;
        while (result->next()) {
            total += result->value(0).toString().size();
        }
        result->deleteLater();
    }

private:
    QSparqlResult* result;

};

}

void tst_QSparqlBenchmark::queryBenchmark()
{
    QFETCH(QString, connectionName);
    QFETCH(QString, queryString);

    QSparqlQuery query(queryString);
    // Note: connection opening cost is left out from the QBENCHMARK.
    QSparqlConnection conn(connectionName);

    QSparqlResult* r = 0;
    QBENCHMARK {
        for (int i = 0; i < 100; ++i) {
            r = conn.exec(query);
            r->waitForFinished();
            QVERIFY(!r->hasError());
            QVERIFY(r->size() > 0);
            delete r;
        }
    }
}

void tst_QSparqlBenchmark::queryBenchmark_data()
{
    QTest::addColumn<QString>("connectionName");
    QTest::addColumn<QString>("queryString");

    // The query is trivial, these tests cases measure (exaggerates) other costs
    // than running the query.
    QString trivialQuery = "select ?u {?u a rdfs:Resource .}";
    QTest::newRow("TrackerDBusAllResources")
        << "QTRACKER"
        << trivialQuery;

    QTest::newRow("TrackerDirectAllResources")
        << "QTRACKER_DIRECT"
        << trivialQuery;

    // A bit more complicated query. Test data for running this can be found in
    // the tracker project.
    QString artistsAndAlbums =
        "SELECT nmm:artistName(?artist) GROUP_CONCAT(nie:title(?album),'|') "
        "WHERE "
        "{ "
        "?song a nmm:MusicPiece . "
        "?song nmm:performer ?artist . "
        "?song nmm:musicAlbum ?album . "
        "} GROUP BY ?artist";
    QTest::newRow("TrackerDBusArtistsAndAlbums")
        << "QTRACKER"
        << artistsAndAlbums;

    QTest::newRow("TrackerDirectArtistsAndAlbums")
        << "QTRACKER_DIRECT"
        << artistsAndAlbums;
}

QTEST_MAIN(tst_QSparqlBenchmark)
#include "tst_qsparql_benchmark.moc"
