/****************************************************************************
**
** Copyright (C) 2010-2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (ivan.frade@nokia.com)
**
** This file is part of the QtSparql module (not yet part of the Qt Toolkit).
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

#include "qsparql_endpoint_p.h"

#include <qcoreapplication.h>
#include <qvariant.h>
#include <qdatetime.h>
#include <qsparqlerror.h>
#include <qsparqlbinding.h>
#include <qsparqlquery.h>
#include <qsparqlqueryoptions.h>
#include <qsparqlresultrow.h>
#include <private/qsparqlntriples_p.h>

#include <qstringlist.h>
#include <qtextcodec.h>
#include <qvector.h>
#include <QtCore/qeventloop.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qurl.h>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtCore/qurlquery.h>
#endif

#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkrequest.h>
#include <QtNetwork/qnetworkreply.h>
#include <QtNetwork/qnetworkproxy.h>
#include <QtNetwork/qauthenticator.h>

#include <QtXml/QDomNode>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QtXml/QXmlDefaultHandler>
#include <QtXml/QXmlInputSource>

#include <qdebug.h>

QT_BEGIN_NAMESPACE

class XmlResultsParser;

struct EndpointDriverPrivate {
    EndpointDriverPrivate()
        : manager(0), managerOwned(false)
    {
    }
    QSparqlConnectionOptions options;
    QUrl url;
    QString user;
    QString password;
#ifndef QT_NO_NETWORKPROXY
    QNetworkProxy proxy;
#endif
    QNetworkAccessManager *manager;
    bool managerOwned;
};

// This class is only needed for debugging reasons
class XmlInputSource : public QXmlInputSource
{
public:
    XmlInputSource(QIODevice * dev) : QXmlInputSource(dev) {}

    void fetchData()
    {
        QXmlInputSource::fetchData();
        // qDebug() << "data" << data();
    }
};

class XmlResultsParser : public QXmlDefaultHandler
{
public:
    XmlResultsParser(EndpointResultPrivate * res) : d(res)
    {
    }

    bool startElement(  const QString & namespaceURI,
                        const QString & localName,
                        const QString &qName,
                        const QXmlAttributes &attributes);

    bool endElement(    const QString & namespaceURI,
                        const QString & localName,
                        const QString &qName);

    bool characters(const QString &str);
    bool fatalError(const QXmlParseException &exception);
    QString errorString() const;

private:
    QString currentText;
    QString errorStr;
    QXmlAttributes lattrs;
    QSparqlBinding binding;
    QSparqlResultRow resultRow;
    EndpointResultPrivate * d;
};

class EndpointResultPrivate  : public QObject {
    Q_OBJECT
public:
    EndpointResultPrivate(EndpointResult *result, EndpointDriverPrivate *dpp)
    : reply(0), xml(0), parser(0), reader(0),
        isFinished(false), noResults(false), loop(0), q(result), driverPrivate(dpp)
    {
    }

    ~EndpointResultPrivate()
    {
        delete xml;
        delete parser;
        delete reader;
    }

    void setBoolValue(bool v)
    {
        q->setBoolValue(v);
    }

    QNetworkReply *reply;
    QByteArray buffer;
    XmlInputSource *xml;
    XmlResultsParser *parser;
    QXmlSimpleReader *reader;
    QVector<QSparqlResultRow> results;
    bool isFinished;
    bool noResults;
    QEventLoop *loop;
    EndpointResult *q;
    EndpointDriverPrivate *driverPrivate;

public Q_SLOTS:
    void authenticate(QNetworkReply * reply, QAuthenticator * authenticator);
    void readData();
    void handleError(QNetworkReply::NetworkError code);
    void terminate();
    void parseResults();
};


bool XmlResultsParser::startElement(const QString & namespaceURI,
                            const QString & localName,
                            const QString &qName,
                            const QXmlAttributes &attributes)
{
    Q_UNUSED(namespaceURI);
    Q_UNUSED(localName);

    currentText = QString();

    if (qName == QLatin1String("sparql")) {
    } else if (qName == QLatin1String("head")) {
    } else if (qName == QLatin1String("variable")) {
    } else if (qName == QLatin1String("results")) {
    } else if (qName == QLatin1String("result")) {
        resultRow = QSparqlResultRow();
    } else if (qName == QLatin1String("binding")) {
        binding = QSparqlBinding();
        binding.setName(attributes.value(QString::fromLatin1("name")));
    } else if (qName == QLatin1String("bnode")) {
    } else if (qName == QLatin1String("uri")) {
    } else if (qName == QLatin1String("literal")) {
        lattrs = attributes;
    } else {
    }

    return true;
}

bool XmlResultsParser::endElement(const QString & namespaceURI,
                            const QString & localName,
                            const QString &qName)
{
    Q_UNUSED(namespaceURI);
    Q_UNUSED(localName);

    if (!d->noResults) {
        if (qName == QLatin1String("sparql")) {
        } else if (qName == QLatin1String("head")) {
        } else if (qName == QLatin1String("variable")) {
        } else if (qName == QLatin1String("results")) {
        } else if (qName == QLatin1String("result")) {
            d->results.append(resultRow);
        } else if (qName == QLatin1String("binding")) {
            resultRow.append(binding);
        } else if (qName == QLatin1String("boolean")) {
            bool boolValue = currentText.toLower() == QLatin1String("true");
            d->setBoolValue(boolValue);
            binding.setValue(QVariant(boolValue));
            resultRow.append(binding);
            d->results.append(resultRow);
        } else if (qName == QLatin1String("bnode")) {
            currentText.replace(QRegExp(QString::fromLatin1("^nodeID://")), QString::fromLatin1(""));
            currentText.replace(QRegExp(QString::fromLatin1("^_:")), QString::fromLatin1(""));
            binding.setBlankNodeLabel(currentText);
        } else if (qName == QLatin1String("uri")) {
            QUrl url(currentText);
            binding.setValue(QVariant(url));
        } else if (qName == QLatin1String("literal")) {
            if (lattrs.index(QString::fromLatin1("datatype")) != -1) {
                if (lattrs.index(QString::fromLatin1("xsi:type")) != -1) {
                    // TODO: How should we treat xsi:types here?
                    binding.setValue(currentText, QUrl(lattrs.value(QString::fromLatin1("datatype"))));
                } else {
                    binding.setValue(currentText, QUrl(lattrs.value(QString::fromLatin1("datatype"))));
                }
            } else if (lattrs.index(QString::fromLatin1("xml:lang")) != -1) {
                binding.setValue(QVariant(currentText));
                binding.setLanguageTag(lattrs.value(QString::fromLatin1("xml:lang")));
            } else {
                binding.setValue(QVariant(currentText));
            }
        } else {
        }
    }

    return true;
}

bool XmlResultsParser::characters(const QString &str)
{
    currentText += str;
    return true;
}

bool XmlResultsParser::fatalError(const QXmlParseException &exception)
{
    Q_UNUSED(exception);
    return false;
}

QString XmlResultsParser::errorString() const
{
    return errorStr;
}

void EndpointResultPrivate::authenticate(QNetworkReply * reply, QAuthenticator * authenticator)
{
    Q_UNUSED(reply);
    authenticator->setUser(driverPrivate->user);
    authenticator->setPassword(driverPrivate->password);
}

void EndpointResultPrivate::handleError(QNetworkReply::NetworkError code)
{
    if (code != QNetworkReply::UnknownContentError) {
        q->setLastError(QSparqlError(reply->errorString(), QSparqlError::ConnectionError, code));
        terminate();
        qWarning() << "QEndpoint:" << q->lastError() << q->query();
    }
    terminate();
}

void EndpointResultPrivate::terminate()
{
    if (isFinished)
        return;

    isFinished = true;
    q->Q_EMIT finished();
    
    if (loop != 0)
        loop->exit();
}

void EndpointResultPrivate::readData()
{
    if (isFinished) {
        reply->readAll();
        return;
    }

    if (q->isGraph()) {
        buffer += reply->readAll();
        return;
    }

    if (reader == 0) {
        parser = new XmlResultsParser(this);
        reader = new QXmlSimpleReader();
        reader->setContentHandler(parser);
        reader->setErrorHandler(parser);

        if (!reader->parse(xml, true)) {
            q->setLastError(QSparqlError(xml->data(), QSparqlError::StatementError));
            terminate();
            qWarning() << "QEndpoint:" << q->lastError() << q->query();
            return;
        }
    }

    while (reply->bytesAvailable() > 0) {
        if (!reader->parseContinue()) {
            q->setLastError(QSparqlError(xml->data(), QSparqlError::StatementError));
            terminate();
            qWarning() << "QEndpoint:" << q->lastError() << q->query();
            return;
        }
    }

    q->Q_EMIT dataReady(results.count());
}

void EndpointResultPrivate::parseResults()
{ 
    if (isFinished)
        return;

    if (q->isGraph()) {
        QSparqlNTriples parser(buffer);
        results = parser.parse();
    }

    terminate();    
    return;
}

EndpointResult::EndpointResult(EndpointDriverPrivate* p)
{
    d = new EndpointResultPrivate(this, p);
}

EndpointResult::~EndpointResult()
{
    cleanup();
    delete d;
}

QVariant EndpointResult::handle() const
{
    return QVariant();
}

void EndpointResult::cleanup()
{
    // if we still have a network manager,
    // delete the previous result here
    if (d->driverPrivate)
        delete d->reply;
    d->reply = 0;
}

QSparqlBinding EndpointResult::binding(int field) const
{
    if (!isValid()) {
        return QSparqlBinding();
    }

    if (field >= d->results[pos()].count() || field < 0) {
        qWarning() << "EndpointResult::data[" << pos() << "]: column" << field << "out of range";
        return QSparqlBinding();
    }

    return d->results[pos()].binding(field);
}

QVariant EndpointResult::value(int field) const
{
    if (!isValid()) {
        return QVariant();
    }

    if (field >= d->results[pos()].count() || field < 0) {
        qWarning() << "EndpointResult::data[" << pos() << "]: column" << field << "out of range";
        return QVariant();
    }

    return d->results[pos()].value(field);
}

// This is just a temporary hack; eventually this should be refactored so that
// the work is done here instead of Result::exec.
EndpointResult* EndpointDriver::exec(const QString& query, QSparqlQuery::StatementType type, const QSparqlQueryOptions& options)
{
    if (options.executionMethod() == QSparqlQueryOptions::SyncExec)
        return 0;

    EndpointResult* res = createResult();
    res->exec(query, type, prefixes());
    return res;
}

bool EndpointResult::exec(const QString& query, QSparqlQuery::StatementType type, const QString& prefixes)
{
    QUrl queryUrl(d->driverPrivate->url);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QUrlQuery urlQuery(queryUrl);
    urlQuery.addQueryItem(QLatin1String("query"), prefixes + query);
#else
    queryUrl.addQueryItem(QLatin1String("query"), prefixes + query);
#endif
    setQuery(query);
    setStatementType(type);

    // Virtuoso protocol extension options - timeout and maxrows
    QVariant timeout = d->driverPrivate->options.option(QLatin1String("timeout"));
    if (timeout.isValid()) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        urlQuery.addQueryItem(QLatin1String("timeout"), timeout.toString());
#else
        queryUrl.addQueryItem(QLatin1String("timeout"), timeout.toString());
#endif
    }

    QVariant maxrows = d->driverPrivate->options.option(QLatin1String("maxrows"));
    if (maxrows.isValid()) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        urlQuery.addQueryItem(QLatin1String("maxrows"), maxrows.toString());
#else
        queryUrl.addQueryItem(QLatin1String("maxrows"), maxrows.toString());
#endif
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    queryUrl.setQuery(urlQuery);
#endif

    // qDebug() << "Real url to run.... " << queryUrl.toString();

    d->buffer.clear();
    QNetworkRequest request(queryUrl);

    if (isGraph())
        // A Virtuoso protocol extension for CONSTRUCT or DESCRIBE queries.
        // With DBPedia, 'text/plain' returns triples, but it isn't documented
        // in the Virtuoso manual
        request.setRawHeader("Accept", "text/plain");
    else
        request.setRawHeader("Accept", "application/sparql-results+xml");

    request.setRawHeader("charset", "utf-8");

    d->reply = d->driverPrivate->manager->get(request);

    if (!isGraph())
        d->xml = new XmlInputSource(d->reply);

    // We don't want to add any results if it's an insert or delete, however, we still need to parse them
    // because there may be warnings that need to be printed
    if (statementType() == QSparqlQuery::InsertStatement || statementType() == QSparqlQuery::DeleteStatement)
        d->noResults = true;

    QObject::connect(d->reply, SIGNAL(readyRead()), d, SLOT(readData()));
    QObject::connect(d->reply, SIGNAL(finished()), d, SLOT(parseResults()));
    QObject::connect(d->reply, SIGNAL(error(QNetworkReply::NetworkError)), d, SLOT(handleError(QNetworkReply::NetworkError)));

    if (!d->driverPrivate->user.isEmpty() && !d->driverPrivate->password.isEmpty()) {
        QObject::connect(d->driverPrivate->manager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator *)),
                         d, SLOT(authenticate(QNetworkReply *, QAuthenticator *)));
    }

    return true;
}

void EndpointResult::waitForFinished()
{
    if (d->isFinished)
        return;

    QEventLoop loop;
    d->loop = &loop;
    loop.exec();
    d->loop = 0;
}

bool EndpointResult::isFinished() const
{
    return d->isFinished;
}

int EndpointResult::size() const
{
    return d->results.count();
}

QSparqlResultRow EndpointResult::current() const
{
    if (!isValid()) {
        return QSparqlResultRow();
    }

    if (pos() < 0 || pos() >= d->results.count()) {
        return QSparqlResultRow();
    }

    return d->results[pos()];
}

EndpointDriver::EndpointDriver(QObject * parent)
    : QSparqlDriver(parent)
{
    d = new EndpointDriverPrivate();
}

EndpointDriver::~EndpointDriver()
{
    if (d->managerOwned) {
        delete d->manager;
        d->managerOwned = false;
    }
    delete d;
}

bool EndpointDriver::hasFeature(QSparqlConnection::Feature f) const
{
    switch (f) {
    case QSparqlConnection::QuerySize:
    case QSparqlConnection::AskQueries:
    case QSparqlConnection::ConstructQueries:
    case QSparqlConnection::UpdateQueries:
    case QSparqlConnection::AsyncExec:
        return true;
    case QSparqlConnection::DefaultGraph:
    case QSparqlConnection::SyncExec:
        return false;
    default:
        return false;
    }
}

bool EndpointDriver::hasError() const
{
    return false;
}

bool EndpointDriver::open(const QSparqlConnectionOptions& options)
{
    if (isOpen())
        close();

    d->options = options;
    d->url.setHost(options.hostName());

    if (options.path().isEmpty())
        d->url.setPath(QLatin1String("sparql"));
    else
        d->url.setPath(options.path());

    d->url.setScheme(QLatin1String("http"));
    if (options.port() != -1)
        d->url.setPort(options.port());
    d->user = options.userName();
    d->password = options.password();

    if (d->managerOwned)
        delete d->manager;
    d->manager = 0;
    d->managerOwned = false;

    d->manager = options.networkAccessManager();
    if (!d->manager) {
        // Options didn't give us a QNetworkAccessManager, create our own and
        // mark that we need to delete it.
        d->manager = new QNetworkAccessManager();
        d->managerOwned = true;
    }

#ifndef QT_NO_NETWORKPROXY
    d->proxy = options.proxy();
    if (d->proxy.type() != QNetworkProxy::NoProxy)
        d->manager->setProxy(d->proxy);
#endif

    setOpen(true);
    setOpenError(false);
    return true;
}

void EndpointDriver::close()
{
    Q_EMIT closing();
    if (isOpen()) {
        setOpen(false);
        setOpenError(false);
        d->url = QUrl();
        d->user = QString();
        d->password = QString();
    }
}

EndpointResult* EndpointDriver::createResult() const
{
    EndpointResult *result = new EndpointResult(d);
    QObject::connect(this, SIGNAL(closing()), result, SLOT(driverClosing()));
    return result;
}

void EndpointResult::driverClosing()
{
    if (!isFinished()) {
        setLastError(QSparqlError(
                QString::fromUtf8("QSparqlConnection closed before QSparqlResult"),
                QSparqlError::ConnectionError));
    }
    d->terminate();

    d->driverPrivate = 0;

    qWarning() << "QEndpointResult: QSparqlConnection closed before QSparqlResult with query:" << query();
}

QT_END_NAMESPACE

#include "qsparql_endpoint.moc"
