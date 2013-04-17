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

#include "qsparqlntriples_p.h"

#include <QtCore/qdebug.h>
#include <QtCore/qurl.h>

/*  From the raptor ntriples parser                        
    These are for 7-bit ASCII and not locale-specific 
 */
#define IS_ASCII_ALPHA(c) (((c)>0x40 && (c)<0x5B) || ((c)>0x60 && (c)<0x7B))
#define IS_ASCII_UPPER(c) ((c)>0x40 && (c)<0x5B)
#define IS_ASCII_DIGIT(c) ((c)>0x2F && (c)<0x3A)
#define IS_ASCII_PRINT(c) ((c)>0x1F && (c)<0x7F)
#define TO_ASCII_LOWER(c) ((c)+0x20)
                        

QT_BEGIN_NAMESPACE

void QSparqlNTriples::parseError(QString message) 
{
    QString context;
    
    while (i < buffer.size()) {
        context.append(QLatin1Char(buffer[i]));
        if (buffer[i] == '\n' || buffer[i] == '\r')
            break;
            
        i++;
    }
    
    qWarning() << "ERROR in line " << lineNumber << ":" << message << ": '" << context << "'";        
}

void QSparqlNTriples::skipWhiteSpace() 
{
    while (i < buffer.size()) {
        if (buffer[i] != ' ' && buffer[i] != '\t')
            break;
            
        i++;
    }
}

void QSparqlNTriples::skipComment() 
{
    while (i < buffer.size()) {
        if (buffer[i] == '\n' || buffer[i] == '\r')
            break;
            
        i++;
    }
}

void QSparqlNTriples::skipEoln() 
{
    if (buffer[i] == '\n') {
        i++;
    } else if (buffer[i] == '\r') {
        i++;
        if (i < buffer.size() && buffer[i] == '\n') {
            i++;
        }
    }
    
    lineNumber++;
}

QUrl QSparqlNTriples::parseUri() 
{
    QByteArray uri;
    bool isUtf8 = false;
    
    if (i < buffer.size() && buffer[i] == '<') {
        i++;
        while (i < buffer.size()) {
            if (buffer[i] == '>') {
                i++;
                break;
            }
            
            if (static_cast<uchar>(buffer.at(i)) > 0x7f)
                isUtf8 = true;
            
            uri.append(buffer[i]);
            i++;
        }
    }
    
    if (isUtf8)
        return QUrl(QString::fromUtf8(uri));
    else
        return QUrl::fromEncoded(uri);
}

QSparqlBinding QSparqlNTriples::parseNamedNode(QString name) 
{
    QString nodeName;
    QSparqlBinding binding(name);
    
    i++;
    if (i >= buffer.size() || buffer[i] != ':') {
        parseError(QLatin1String("Expected named node '_:xxxx'"));
    }
    
    i++;
    if (i < buffer.size() && IS_ASCII_ALPHA((uchar) buffer[i])) {
        while (i < buffer.size()) {
            if (!IS_ASCII_ALPHA((uchar) buffer[i]) && !IS_ASCII_DIGIT((uchar) buffer[i]))
                break;
            
            nodeName.append(QLatin1Char(buffer[i]));
            i++;
        }
    }
    
    binding.setBlankNodeLabel(nodeName);
    return binding;
}

QString QSparqlNTriples::parseLanguageTag() 
{
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

QSparqlBinding QSparqlNTriples::parseLiteral(QString name) 
{
    QByteArray literal;
    QString languageTag;
    QUrl dataTypeUri;
    QSparqlBinding binding(name);
    bool isUtf8 = false;
    
    if (buffer[i] == '"') {
        i++;
        while (i < buffer.size()) {
            if (buffer[i] == '"') {
                i++;
                if (i < buffer.size() && buffer[i] == '^') {
                    i++;
                    if (i < buffer.size() && buffer[i] == '^') {
                        i++;
                        dataTypeUri = parseUri();
                    }
                } else if (i < buffer.size() && buffer[i] == '@') {
                    languageTag = parseLanguageTag();
                }
                
                break;
            }
            
            if (static_cast<uchar>(buffer.at(i)) > 0x7f)
                isUtf8 = true;

            if (buffer[i] != '\\') {
                literal.append(buffer[i]);
            } else {
                i++;
                if (i < buffer.size()) {
                    if (buffer[i] == '"' || buffer[i] == 'n' || buffer[i] == 'r' || buffer[i] == 't' || buffer[i] == '\\') {
                        literal.append(buffer[i - 1]);
                        literal.append(buffer[i]);
                    } else if (buffer[i] == 'u') {
                        // Unicode escape \uxxxx
                        isUtf8 = true;
                        QString str = QString::fromLatin1(buffer.mid(i + 1, 4));
                        bool ok = false;
                        ushort unicode = str.toUShort(&ok, 16);
                        if (ok) {
                            literal.append(QString::fromUtf16(&unicode, 1).toUtf8());
                            i += 4;
                        } else {
                            parseError(QLatin1String("Invalid unicode escape sequence"));
                        }
                    } else if (buffer[i] == 'U') {
                        // Unicode escape \Uxxxxxxxx
                        isUtf8 = true;
                        QString str = QString::fromLatin1(buffer.mid(i + 1, 8));
                        bool ok = false;
                        uint unicode = str.toUInt(&ok, 16);
                        if (ok) {
                            literal.append(QString::fromUcs4(&unicode, 1).toUtf8());
                            i += 8;
                        } else {
                            parseError(QLatin1String("Invalid unicode escape sequence"));
                        }
                    } else {
                        parseError(QLatin1String("Invalid literal escape sequence"));
                    }
                }
            }
            
            i++;
        }
    }
            
    QString value;
    if (isUtf8) 
        value = QString::fromUtf8(literal);
    else
        value = QString::fromLatin1(literal);
        
    if (!languageTag.isEmpty()) {
        binding.setValue(value);
        binding.setLanguageTag(languageTag);
    } else if (!dataTypeUri.isEmpty()) {
        binding.setValue(value, dataTypeUri);
    } else {
        binding.setValue(value);
    }
    
    return binding;
}

QSparqlResultRow QSparqlNTriples::parseStatement() 
{
    QSparqlResultRow resultRow;
    
    skipWhiteSpace();
    if (i >= buffer.size())
        return resultRow;
    
    if (buffer[i] == '_') {
        resultRow.append(parseNamedNode(QLatin1String("s")));
    } else if (buffer[i] == '<') {
        resultRow.append(QSparqlBinding(QLatin1String("s"), parseUri()));
    } else {
        parseError(QLatin1String("Expected subject node"));
    }
    
    skipWhiteSpace();
    if (i >= buffer.size())
        return resultRow;

    if (buffer[i] == '<') {
        resultRow.append(QSparqlBinding(QLatin1String("p"), parseUri()));
    } else {
        parseError(QLatin1String("Expected predicate node"));
    }
    
    skipWhiteSpace();
    if (i >= buffer.size())
        return resultRow;

    if (buffer[i] == '<') {
        resultRow.append(QSparqlBinding(QLatin1String("o"), parseUri()));
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

QVector<QSparqlResultRow> QSparqlNTriples::parse()
{
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

QT_END_NAMESPACE
