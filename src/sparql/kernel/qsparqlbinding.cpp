/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtSparql module of the Qt Toolkit.
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
** Nokia at qt-info@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qsparqlbinding.h"
#include "qatomic.h"
#include "qdebug.h"
#include <QtCore/qurl.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qregexp.h>

QT_BEGIN_NAMESPACE

class QSparqlBindingPrivate
{
public:
    enum NodeType { Invalid, Resource, Literal, Blank };
                         
    QSparqlBindingPrivate(const QString &name,
              QVariant::Type type) :
        ref(1), nm(name), type(type), nodetype(QSparqlBindingPrivate::Invalid)
    {
    }

    QSparqlBindingPrivate(const QSparqlBindingPrivate &other)
        : ref(1),
          nm(other.nm),
          type(other.type),
          datatype(other.datatype),
          lang(other.lang),
          nodetype(other.nodetype)
    {}

    bool operator==(const QSparqlBindingPrivate& other) const
    {
        return (nm == other.nm
                && type == other.type
                && nodetype == other.nodetype
                && datatype == other.datatype
                && lang == other.lang);
    }

    QAtomicInt ref;
    QString nm;
    QVariant::Type type;
    QUrl datatype;
    QString lang;
    NodeType nodetype;
};

/*!
    \class QSparqlBinding
    \brief The QSparqlBinding class manipulates the fields in SQL database tables
    and views.

    \ingroup database
    \ingroup shared

    QSparqlBinding represents the characteristics of a single column in a
    database table or view, such as the data type and column name. A
    field also contains the value of the database column, which can be
    viewed or changed.

    Field data values are stored as QVariants. Using an incompatible
    type is not permitted. For example:

    \snippet doc/src/snippets/sqldatabase/sqldatabase.cpp 2

    However, the field will attempt to cast certain data types to the
    field data type where possible:

    \snippet doc/src/snippets/sqldatabase/sqldatabase.cpp 3

    QSparqlBinding objects are rarely created explicitly in application
    code. They are usually accessed indirectly through \l{QSparqlBindingSet}s
    that already contain a list of bindings. For example:

    \snippet doc/src/snippets/sqldatabase/sqldatabase.cpp 4
    \dots
    \snippet doc/src/snippets/sqldatabase/sqldatabase.cpp 5
    \snippet doc/src/snippets/sqldatabase/sqldatabase.cpp 6

    A QSparqlBinding object can provide some meta-data about the
    binding, for example, its name(), variant type(), languageTag(),
    defaultValue(), typeID(), isGenerated() and isReadOnly(). The
    binding's data can be checked to see if it isNull(), and its
    value() retrieved. When editing the data can be set with
    setValue() or set to NULL with clear().

    \sa QSparqlBindingSet
*/

/*!
    Constructs an empty binding called \a name of variant type \a
    type.

    \sa setLanguageTag()
*/
QSparqlBinding::QSparqlBinding(const QString& name, QVariant::Type type)
{
    d = new QSparqlBindingPrivate(name, type);
}

/*!
    Constructs a binding called \a name with the value \a value.

    \sa setLanguageTag()
*/
QSparqlBinding::QSparqlBinding(const QString& name, const QVariant& value)
{
    d = new QSparqlBindingPrivate(name, value.type());
    val = value;
}

/*!
    Constructs a copy of \a other.
*/

QSparqlBinding::QSparqlBinding(const QSparqlBinding& other)
{
    d = other.d;
    d->ref.ref();
    val = other.val;
}

/*!
    Sets the field equal to \a other.
*/

QSparqlBinding& QSparqlBinding::operator=(const QSparqlBinding& other)
{
    qAtomicAssign(d, other.d);
    val = other.val;
    return *this;
}


/*! \fn bool QSparqlBinding::operator!=(const QSparqlBinding &other) const
    Returns true if the field is unequal to \a other; otherwise returns
    false.
*/

/*!
    Returns true if the field is equal to \a other; otherwise returns
    false.
*/
bool QSparqlBinding::operator==(const QSparqlBinding& other) const
{
    return ((d == other.d || *d == *other.d)
            && val == other.val);
}

/*!
    Destroys the object and frees any allocated resources.
*/

QSparqlBinding::~QSparqlBinding()
{
    if (!d->ref.deref())
        delete d;
}


/*!
    Sets the field's \a data type URI.

    \sa dataTypeUri() setType()
*/
void QSparqlBinding::setDataTypeUri(const QUrl &datatype)
{
    detach();
    d->datatype = datatype;
}

/*!
    Sets the binding's \a languageTag.

    \sa languageTag() setType()
*/
void QSparqlBinding::setLanguageTag(const QString &languageTag)
{
    detach();
    d->lang = languageTag;
}

static int extractTimezone(QString& str)
{
    QRegExp zone(QString::fromLatin1("([-+])(\\d\\d:\\d\\d)"));
    int ix = zone.indexIn(str);
    if (ix != -1) {
        int sign = (zone.cap(1) == QLatin1String("-") ? -1 : 1);
        QTime adjustment = QTime::fromString(zone.cap(2), QString::fromLatin1("hh':'mm"));
        str.remove(ix, 6);
        return ((adjustment.hour() * 3600) + (adjustment.minute() * 60)) * sign;
    }
    
    return 0;
}

// FIXME: document this
void QSparqlBinding::setValue(const QString& value, const QUrl& dataTypeUri)
{
    d->nodetype = QSparqlBindingPrivate::Literal;
    d->datatype = dataTypeUri;
    QByteArray s = dataTypeUri.toString().toLatin1();
    
    if (s == "http://www.w3.org/2001/XMLSchema#int") {
        setValue(value.toInt());
    } else if (s == "http://www.w3.org/2001/XMLSchema#integer") {
        setValue(value.toInt());
    } else if (s == "http://www.w3.org/2001/XMLSchema#nonNegativeInteger") {
        setValue(value.toUInt());
    } else if (s == "http://www.w3.org/2001/XMLSchema#decimal") {
        setValue(value.toDouble());
    } else if (s == "http://www.w3.org/2001/XMLSchema#short") {
        setValue(value.toInt());
    } else if (s == "http://www.w3.org/2001/XMLSchema#long") {
        setValue(value.toLongLong());
    } else if (s == "http://www.w3.org/2001/XMLSchema#boolean") {
        setValue(value.toLower() == QLatin1String("true") || value.toLower() == QLatin1String("yes") || value.toInt() != 0);
    } else if (s == "http://www.w3.org/2001/XMLSchema#double") {
        setValue(value.toDouble());
    } else if (s == "http://www.w3.org/2001/XMLSchema#float") {
        setValue(value.toDouble());
    } else if (s == "http://www.w3.org/2001/XMLSchema#string") {
        setValue(value);
    } else if (s == "http://www.w3.org/2001/XMLSchema#date") {
        setValue(QDate::fromString(value, Qt::ISODate));
    } else if (s == "http://www.w3.org/2001/XMLSchema#time") {
        QString v(value);
        int adjustment = extractTimezone(v);
        setValue(QTime::fromString(v, Qt::ISODate).addSecs(adjustment));
    } else if (s == "http://www.w3.org/2001/XMLSchema#dateTime") {
        QString v(value);
        int adjustment = extractTimezone(v);
        setValue(QDateTime::fromString(v, Qt::ISODate).addSecs(adjustment));
    } else if (s == "http://www.w3.org/2001/XMLSchema#base64Binary") {
        setValue(QByteArray::fromBase64(value.toAscii()));
    } else {
        setValue(value);
    }
}

/*!
    Returns a string representation of the node in a form suitable for 
    using in a SPARQL query.
*/

QString QSparqlBinding::toString() const
{    
    if (d->nodetype == QSparqlBindingPrivate::Resource)
        return QLatin1Char('<') + QString::fromAscii(val.toUrl().toEncoded()) + QLatin1Char('>');
    
    if (d->nodetype == QSparqlBindingPrivate::Blank)
        return QLatin1String("_:") + val.toString();
    
    if (d->nodetype == QSparqlBindingPrivate::Literal) {
        QString literal;
        
        switch (val.type()) {
        case QVariant::Int:
        case QVariant::LongLong:
        case QVariant::UInt:
        case QVariant::ULongLong:
            literal = QLatin1Char('\"') + val.toString() + QLatin1Char('\"');
            break;
        case QVariant::Bool:
            literal = val.toBool() ? QLatin1String("'true'") : QLatin1String("'false'");
            break;
        case QVariant::Double:
            literal = QLatin1Char('\"') + QString::number(val.toDouble(), 'e', 10) + QLatin1Char('\"');
            break;
        case QVariant::String:
        {
            literal.append(QLatin1Char('\"'));
            foreach (const QChar ch, val.toString()) {
                if (ch == QLatin1Char('\t'))
                    literal.append(QLatin1String("\\\t"));
                else if (ch == QLatin1Char('\n'))
                    literal.append(QLatin1String("\\\n"));
                else if (ch == QLatin1Char('\r'))
                    literal.append(QLatin1String("\\\b"));
                else if (ch == QLatin1Char('\f'))
                    literal.append(QLatin1String("\\\f"));
                else if (ch == QLatin1Char('\"'))
                    literal.append(QLatin1String("\\\""));
                else if (ch == QLatin1Char('\''))
                    literal.append(QLatin1String("\\\'"));
                else if (ch == QLatin1Char('\\'))
                    literal.append(QLatin1String("\\\\"));
                else
                    literal.append(ch);
            }
            literal.append(QLatin1Char('\"'));
            break;
        }
        case QVariant::Date:
        case QVariant::Time:
        case QVariant::DateTime:
        {
            QDate dt = val.toDateTime().date();
            QTime tm = val.toDateTime().time();
            // Dateformat has to be "yyyy-MM-ddThh:mm:ss", with leading zeroes if month or day < 10
            literal = QLatin1Char('\"') + QString::number(dt.year()) + QLatin1Char('-') +
                QString::number(dt.month()).rightJustified(2, QLatin1Char('0'), true) +
                QLatin1Char('-') +
                QString::number(dt.day()).rightJustified(2, QLatin1Char('0'), true) +
                QLatin1Char('T') +
                tm.toString() + QLatin1Char('\"');
            break;
        }
        case QVariant::ByteArray:
            literal = QLatin1Char('\"') + QString::fromAscii(val.toByteArray().toBase64()) + QLatin1Char('\"') ;
            break;
        default:
            break;
        }
            
        if (!d->lang.isEmpty())
            literal.append(QLatin1Char('@') + d->lang);
    
        if (!d->datatype.isEmpty())
            literal.append(QLatin1String("^^<") + QString::fromAscii(dataTypeUri().toEncoded()) + QLatin1Char('>'));
        
        return literal;
    }
    
    return QString();
}

/*!
    Sets the value of the binding to \a value..

    If the data type of \a value differs from the field's current
    data type, an attempt is made to cast it to the proper type. This
    preserves the data type of the field in the case of assignment,
    e.g. a QString to an integer data type.

    To set the value to NULL, use clear().

    \sa value() isReadOnly() defaultValue()
*/

void QSparqlBinding::setValue(const QVariant& value)
{
    val = value;
    
    if (value.type() == QVariant::Url)
        d->nodetype = QSparqlBindingPrivate::Resource;
    else
        d->nodetype = QSparqlBindingPrivate::Literal;
}

// FIXME: document this
void QSparqlBinding::setBlankNodeIdentifier(const QString& id)
{
    val = id;
    d->nodetype = QSparqlBindingPrivate::Blank;
}

/*!
    Clears the value of the binding and sets it to NULL.
    If the field is read-only, nothing happens.

    \sa setValue() isReadOnly()
*/

void QSparqlBinding::clear()
{
    val = QVariant(type());
}

/*!
    Sets the name of the field to \a name.

    \sa name()
*/

void QSparqlBinding::setName(const QString& name)
{
    detach();
    d->nm = name;
}

/*!
    \fn QVariant QSparqlBinding::value() const

    Returns the value of the field as a QVariant.

    Use isNull() to check if the field's value is NULL.
*/

/*!
    Returns the name of the field.

    \sa setName()
*/
QString QSparqlBinding::name() const
{
    return d->nm;
}

/*!
    Returns the binding's value as a QVariant type.

    \sa setType()
*/
QVariant::Type QSparqlBinding::type() const
{
    return d->type;
}


QUrl QSparqlBinding::dataTypeUri() const
{
    if (d->nodetype != QSparqlBindingPrivate::Literal)
        return QUrl();

    if (!d->datatype.isEmpty()) {
        return d->datatype;
    }
    
    switch (val.type()) {
    case QVariant::Int:
        return QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#int");
    case QVariant::LongLong:
        return QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#long");
    case QVariant::UInt:
        return QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#unsignedInt");
    case QVariant::ULongLong:
        return QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#unsignedLong");
    case QVariant::Bool:
        return QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#boolean");
    case QVariant::Double:
        return QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#double");
    case QVariant::String:
        return QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#string");
    case QVariant::Date:
        return QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#date");
    case QVariant::Time:
        return QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#time");
    case QVariant::DateTime:
        return QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#dateTime");
    case QVariant::ByteArray:
        return QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#base64Binary");
    default:
        return QUrl();
    }
}

/*!
    Set's the field's variant type to \a type.

    \sa type() setLanguageTag()
*/
void QSparqlBinding::setType(QVariant::Type type)
{
    detach();
    d->type = type;
}



/*!
    Returns true if the value is a resource node.

    \sa setResource()
*/
bool QSparqlBinding::isResource() const
{
    return d->nodetype == QSparqlBindingPrivate::Resource;
}

/*!
    Returns true if the value is a literal node.

    \sa setLiteral()
*/
bool QSparqlBinding::isLiteral() const
{
    return d->nodetype == QSparqlBindingPrivate::Literal;
}

/*!
    Returns true if the value is a blank node.

    \sa setBlank()
*/
bool QSparqlBinding::isBlank() const
{
    return d->nodetype == QSparqlBindingPrivate::Blank;
}

/*! \internal
*/
void QSparqlBinding::detach()
{
    qAtomicDetach(d);
}

/*!
    Returns the field's languageTag.

    \sa setLanguageTag() type()
*/
QString QSparqlBinding::languageTag() const
{
    return d->lang;
}


/*!
    Returns true if the field's variant type is valid; otherwise
    returns false.
*/
bool QSparqlBinding::isValid() const
{
    return d->type != QVariant::Invalid;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QSparqlBinding &f)
{
#ifndef Q_BROKEN_DEBUG_STREAM
    dbg.nospace() << "QSparqlBinding(" << f.name() << ", " << QVariant::typeToName(f.type());
    if (!f.languageTag().isEmpty())
        dbg.nospace() << ", languageTag: " << f.languageTag();
    dbg.nospace() << ')';
    return dbg.space();
#else
    qWarning("This compiler doesn't support streaming QSparqlBinding to QDebug");
    return dbg;
    Q_UNUSED(f);
#endif
}
#endif

QT_END_NAMESPACE
