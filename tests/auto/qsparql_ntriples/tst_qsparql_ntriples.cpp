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

#include <QtTest/QtTest>
#include <QtSparql>
#include <private/qsparqlntriples_p.h>

class tst_QSparqlNTriples : public QObject
{
    Q_OBJECT

public:
    tst_QSparqlNTriples();
    virtual ~tst_QSparqlNTriples();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void parse_file();
};

tst_QSparqlNTriples::tst_QSparqlNTriples()
{
}

tst_QSparqlNTriples::~tst_QSparqlNTriples()
{
}

void tst_QSparqlNTriples::initTestCase()
{
}

void tst_QSparqlNTriples::cleanupTestCase()
{
}

void tst_QSparqlNTriples::init()
{
}

void tst_QSparqlNTriples::cleanup()
{
}

void tst_QSparqlNTriples::parse_file()
{
    // If the build tree is not the same as the source tree, this file must be
    // copied to the build tree at the moment. The must be some way to do that
    // with qmake
    QFile file("test.nt");
    QCOMPARE(file.open(QIODevice::ReadOnly), true);

    QByteArray buffer = file.readAll();
    QSparqlNTriples parser(buffer);
    QVector<QSparqlResultRow> results = parser.parse();

    QCOMPARE(results[0].binding("s").toString(), QString::fromUtf8("<http://example.org/resource1>"));
    QCOMPARE(results[0].binding("p").toString(), QString::fromUtf8("<http://example.org/property>"));
    QCOMPARE(results[0].binding("o").toString(), QString::fromUtf8("<http://example.org/resource2>"));

    QCOMPARE(results[1].binding("s").toString(), QString::fromUtf8("_:anon"));
    QCOMPARE(results[1].binding("p").toString(), QString::fromUtf8("<http://example.org/property>"));
    QCOMPARE(results[1].binding("o").toString(), QString::fromUtf8("<http://example.org/resource2>"));

    QCOMPARE(results[2].binding("s").toString(), QString::fromUtf8("<http://example.org/resource2>"));
    QCOMPARE(results[2].binding("p").toString(), QString::fromUtf8("<http://example.org/property>"));
    QCOMPARE(results[2].binding("o").toString(), QString::fromUtf8("_:anon"));

    QCOMPARE(results[3].binding("s").toString(), QString::fromUtf8("<http://example.org/resource3>"));
    QCOMPARE(results[3].binding("p").toString(), QString::fromUtf8("<http://example.org/property>"));
    QCOMPARE(results[3].binding("o").toString(), QString::fromUtf8("<http://example.org/resource2>"));

    QCOMPARE(results[4].binding("s").toString(), QString::fromUtf8("<http://example.org/resource4>"));
    QCOMPARE(results[4].binding("p").toString(), QString::fromUtf8("<http://example.org/property>"));
    QCOMPARE(results[4].binding("o").toString(), QString::fromUtf8("<http://example.org/resource2>"));

    QCOMPARE(results[5].binding("s").toString(), QString::fromUtf8("<http://example.org/resource5>"));
    QCOMPARE(results[5].binding("p").toString(), QString::fromUtf8("<http://example.org/property>"));
    QCOMPARE(results[5].binding("o").toString(), QString::fromUtf8("<http://example.org/resource2>"));

    QCOMPARE(results[6].binding("s").toString(), QString::fromUtf8("<http://example.org/resource6>"));
    QCOMPARE(results[6].binding("p").toString(), QString::fromUtf8("<http://example.org/property>"));
    QCOMPARE(results[6].binding("o").toString(), QString::fromUtf8("<http://example.org/resource2>"));

    QCOMPARE(results[7].binding("s").toString(), QString::fromUtf8("<http://example.org/resource7>"));
    QCOMPARE(results[7].binding("p").toString(), QString::fromUtf8("<http://example.org/property>"));
    QCOMPARE(results[7].binding("o").toString(), QString::fromUtf8("\"simple literal\""));

    QCOMPARE(results[8].binding("s").toString(), QString::fromUtf8("<http://example.org/resource8>"));
    QCOMPARE(results[8].binding("p").toString(), QString::fromUtf8("<http://example.org/property>"));
    QCOMPARE(results[8].binding("o").toString(), QString::fromUtf8("\"backslash:\\\\\\\\\""));

    QCOMPARE(results[9].binding("s").toString(), QString::fromUtf8("<http://example.org/resource9>"));
    QCOMPARE(results[9].binding("p").toString(), QString::fromUtf8("<http://example.org/property>"));
    QCOMPARE(results[9].binding("o").toString(), QString::fromUtf8("\"dquote:\\\\\\\"\""));

    QCOMPARE(results[10].binding("s").toString(), QString::fromUtf8("<http://example.org/resource10>"));
    QCOMPARE(results[10].binding("p").toString(), QString::fromUtf8("<http://example.org/property>"));
    QCOMPARE(results[10].binding("o").toString(), QString::fromUtf8("\"newline:\\\\n\""));

    QCOMPARE(results[11].binding("s").toString(), QString::fromUtf8("<http://example.org/resource11>"));
    QCOMPARE(results[11].binding("p").toString(), QString::fromUtf8("<http://example.org/property>"));
    QCOMPARE(results[11].binding("o").toString(), QString::fromUtf8("\"return\\\\r\""));

    QCOMPARE(results[12].binding("s").toString(), QString::fromUtf8("<http://example.org/resource12>"));
    QCOMPARE(results[12].binding("p").toString(), QString::fromUtf8("<http://example.org/property>"));
    QCOMPARE(results[12].binding("o").toString(), QString::fromUtf8("\"tab:\\\\t\""));

    QCOMPARE(results[13].binding("s").toString(), QString::fromUtf8("<http://example.org/resource13>"));
    QCOMPARE(results[13].binding("p").toString(), QString::fromUtf8("<http://example.org/property>"));
    QCOMPARE(results[13].binding("o").toString(), QString::fromUtf8("<http://example.org/resource2>"));

    QCOMPARE(results[14].binding("s").toString(), QString::fromUtf8("<http://example.org/resource14>"));
    QCOMPARE(results[14].binding("p").toString(), QString::fromUtf8("<http://example.org/property>"));
    QCOMPARE(results[14].binding("o").toString(), QString::fromUtf8("\"x\""));

    QCOMPARE(results[15].binding("s").toString(), QString::fromUtf8("<http://example.org/resource15>"));
    QCOMPARE(results[15].binding("p").toString(), QString::fromUtf8("<http://example.org/property>"));
    QCOMPARE(results[15].binding("o").toString(), QString::fromUtf8("_:anon"));

    QCOMPARE(results[16].binding("s").toString(), QString::fromUtf8("<http://example.org/resource16>"));
    QCOMPARE(results[16].binding("p").toString(), QString::fromUtf8("<http://example.org/property>"));
    QCOMPARE(results[16].binding("o").toString(), QString::fromUtf8("\"é\""));

    QCOMPARE(results[17].binding("s").toString(), QString::fromUtf8("<http://example.org/resource17>"));
    QCOMPARE(results[17].binding("p").toString(), QString::fromUtf8("<http://example.org/property>"));
    QCOMPARE(results[17].binding("o").toString(), QString::fromUtf8("\"€\""));

    QCOMPARE(results[18].binding("s").toString(), QString::fromUtf8("<http://example.org/resource21>"));
    QCOMPARE(results[18].binding("p").toString(), QString::fromUtf8("<http://example.org/property>"));
    QCOMPARE(results[18].binding("o").toString(), QString::fromUtf8("\"\"^^<http://www.w3.org/2000/01/rdf-schema#XMLLiteral>"));

    QCOMPARE(results[19].binding("s").toString(), QString::fromUtf8("<http://example.org/resource22>"));
    QCOMPARE(results[19].binding("p").toString(), QString::fromUtf8("<http://example.org/property>"));
    QCOMPARE(results[19].binding("o").toString(), QString::fromUtf8("\" \"^^<http://www.w3.org/2000/01/rdf-schema#XMLLiteral>"));

    QCOMPARE(results[20].binding("s").toString(), QString::fromUtf8("<http://example.org/resource23>"));
    QCOMPARE(results[20].binding("p").toString(), QString::fromUtf8("<http://example.org/property>"));
    QCOMPARE(results[20].binding("o").toString(), QString::fromUtf8("\"x\"^^<http://www.w3.org/2000/01/rdf-schema#XMLLiteral>"));

    QCOMPARE(results[21].binding("s").toString(), QString::fromUtf8("<http://example.org/resource23>"));
    QCOMPARE(results[21].binding("p").toString(), QString::fromUtf8("<http://example.org/property>"));
    QCOMPARE(results[21].binding("o").toString(), QString::fromUtf8("\"\\\\\\\"\"^^<http://www.w3.org/2000/01/rdf-schema#XMLLiteral>"));

    QCOMPARE(results[22].binding("s").toString(), QString::fromUtf8("<http://example.org/resource24>"));
    QCOMPARE(results[22].binding("p").toString(), QString::fromUtf8("<http://example.org/property>"));
    QCOMPARE(results[22].binding("o").toString(), QString::fromUtf8("\"<a></a>\"^^<http://www.w3.org/2000/01/rdf-schema#XMLLiteral>"));

    QCOMPARE(results[23].binding("s").toString(), QString::fromUtf8("<http://example.org/resource25>"));
    QCOMPARE(results[23].binding("p").toString(), QString::fromUtf8("<http://example.org/property>"));
    QCOMPARE(results[23].binding("o").toString(), QString::fromUtf8("\"a <b></b>\"^^<http://www.w3.org/2000/01/rdf-schema#XMLLiteral>"));

    QCOMPARE(results[24].binding("s").toString(), QString::fromUtf8("<http://example.org/resource26>"));
    QCOMPARE(results[24].binding("p").toString(), QString::fromUtf8("<http://example.org/property>"));
    QCOMPARE(results[24].binding("o").toString(), QString::fromUtf8("\"a <b></b> c\"^^<http://www.w3.org/2000/01/rdf-schema#XMLLiteral>"));

    QCOMPARE(results[25].binding("s").toString(), QString::fromUtf8("<http://example.org/resource26>"));
    QCOMPARE(results[25].binding("p").toString(), QString::fromUtf8("<http://example.org/property>"));
    QCOMPARE(results[25].binding("o").toString(), QString::fromUtf8("\"a\\\\n<b></b>\\\\nc\"^^<http://www.w3.org/2000/01/rdf-schema#XMLLiteral>"));

    QCOMPARE(results[26].binding("s").toString(), QString::fromUtf8("<http://example.org/resource27>"));
    QCOMPARE(results[26].binding("p").toString(), QString::fromUtf8("<http://example.org/property>"));
    QCOMPARE(results[26].binding("o").toString(), QString::fromUtf8("\"chat\"^^<http://www.w3.org/2000/01/rdf-schema#XMLLiteral>"));

    QCOMPARE(results[27].binding("s").toString(), QString::fromUtf8("<http://example.org/resource30>"));
    QCOMPARE(results[27].binding("p").toString(), QString::fromUtf8("<http://example.org/property>"));
    QCOMPARE(results[27].binding("o").toString(), QString::fromUtf8("\"chat\"@fr"));

    QCOMPARE(results[28].binding("s").toString(), QString::fromUtf8("<http://example.org/resource31>"));
    QCOMPARE(results[28].binding("p").toString(), QString::fromUtf8("<http://example.org/property>"));
    QCOMPARE(results[28].binding("o").toString(), QString::fromUtf8("\"chat\"@en"));

    QCOMPARE(results[29].binding("s").toString(), QString::fromUtf8("<http://example.org/resource32>"));
    QCOMPARE(results[29].binding("p").toString(), QString::fromUtf8("<http://example.org/property>"));
    QCOMPARE(results[29].binding("o").toString(), QString::fromUtf8("\"abc\"^^<http://example.org/datatype1>"));

}

QTEST_MAIN(tst_QSparqlNTriples)
#include "tst_qsparql_ntriples.moc"
