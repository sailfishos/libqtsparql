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

// This is a tracker-specific header file. The test also needs to be linked
// against libqsparqltracker.so (not only libqtsparql.so).
#include "QtSparql/qsparql_tracker_signals.h"

//const QString qtest(qTableName( "qtest", __FILE__ )); // FIXME: what's this

//TESTED_FILES=

class Receiver : public QObject
{
    Q_OBJECT

public slots:
    void changed(QList<QTrackerChangeNotifier::Quad> d, QList<QTrackerChangeNotifier::Quad> i);
public:
    QList<QTrackerChangeNotifier::Quad> deletes;
    QList<QTrackerChangeNotifier::Quad> inserts;
};

void Receiver::changed(QList<QTrackerChangeNotifier::Quad> d, QList<QTrackerChangeNotifier::Quad> i)
{
    deletes.append(d);
    inserts.append(i);
}

class tst_QSparqlTrackerSignals : public QObject
{
    Q_OBJECT

public:
    tst_QSparqlTrackerSignals();
    virtual ~tst_QSparqlTrackerSignals();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void contact_added();
private:
    QString className;
    int typeId, personContactId, nameGivenId;
};

tst_QSparqlTrackerSignals::tst_QSparqlTrackerSignals()
{
}

tst_QSparqlTrackerSignals::~tst_QSparqlTrackerSignals()
{
}

void tst_QSparqlTrackerSignals::initTestCase()
{
    // For running the test without installing the plugins. Should work in
    // normal and vpath builds.
    QCoreApplication::addLibraryPath("../../../plugins");

    // Query: what's the current name of the contacts class?
    QSparqlConnection conn("QTRACKER");
    QSparqlQuery q("select tracker:uri(tracker:id(nco:PersonContact)) "
                   "tracker:id(rdf:type) "
                   "tracker:id(nco:PersonContact) "
                   "tracker:id(nco:nameGiven) "
                   "{}");
    QSparqlResult* r = conn.exec(q);
    r->waitForFinished();
    if (r->hasError()) {
        qWarning() << r->lastError().message();
        qFatal("Initial query: error");
    }
    if (r->next() && r->current().count() == 4) {
        className = r->value(0).toString();
        typeId = r->value(1).toInt();
        personContactId = r->value(2).toInt();
        nameGivenId = r->value(3).toInt();
    }
    else
        qFatal("Initial query: not enough data");
    delete r;
}

void tst_QSparqlTrackerSignals::cleanupTestCase()
{
}

void tst_QSparqlTrackerSignals::init()
{
}

void tst_QSparqlTrackerSignals::cleanup()
{
}

// For QSignalSpy
Q_DECLARE_METATYPE(QList<QTrackerChangeNotifier::Quad>)

void tst_QSparqlTrackerSignals::contact_added()
{
    QTrackerChangeNotifier notifier(className);

    // For QSignalSpy
    qRegisterMetaType<QList<QTrackerChangeNotifier::Quad> >("QList<QTrackerChangeNotifier::Quad>");

    QSignalSpy spy(&notifier,
                   SIGNAL(changed(QList<QTrackerChangeNotifier::Quad>,
                                  QList<QTrackerChangeNotifier::Quad>)));

    // TODO: also read the parameters from the spy.
    Receiver receiver;
    QObject::connect(&notifier,
                     SIGNAL(changed(QList<QTrackerChangeNotifier::Quad>,
                                    QList<QTrackerChangeNotifier::Quad>)),
                     &receiver,
                     SLOT(changed(QList<QTrackerChangeNotifier::Quad>,
                                  QList<QTrackerChangeNotifier::Quad>)));

    // Now do an insert...
    QSparqlQuery q("insert { <added.uri> a nco:PersonContact ;"
                   "nie:isLogicalPartOf <qsparql-tracker-tests> ;"
                   "nco:nameGiven \"temp.name\" .}", QSparqlQuery::InsertStatement);
    QSparqlConnection conn("QTRACKER");
    QSparqlResult* r = conn.exec(q);
    QVERIFY(r != 0);
    QVERIFY(!r->hasError());
    r->waitForFinished();
    QVERIFY(!r->hasError());

    // And process the events so that
    // 1) tracker sends the signal through D-Bus
    // 2) QTrackerChangeNotifier gets the signal and sends it to the spy

    QTime timer;
    timer.start();
    while(timer.elapsed() < 5000 && !spy.count())
        QCoreApplication::processEvents();

    QCOMPARE(spy.count(), 1);

    QVERIFY(receiver.deletes.size() == 0);
    QVERIFY(receiver.inserts.size() == 3);

    // The triples that are inserted:
    // newid rdf:type nco:PersonContact
    // newid nie:isLogicalPartOf testcontext
    // newid nameGiven 0

    // The graph is the default graph
    QCOMPARE(receiver.inserts[0].graph, 0);
    QCOMPARE(receiver.inserts[1].graph, 0);
    QCOMPARE(receiver.inserts[2].graph, 0);

    // The newid is the same for all rows
    QCOMPARE(receiver.inserts[0].subject, receiver.inserts[1].subject);
    QCOMPARE(receiver.inserts[0].subject, receiver.inserts[2].subject);

    //qDebug() << receiver.inserts;

    bool typeFound = false;
    bool nameFound = false;
    for (int i=0; i<3; ++i) {
        if (receiver.inserts[i].predicate == typeId && receiver.inserts[i].object == personContactId)
            typeFound = true;
        else if (receiver.inserts[i].predicate == nameGivenId && receiver.inserts[i].object == 0)
            nameFound = true;
    }
    QVERIFY(typeFound);
    QVERIFY(nameFound);
}

QTEST_MAIN( tst_QSparqlTrackerSignals )
#include "tst_qsparql_tracker_signals.moc"
