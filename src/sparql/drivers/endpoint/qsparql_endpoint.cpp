/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (ivan.frade@nokia.com)
**
** This file is part of the QtSparql module (not yet part of the Qt Toolkit).
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
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
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you have questions regarding the use of this file, please contact
** Nokia at ivan.frade@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qsparql_endpoint.h"

#include <qcoreapplication.h>
#include <qvariant.h>
#include <qdatetime.h>
#include <qsparqlerror.h>
#include <qsparqlbinding.h>
#include <qsparqlquery.h>
#include <qsparqlresultrow.h>
#include <qstringlist.h>
#include <qtextcodec.h>
#include <qvector.h>
#include <QtCore/qeventloop.h>
#include <QtCore/qstringlist.h>

#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkrequest.h>
#include <QtNetwork/qnetworkreply.h>
#include <QtNetwork/qnetworkproxy.h>

#include <QtXml/QDomNode>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

#include <qdebug.h>

#define GET_NEXT_CHAR   ++i; \
                        prev = ch; \
                        ch = buffer.at(i);

/*  From the raptor ntriples parser                        
    These are for 7-bit ASCII and not locale-specific 
 */
#define IS_ASCII_ALPHA(c) (((c)>0x40 && (c)<0x5B) || ((c)>0x60 && (c)<0x7B))
#define IS_ASCII_UPPER(c) ((c)>0x40 && (c)<0x5B)
#define IS_ASCII_DIGIT(c) ((c)>0x2F && (c)<0x3A)
#define IS_ASCII_PRINT(c) ((c)>0x1F && (c)<0x7F)
#define TO_ASCII_LOWER(c) ((c)+0x20)
                        

QT_BEGIN_NAMESPACE

class NTriplesParser {
public:
    NTriplesParser(QByteArray &b) : buffer(b), i(0) {}
    
    void parseError(QString message) {
        QString context;
        
        while (i < buffer.size()) {
            context.append(QLatin1Char(buffer[i]));
            if (buffer[i] == '\n' || buffer[i] == '\r')
                break;
                
            i++;
        }
        
        qWarning() << "ERROR: NTriples Parser: " << message << ": '" << context << "'";        
    }
    
    void skipWhiteSpace() {
        while (i < buffer.size()) {
            if (buffer[i] != ' ' && buffer[i] != '\t')
                break;
                
            i++;
        }
    }
    
    void skipComment() {
        while (i < buffer.size()) {
            if (buffer[i] == '\n' || buffer[i] == '\r')
                break;
                
            i++;
        }
    }
    
    void skipEoln() {
        if (buffer[i] == '\n') {
            i++;
        } else if (buffer[i] == '\r') {
            i++;
            if (i < buffer.size() && buffer[i] == '\n') {
                i++;
            }
        }
    }
    
    QSparqlBinding parseUri(QString name) {
        QString uri;
        if (buffer[i] == '<') {
            i++;
            while (i < buffer.size()) {
                if (buffer[i] == '>') {
                    i++;
                    break;
                }
                
                uri.append(QLatin1Char(buffer[i]));
                i++;
            }
        }
        
        return QSparqlBinding(name, QUrl(uri));
    }
    
    QSparqlBinding parseNamedNode(QString name) {
        QString namedNode;
        QSparqlBinding binding(name);
        
        i++;
        if (i >= buffer.size() || buffer[i] != ':') {
            parseError(QLatin1String("Expected name node"));
        }
        
        i++;
        if (i < buffer.size() && IS_ASCII_ALPHA((uchar) buffer[i])) {
            while (i < buffer.size()) {
                if (!IS_ASCII_ALPHA((uchar) buffer[i]) && !IS_ASCII_DIGIT((uchar) buffer[i]))
                    break;
                
                namedNode.append(QLatin1Char(buffer[i]));
                i++;
            }
        }
        
        binding.setValue(namedNode);
        return binding;
    }
    
    QString parseLanguageTag() {
        QString languageTag;
        
        if (i < buffer.size() && buffer[i] == '@') {
            i++;
            while (i < buffer.size()) {
                if (!IS_ASCII_ALPHA((uchar) buffer[i]))
                    break;
                
                languageTag.append(QLatin1Char(buffer[i]));
                i++;
            }
        }
        
        return languageTag;
    }
    
    QUrl parseDataTypeUri() {
        QString uri;
        
        if (i < buffer.size() && buffer[i] == '<') {
            i++;
            while (i < buffer.size()) {
                if (buffer[i] == '>') {
                    i++;
                    break;
                }
                
                uri.append(QLatin1Char(buffer[i]));
                i++;
            }
        }
        
        return QUrl(uri);
    }
    
    QSparqlBinding parseLiteral(QString name) {
        QString literal;
        QString languageTag;
        QUrl dataTypeUri;
        QSparqlBinding binding(name);
        
        if (buffer[i] == '"') {
            i++;
            while (i < buffer.size()) {
                if (buffer[i] == '"') {
                    i++;
                    if (i < buffer.size() && buffer[i] == '^') {
                        i++;
                        if (i < buffer.size() && buffer[i] == '^') {
                            i++;
                            dataTypeUri = parseDataTypeUri();
                        }
                    } else if (i < buffer.size() && buffer[i] == '@') {
                        languageTag = parseLanguageTag();
                    }
                    
                    break;
                }
                
                literal.append(QLatin1Char(buffer[i]));
                if (buffer[i] == '\\') {
                    i++;
                    if (i < buffer.size()) {
                        if (buffer[i] == '"' || buffer[i] == 'n' || buffer[i] == 'r' || buffer[i] == 't') {
                            literal.append(QLatin1Char(buffer[i]));
                        } else if (buffer[i] == 'u') {
                            // Unicode escape \uxxxx
                        } else if (buffer[i] == 'U') {
                           // Unicode escape \Uxxxxxxxx
                        } else {
                            parseError(QLatin1String("Invalid literal escape sequence"));
                        }
                    }
                }
                
                i++;
            }
        }
                
        if (!languageTag.isEmpty()) {
            binding.setValue(literal);
            binding.setLanguageTag(languageTag);
        } else if (!dataTypeUri.isEmpty()) {
            binding.setValue(literal, dataTypeUri);
        } else {
            binding.setValue(literal);
        }
        
        return binding;
    }
    
    QSparqlResultRow parseStatement() {
        QSparqlResultRow resultRow;
        
        skipWhiteSpace();
        if (i >= buffer.size())
            return resultRow;
        
        if (buffer[i] == '_') {
            resultRow.append(parseNamedNode(QLatin1String("s")));
        } else if (buffer[i] == '<') {
            resultRow.append(parseUri(QLatin1String("s")));
        } else {
            parseError(QLatin1String("Expected subject node"));
        }
        
        skipWhiteSpace();
        if (i >= buffer.size())
            return resultRow;

        if (buffer[i] == '<') {
            resultRow.append(parseUri(QLatin1String("p")));
        } else {
            parseError(QLatin1String("Expected predicate node"));
        }
        
        skipWhiteSpace();
        if (i >= buffer.size())
            return resultRow;

        if (buffer[i] == '<') {
            resultRow.append(parseUri(QLatin1String("o")));
        } else if (buffer[i] == '"') {
            resultRow.append(parseLiteral(QLatin1String("o")));
        } else if (buffer[i] == '_') {
            resultRow.append(parseNamedNode(QLatin1String("o")));
        } else {
            parseError(QLatin1String("Expected object node"));
        }
        
        skipWhiteSpace();
        if (i >= buffer.size())
            return resultRow;

        if (i >= buffer.size() || buffer[i] == '.') {
            i++;
        } else {
            parseError(QLatin1String("Expected '.' as statement terminator"));
        }
        
        skipWhiteSpace();
        
        return resultRow;
    }
    
    QList<QSparqlResultRow> parse() {
        skipWhiteSpace();
        
        while (i < buffer.size()) {
            if (buffer[i] == '#') {
                skipComment();
            } else if (buffer[i] == '\n' || buffer[i] == '\r') {
                ; // Blank line
            } else {
                results.append(parseStatement());
            }
            
            skipEoln();
            skipWhiteSpace();
        }
        
        return results;
    }
    
    QByteArray buffer;
    int i;
    QList<QSparqlResultRow> results;
};

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

class EndpointResultPrivate  : public QObject {
    Q_OBJECT
public:
    EndpointResultPrivate(EndpointResult *result, EndpointDriverPrivate *dpp)
    : reply(0), isFinished(false), loop(0), q(result), driverPrivate(dpp) 
    {
    }
    
    ~EndpointResultPrivate()
    {
        delete reply;
    }
    
    QNetworkReply *reply;
    QByteArray buffer;
    QList<QSparqlResultRow> results;
    bool isFinished;
    QEventLoop *loop;
    EndpointResult *q;
    EndpointDriverPrivate *driverPrivate;
    
public Q_SLOTS:
    void readData()
    {
        buffer += reply->readAll();
    }
    
    void handleError(QNetworkReply::NetworkError code);
    void terminate();
    void parseNTriples();
    void parseResults();
};

void EndpointResultPrivate::handleError(QNetworkReply::NetworkError code)
{
    if (code == QNetworkReply::UnknownContentError)
        q->setLastError(QSparqlError(QString::fromLatin1(buffer), QSparqlError::StatementError, code));
    else
        q->setLastError(QSparqlError(reply->errorString(), QSparqlError::ConnectionError, code));

    terminate();
}

void EndpointResultPrivate::terminate()
{
    isFinished = true;
    q->emit finished();
    
    if (loop != 0)
        loop->exit();
}

void EndpointResultPrivate::parseResults()
{ 
    if (isFinished)
        return;
    
    if (q->isGraph()) {
        NTriplesParser parser(buffer);
        results = parser.parse();
        terminate();
        return;
    }
    
    QDomDocument doc(QLatin1String("sparqlresults"));
    if (!doc.setContent(buffer)) {
        terminate();
        return;
    }

    QDomElement sparqlElement = doc.documentElement();

    QDomNode sectionNode = sparqlElement.firstChild();
    while (!sectionNode.isNull()) {
        QDomElement sectionElement = sectionNode.toElement();
        if (!sectionElement.isNull()) {

            if (sectionElement.tagName() == QLatin1String("head")) {
                QDomNode variableNode = sectionElement.firstChild();
                while (!variableNode.isNull()) {
                    QDomElement variableElement = variableNode.toElement();
                    if (!variableElement.isNull()) {
                    }
                    
                    variableNode = variableNode.nextSibling();
                }
            } else if (sectionElement.tagName() == QLatin1String("boolean")) {
                q->setBoolValue(sectionElement.text().toLower() == QLatin1String("true"));
            } else if (sectionElement.tagName() == QLatin1String("results")) {
                QDomNode resultsNode = sectionElement.firstChild();
                while (!resultsNode.isNull()) {
                    QDomElement resultsElement = resultsNode.toElement();
                    if (!resultsElement.isNull()) {
                        QSparqlResultRow resultRow;                        
                        QDomNode resultNode = resultsElement.firstChild();
                        
                        while (!resultNode.isNull()) {
                            QDomElement bindingElement = resultNode.toElement();
                            
                            if (!bindingElement.isNull()) {
                                QSparqlBinding binding;
                                binding.setName(bindingElement.attribute(QLatin1String("name")));
                                QDomNode bindingNode = bindingElement.firstChild();
                                QDomElement valueElement = bindingNode.toElement();
                                
                                if (!valueElement.isNull()) {
                                    if (valueElement.tagName() == QLatin1String("bnode")) {
                                        binding.setBlankNodeIdentifier(valueElement.text());
                                    } else if (valueElement.tagName() == QLatin1String("literal")) {
                                        if (valueElement.hasAttribute(QString::fromLatin1("datatype"))) {
                                            if (valueElement.hasAttribute(QString::fromLatin1("xsi:type"))) {
                                                // TODO: How should we treat xsi:types here?
                                                binding.setValue(valueElement.text(), QUrl(valueElement.attribute(QString::fromLatin1("datatype"))));
                                            } else {
                                                binding.setValue(valueElement.text(), QUrl(valueElement.attribute(QString::fromLatin1("datatype"))));
                                            }
                                        } else if (valueElement.hasAttribute(QString::fromLatin1("xml:lang"))) {
                                            binding.setValue(QVariant(valueElement.text()));
                                        } else {
                                            binding.setValue(QVariant(valueElement.text()));
                                        }
                                    } else if (valueElement.tagName() == QLatin1String("uri")) {
                                        QUrl url(valueElement.text());
                                        binding.setValue(QVariant(url));
                                    } else {
                                        // Error
                                        qWarning() << "Unknown value element: " << valueElement.text();
                                    }
                                    
                                    resultRow.append(binding);
                                }
                            }

                            resultNode = resultNode.nextSibling();
                        }
                        
                        results.append(resultRow);
                    }
                    
                    resultsNode = resultsNode.nextSibling();
                }
            }            
        }
        
        sectionNode = sectionNode.nextSibling();
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
    setPos(QSparql::BeforeFirstRow);
}

bool EndpointResult::fetch(int i)
{
    setPos(i);
    return pos() < d->results.count();
}

bool EndpointResult::fetchNext()
{
    if (pos() == QSparql::AfterLastRow) {
        return false;
    }

    setPos(pos() + 1);
    
    if (d->isFinished && pos() >= d->results.count()) {
        setPos(QSparql::AfterLastRow);
        return false;
    }
        
    return pos() < d->results.count();
}

bool EndpointResult::fetchLast()
{
    setPos(d->results.count() - 1);
    return pos() >= 0;
}

bool EndpointResult::fetchFirst()
{
    if (pos() == 0)
        return true;

    return fetch(0);
}

QVariant EndpointResult::data(int field) const
{
    if (field >= d->results[pos()].count() || field < 0) {
        qWarning() << "EndpointResult::data[" << pos() << "]: column" << field << "out of range";
        return QVariant();
    }

    return d->results[pos()].binding(field).value();
}

// This is just a temporary hack; eventually this should be refactored so that
// the work is done here instead of Result::exec.
EndpointResult* EndpointDriver::exec(const QString& query, QSparqlQuery::StatementType type)
{
    EndpointResult* res = createResult();
    res->exec(query, type);
    return res;
}

bool EndpointResult::exec(const QString& query, QSparqlQuery::StatementType type)
{
    cleanup();
    
    QUrl queryUrl(d->driverPrivate->url);
    queryUrl.addQueryItem(QLatin1String("query"), query);
    setQuery(query);
    setStatementType(type);

    // Virtuoso protocol extension options - timeout and maxrows
    QVariant timeout = d->driverPrivate->options.option(QLatin1String("timeout"));
    if (timeout.isValid()) {
        queryUrl.addQueryItem(QLatin1String("timeout"), timeout.toString());
    }
    
    QVariant maxrows = d->driverPrivate->options.option(QLatin1String("maxrows"));
    if (maxrows.isValid()) {
        queryUrl.addQueryItem(QLatin1String("maxrows"), maxrows.toString());
    }
    
    qDebug() << "Real url to run.... " << queryUrl.toString();

    d->buffer.clear();
    QNetworkRequest request(queryUrl);
    
    if (isGraph())
        // A Virtuoso protocol extension for CONSTRUCT or DESCRIBE queries
        //request.setRawHeader("Accept", "text/rdf+n3");
        // However, DBPedia only returns ntriples if the accept header is 
        // set to 'text/plain' as below
        request.setRawHeader("Accept", "text/plain");
    else
        request.setRawHeader("Accept", "application/sparql-results+xml");
    
    request.setRawHeader("charset", "utf-8");
    
    d->reply = d->driverPrivate->manager->get(request);

    QObject::connect(d->reply, SIGNAL(readyRead()), d, SLOT(readData()));
    QObject::connect(d->reply, SIGNAL(finished()), d, SLOT(parseResults()));
    QObject::connect(d->reply, SIGNAL(error(QNetworkReply::NetworkError)), d, SLOT(handleError(QNetworkReply::NetworkError)));
    
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

QSparqlResultRow EndpointResult::resultRow() const
{
    QSparqlResultRow info;
    if (pos() < 0 || pos() >= d->results.count())
        return info;

    return d->results[pos()];
}

EndpointDriver::EndpointDriver(QObject * parent)
    : QSparqlDriver(parent)
{
    d = new EndpointDriverPrivate();
}

EndpointDriver::~EndpointDriver()
{
    if (d->managerOwned)
        delete d->manager;
    delete d;
}

bool EndpointDriver::hasFeature(QSparqlConnection::Feature f) const
{
/*    switch (f) {
    case QSparqlConnection::Transactions:
        return false;
    case QSparqlConnection::NamedPlaceholders:
    case QSparqlConnection::BatchOperations:
    case QSparqlConnection::SimpleLocking:
    case QSparqlConnection::EventNotifications:
    case QSparqlConnection::FinishQuery:
        return false;
    case QSparqlConnection::QuerySize:
    case QSparqlConnection::BLOB:
    case QSparqlConnection::LastInsertId:
    case QSparqlConnection::Unicode:
    case QSparqlConnection::LowPrecisionNumbers:
        return true;
    case QSparqlConnection::PreparedQueries:
    case QSparqlConnection::PositionalPlaceholders:
        return false;
    case QSparqlConnection::MultipleResultSets:
        return false;
        }*/
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
    return new EndpointResult(d);
}

QT_END_NAMESPACE

#include "qsparql_endpoint.moc"
