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

#include "../testhelpers.h"

#include <tracker-sparql.h>

#include <QtTest/QtTest>
#include <QtSparql/QtSparql>
#include <sys/time.h>
#include <stdio.h>
#include <QDomDocument>

#define START_BENCHMARK \
    int benchmarkTotal=0; { \
    struct timeval benchmark_tv; \
    gettimeofday(&benchmark_tv, NULL); \
    const long benchmark_start = benchmark_tv.tv_sec * 1000 + (benchmark_tv.tv_usec + 500) / 1000; \
    do

#define END_BENCHMARK(BENCHMARKNAME) while(false); \
        gettimeofday(&benchmark_tv, NULL); \
        const long benchmark_end = benchmark_tv.tv_sec * 1000 + (benchmark_tv.tv_usec + 500) / 1000; \
        char benchmark_tsbuf[80]; \
        qsnprintf(benchmark_tsbuf, sizeof(benchmark_tsbuf), "QSparql_benchmark_data: %s %lu\n",  \
                  qPrintable(BENCHMARKNAME), benchmark_end - benchmark_start); \
        fwrite(benchmark_tsbuf, qstrnlen(benchmark_tsbuf, sizeof(benchmark_tsbuf)), 1, stderr); \
        benchmarkTotal += benchmark_end - benchmark_start; \
    }

#define PRINT_STATS(BENCHMARKNAME, LIST) { \
    qSort(LIST); \
    int median = (LIST.size()+1)/2; \
    int average = 0; \
    for(int i=0;i<LIST.size();i++) \
        average += LIST[i]; \
    QString underline; \
    for(int i=0;i<BENCHMARKNAME.size();i++) \
    { \
        underline.append("-"); \
    } \
    fprintf(stderr, "\n%s\n%s\n\n", qPrintable(BENCHMARKNAME), qPrintable(underline)); \
    fprintf(stderr, "median\t\t\t\t\t\t\t%i\n", LIST[median]); \
    fprintf(stderr, "mean\t\t\t\t\t\t\t%i\n", average/LIST.size());\
    fprintf(stderr, "total time\t\t\t\t\t\t%i\n\n", average); \
    appendResult(BENCHMARKNAME, LIST[median], average/LIST.size(), average); \
    }
#define NO_QUERIES 100

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
    // Actual benchmarks
    void queryBenchmark();
    void queryBenchmark_data();

    void dataReadingBenchmark();
    void dataReadingBenchmark_data();

    // Reference benchmarks
    void queryWithLibtrackerSparql();
    void queryWithLibtrackerSparql_data();

    void queryWithLibtrackerSparqlInThread();
    void queryWithLibtrackerSparqlInThread_data();

    void threadCreatingOverhead();
    void generateResultsReport();
private:
    void appendResult(QString name, int median, int mean, int total);
    QDomDocument result;
};

namespace {
    // Query strings

    // Test data for running this can be found in the tracker project.
const QString artistsAndAlbumsQuery =
    "SELECT nmm:artistName(?artist) GROUP_CONCAT(nie:title(?album),'|') "
    "WHERE "
    "{ "
    "?song a nmm:MusicPiece . "
    "?song nmm:performer ?artist . "
    "?song nmm:musicAlbum ?album . "
        "} GROUP BY ?artist";

const int artistsAndAlbumsColumnCount = 2;

const QString musicQuery =
    "select ?song ?title "
    "tracker:coalesce(nmm:artistName(nmm:performer(?song)), 'UNKNOWN_ARTIST') "
    "tracker:coalesce(nie:title(nmm:musicAlbum(?song)), 'UNKNOWN_ALBUM') "
    "nfo:duration(?song) "
    "( EXISTS { ?song nao:hasTag nao:predefined-tag-favorite } ) "
    "nfo:genre(?song) nie:contentCreated(?song) "
    "tracker:id(?song) tracker:id(nmm:performer(?song)) "
    "tracker:id(nmm:musicAlbum(?song)) "
    "where { ?song a nmm:MusicPiece . "
    "?song nie:title ?title . } "
    "order by ?title "
    "<http://www.tracker-project.org/ontologies/tracker#id>(?song)";

const int musicQueryColumnCount = 11;

void readValuesFromCursor(TrackerSparqlCursor* cursor,
                          QList<QVector<QVariant> >& values)
{
    QString value;
    gint n_columns = -1;
    GError* error = 0;
    while (tracker_sparql_cursor_next (cursor, 0, &error)) {
        if (error) {
            g_error_free(error);
            return;
        }
        if (n_columns < 0)
            n_columns = tracker_sparql_cursor_get_n_columns(cursor);
        QVector<QVariant> row;
        row.reserve(n_columns);
        for (int i = 0; i < n_columns; i++) {
            // Simulating what QtSparql does with the data: get the
            // value as string, do conversion to utf-8, get the type,
            // and store the value as QList of QVector of QVariant.
            /*const TrackerSparqlValueType type =*/ tracker_sparql_cursor_get_value_type(cursor, i);
            const gchar* stringValue = tracker_sparql_cursor_get_string(cursor, i, 0);
            row.append(QVariant(QString::fromUtf8(stringValue)));
        }
        values.append(row);
    }
}

}

tst_QSparqlBenchmark::tst_QSparqlBenchmark() : result("Benchmark_results")
{
    QString description("Some description of what was improved "
                        "comparing to previous verion");
    QDomElement benchmarks = result.createElement("benchmark");
    QDomElement asset = result.createElement("asset");
    QDomElement tests = result.createElement("tests");
    QDomElement created = result.createElement("created");
    QDomElement descriptionNode = result.createElement("description");
    QDomText date = result.createTextNode(QDate::currentDate().toString());
    QDomText text = result.createTextNode(description);
    created.appendChild(date);
    descriptionNode.appendChild(text);
    asset.appendChild(created);
    asset.appendChild(descriptionNode);
    result.appendChild(benchmarks);
    benchmarks.appendChild(asset);
    benchmarks.appendChild(tests);
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
    QFETCH(QString, benchmarkName);
    QFETCH(QString, connectionName);
    QFETCH(QString, queryString);
    QFETCH(int, executionMethod);

    QSparqlQuery query(queryString);
    QSparqlConnection conn(connectionName);

    // Note: connection opening cost is left out of the benchmark. If connection
    // opening is async, we need to wait here long enough to know that it has
    // opened (there is no signal sent or such to know it has opened).
    if (executionMethod == QSparqlQueryOptions::AsyncExec)
        QTest::qWait(2000);

    QSparqlQueryOptions queryOptions;
    queryOptions.setExecutionMethod((QSparqlQueryOptions::ExecutionMethod)executionMethod);

    QSparqlResult* r = 0;
    QList<int> totalTimes;
    // We run multiple queries here (and don't leave it for QBENCHMARK to
    // run this multiple times, to be able to measure things like "how much
    // does adding a QThreadPool help".
    for (int i = 0; i < NO_QUERIES; ++i) {
        START_BENCHMARK {
            r = conn.exec(query, queryOptions);
            if (executionMethod == QSparqlQueryOptions::AsyncExec)
                r->waitForFinished();
            CHECK_QSPARQL_RESULT(r);
            delete r;
        }
        END_BENCHMARK(benchmarkName);
        totalTimes.append(benchmarkTotal);
    }
    PRINT_STATS(benchmarkName, totalTimes);
}

void tst_QSparqlBenchmark::queryBenchmark_data()
{
    QTest::addColumn<QString>("benchmarkName");
    QTest::addColumn<QString>("connectionName");
    QTest::addColumn<int>("executionMethod");
    QTest::addColumn<QString>("queryString");

    QTest::newRow("TrackerDBusArtistsAndAlbums")
        << "dbus-artistsandalbums"
        << "QTRACKER"
        << (int)QSparqlQueryOptions::AsyncExec
        << artistsAndAlbumsQuery;

    QTest::newRow("TrackerDirectArtistsAndAlbums-Async")
        << "direct-artistsandalbums-Async"
        << "QTRACKER_DIRECT"
        << (int)QSparqlQueryOptions::AsyncExec
        << artistsAndAlbumsQuery;
    QTest::newRow("TrackerDirectArtistsAndAlbums-Sync")
        << "direct-artistandalbums-Sync"
        << "QTRACKER_DIRECT"
        << (int)QSparqlQueryOptions::SyncExec
        << artistsAndAlbumsQuery;

}

void tst_QSparqlBenchmark::dataReadingBenchmark()
{
    QFETCH(QString, benchmarkName);
    QFETCH(QString, queryString);
    QFETCH(int, columnCount);
    QFETCH(int, executionMethod);

    // Set the dataReadyInterval to be large enough so that it won't affect this
    // test case.
    QSparqlConnectionOptions opts;
    opts.setDataReadyInterval(1000000);

    QSparqlQueryOptions queryOptions;
    queryOptions.setExecutionMethod((QSparqlQueryOptions::ExecutionMethod)executionMethod);
    QSparqlConnection conn("QTRACKER_DIRECT", opts);
    // Note: connection opening cost is left out of the benchmark. Connection
    // opening is async, so we need to wait here long enough to know that it has
    // opened (there is no signal sent or such to know it has opened).
    QTest::qWait(2000);

    QSparqlQuery query(queryString);

    QString finished = benchmarkName + "-fin";
    QString read = benchmarkName + "-read";

    QSparqlResult* r = 0;
    QList<int> totalTimesFinished;
    QList<int> totalTimesRead;

    for (int i = 0; i < NO_QUERIES; ++i) {
        {
            START_BENCHMARK {
                r = conn.exec(query,queryOptions);
                if(executionMethod == QSparqlQueryOptions::AsyncExec) {
                    QEventLoop loop;
                    connect(r, SIGNAL(finished()), &loop, SLOT(quit()));
                    loop.exec();
                }
            }
            END_BENCHMARK(finished);
            totalTimesFinished.append(benchmarkTotal);
        }
        CHECK_QSPARQL_RESULT(r);
        {
            int size = 0;
            START_BENCHMARK {
                while (r->next()) {
                    for (int c = 0; c < columnCount; ++c) {
                        QVariant var = r->value(c);
                        // Probably it's enough not to do anything with the
                        // value; the statement of getting the value won't be
                        // optimized out since calling r->value() might have
                        // side effects.
                    }
                    ++size;
                }
            }
            END_BENCHMARK(read);
            totalTimesRead.append(benchmarkTotal);
            // For verifying that enough data was really read
            // qDebug() << "No of results" << size;
        }
        delete r;
    }
    PRINT_STATS(finished, totalTimesFinished);
    PRINT_STATS(read, totalTimesRead);
}

void tst_QSparqlBenchmark::dataReadingBenchmark_data()
{
    QTest::addColumn<QString>("benchmarkName");
    QTest::addColumn<QString>("queryString");
    QTest::addColumn<int>("columnCount");
    QTest::addColumn<int>("executionMethod");

    QTest::newRow("ReadingArtistsAndAlbums-Async")
        << "read-artistsandalbums-Async"
        << artistsAndAlbumsQuery
        << artistsAndAlbumsColumnCount
        << (int)QSparqlQueryOptions::AsyncExec;

    QTest::newRow("ReadingArtistsAndAlbums-Sync")
        << "read-artistsandalbums-Sync"
        << artistsAndAlbumsQuery
        << artistsAndAlbumsColumnCount
        << (int)QSparqlQueryOptions::SyncExec;

    QTest::newRow("ReadingMusic-Async")
        << "read-music-Async"
        << musicQuery
        << musicQueryColumnCount
        << (int)QSparqlQueryOptions::AsyncExec;

    QTest::newRow("ReadingMusic-Sync")
        << "read-music-Sync"
        << musicQuery
        << musicQueryColumnCount
        << (int)QSparqlQueryOptions::SyncExec;
}

void tst_QSparqlBenchmark::queryWithLibtrackerSparql()
{
    g_type_init();
    QFETCH(QString, benchmarkName);
    QFETCH(QString, queryString);
    GError* error = 0;
    TrackerSparqlConnection* connection = tracker_sparql_connection_get(0, &error);
    QVERIFY(connection);
    QVERIFY(error == 0);
    QList<int> totalTimes;
    for (int i = 0; i < NO_QUERIES; ++i) {
        START_BENCHMARK {
            TrackerSparqlCursor* cursor =
                tracker_sparql_connection_query(connection,
                                                queryString.toUtf8(),
                                                NULL,
                                                &error);

            QVERIFY(error == 0);
            QVERIFY(cursor);

            QList<QVector<QVariant> > values;

            readValuesFromCursor(cursor, values);

            QVERIFY(values.size() > 0);

            g_object_unref(cursor);
        }
        END_BENCHMARK(benchmarkName);
        totalTimes.append(benchmarkTotal);
    }

    g_object_unref(connection);
    PRINT_STATS(benchmarkName,totalTimes);
}

void tst_QSparqlBenchmark::queryWithLibtrackerSparql_data()
{
    QTest::addColumn<QString>("benchmarkName");
    QTest::addColumn<QString>("queryString");

    QTest::newRow("LibtrackerSparqlArtistsAndAlbums")
        << "lts-artistsandalbums"
        << artistsAndAlbumsQuery;

    QTest::newRow("LibtrackersparqlMusic")
        << "lts-music"
        << musicQuery;
}

namespace {
class QueryRunner : public QThread
{
public:
    QueryRunner(TrackerSparqlConnection* c, const QString& q, bool d = false)
        : connection(c), queryString(q), isDummy(d), hasRun(false)
        {
        }
    void run()
        {
            hasRun = true;
            if (isDummy)
                return;

            GError* error = 0;
            TrackerSparqlCursor* cursor =
                tracker_sparql_connection_query(connection,
                                                queryString.toUtf8(),
                                                NULL,
                                                &error);

            QVERIFY(error == 0);
            QVERIFY(cursor);

            QList<QVector<QVariant> > values;

            readValuesFromCursor(cursor, values);

            QVERIFY(values.size() > 0);
            g_object_unref(cursor);
        }
    TrackerSparqlConnection* connection;
    QString queryString;
    bool isDummy;
    bool hasRun;
};

} // unnamed namespace

void tst_QSparqlBenchmark::queryWithLibtrackerSparqlInThread()
{
    g_type_init();
    QFETCH(QString, benchmarkName);
    benchmarkName += QString("-thread");
    QFETCH(QString, queryString);
    GError* error = 0;
    TrackerSparqlConnection* connection = tracker_sparql_connection_get(0, &error);
    QVERIFY(connection);
    QVERIFY(error == 0);
    QList<int> totalTimes;
    for (int i = 0; i < NO_QUERIES; ++i) {
        START_BENCHMARK {
            QueryRunner runner(connection, queryString);
            runner.start();
            runner.wait();
        }
        END_BENCHMARK(benchmarkName);
        totalTimes.append(benchmarkTotal);
    }

    g_object_unref(connection);
    PRINT_STATS(benchmarkName, totalTimes);
}

void tst_QSparqlBenchmark::queryWithLibtrackerSparqlInThread_data()
{
    queryWithLibtrackerSparql_data();
}


void tst_QSparqlBenchmark::threadCreatingOverhead()
{
    QBENCHMARK {
        for (int i = 0; i < NO_QUERIES; ++i) {
            QueryRunner runner(0, "", true);
            runner.start();
            runner.wait();
            QVERIFY(runner.hasRun);
        }
    }
}

void tst_QSparqlBenchmark::generateResultsReport()
{
    QDomElement tests = result.firstChild().namedItem("tests").toElement();
    tests.setAttribute("count", tests.elementsByTagName("test").count());
    //include date in filename so it can be sorted by filename and still have chronological order
    QString coreFilenameTemplate("benchmark-%1-%2-%3");
    QString filenameTemplate("%1.run-%2.xml");
    QString dirPath = QDir::homePath() + QDir::separator();
    int y, m, d;
    QDate::currentDate().getDate(&y, &m, &d);
    QString coreFilename = coreFilenameTemplate.arg(y).arg(m, 2, 10, QLatin1Char('0'))
                                                        .arg(d, 2, 10, QLatin1Char('0'));
    int run =1;
    QString filePath= dirPath + filenameTemplate.arg(coreFilename).arg(run);
    QFileInfo fi(filePath);
    while(fi.exists())
    {
        run++;
        filePath= dirPath + filenameTemplate.arg(coreFilename).arg(run);
        fi.setFile(filePath);
    }
    QFile data(filePath);
    if (data.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&data);
        result.save(out, 4);
    }
    else
        qDebug() << "Could not open "<< filePath <<" for writing. Check permissions";
}

void tst_QSparqlBenchmark::appendResult(QString name, int median, int mean, int total)
{
    QDomNode test = result.createElement("test");
    test.toElement().setAttribute("name", name);
    result.firstChild().namedItem("tests").appendChild(test);
    QDomNode medianNode = result.createElement("median");
    medianNode.toElement().setAttribute("value", median);
    test.appendChild(medianNode);
    QDomNode meanNode = result.createElement("mean");
    meanNode.toElement().setAttribute("value", mean);
    test.appendChild(meanNode);
    QDomNode totalNode = result.createElement("total");
    totalNode.toElement().setAttribute("value", total);
    test.appendChild(totalNode);
}

QTEST_MAIN(tst_QSparqlBenchmark)
#include "tst_qsparql_benchmark.moc"
