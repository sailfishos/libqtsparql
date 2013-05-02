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

#include "qsparqlbinding.h"
#include "qatomic.h"
#include "qdebug.h"
#include <QtCore/qurl.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qregexp.h>

#define XSD_ALL
#include "qsparqlxsd_p.h"

QT_BEGIN_NAMESPACE

class QSparqlBindingPrivate
{
public:
    enum NodeType { Invalid, Uri, Literal, Blank };

    QSparqlBindingPrivate(const QString &name) :
        ref(1), nm(name), nodetype(QSparqlBindingPrivate::Invalid)
    {
    }

    QSparqlBindingPrivate(const QSparqlBindingPrivate &other)
        : ref(1),
          nm(other.nm),
          dataType(other.dataType),
          lang(other.lang),
          nodetype(other.nodetype)
    {}

    bool operator==(const QSparqlBindingPrivate& other) const
    {
        return (nodetype == other.nodetype
                && dataType == other.dataType
                && lang == other.lang);
    }

    QAtomicInt ref;
    QString nm;
    QUrl dataType;
    QString lang;
    NodeType nodetype;
};

/*!
    \class QSparqlBinding
    \brief The QSparqlBinding class handles a binding between a SPARQL query variable
    name and the value of the RDF node.

    QSparqlBinding represents the characteristics of a single RDF node in a
    query result, such as the data type and variable name. A
    binding also contains the value of the variable, which can be
    viewed or changed.

    Binding data values are stored as QVariants.

    QSparqlBinding objects are rarely created explicitly in application
    code. They are usually accessed indirectly through QSparqlResultRow that
    already contain a list of bindings.

    A QSparqlBinding object can provide some meta-data about the
    binding, for example, its name(), variant type(), languageTag(),
    and dataTypeUri(). The RDF node type is given with the isUri(),
    isLiteral() and isBlank() methods. The
    binding's data can be checked to see if it isValid(), and its
    value() retrieved, or a string representation toString(). When
    editing the data can be set with setValue() or set to an invalid
    type with clear().

    \sa QSparqlResultRow
*/

/*!
    Constructs an empty binding called \a name of variant type \a
    type.

    \sa setDataTypeUri() setLanguageTag() setBlankNodeLabel()
*/
QSparqlBinding::QSparqlBinding(const QString& name)
{
    d = new QSparqlBindingPrivate(name);
}

/*!
    Constructs a binding called \a name with the value \a value.

    \sa setDataTypeUri() setLanguageTag() setBlankNodeLabel()
*/
QSparqlBinding::QSparqlBinding(const QString& name, const QVariant& value)
{
    d = new QSparqlBindingPrivate(name);
    setValue(value);
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
    Sets the binding equal to \a other.
*/

QSparqlBinding& QSparqlBinding::operator=(const QSparqlBinding& other)
{
    qAtomicAssign(d, other.d);
    val = other.val;
    return *this;
}


/*! \fn bool QSparqlBinding::operator!=(const QSparqlBinding &other) const
    Returns true if the binding is unequal to \a other; otherwise returns
    false.
*/

/*!
    Returns true if the binding is equal to \a other; otherwise returns
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
    Sets the binding's \a data type URI.

    \sa dataTypeUri()
*/
void QSparqlBinding::setDataTypeUri(const QUrl &dataType)
{
    detach();
    d->dataType = dataType;
}

/*!
    Sets the binding's \a languageTag.

    \sa languageTag() setDataTypeUri()
*/
void QSparqlBinding::setLanguageTag(const QString &languageTag)
{
    detach();
    d->lang = languageTag;
}

static int extractTimezone(QString& str)
{
    QRegExp zone(QLatin1String("([-+])(\\d\\d:\\d\\d)"));
    int ix = zone.indexIn(str);
    if (ix != -1) {
        int sign = (zone.cap(1) == QLatin1String("-") ? -1 : 1);
        QTime adjustment = QTime::fromString(zone.cap(2), QString::fromLatin1("hh':'mm"));
        str.remove(ix, 6);
        return ((adjustment.hour() * 3600) + (adjustment.minute() * 60)) * sign;
    }

    return 0;
}

/*!
    Sets the binding's value and the URI of its data type

    \sa dataTypeUri() setDataTypeUri()
*/
void QSparqlBinding::setValue(const QString& value, const QUrl& dataTypeUri)
{
    d->nodetype = QSparqlBindingPrivate::Literal;
    QByteArray s = dataTypeUri.toString().toLatin1();
    bool ok = true;

    if (s == "http://www.w3.org/2001/XMLSchema#int") {
        d->dataType = *XSD::Int();
        setValue(value.toInt(&ok));
    } else if (s == "http://www.w3.org/2001/XMLSchema#integer") {
        d->dataType = *XSD::Integer();
        setValue(value.toLongLong(&ok));
    } else if (s == "http://www.w3.org/2001/XMLSchema#nonNegativeInteger") {
        d->dataType = *XSD::NonNegativeInteger();
        setValue(value.toULongLong(&ok));
    } else if (s == "http://www.w3.org/2001/XMLSchema#unsignedInt") {
        d->dataType = *XSD::UnsignedInt();
        setValue(value.toUInt(&ok));
    } else if (s == "http://www.w3.org/2001/XMLSchema#decimal") {
        d->dataType = *XSD::Decimal();
        setValue(value.toDouble(&ok));
    } else if (s == "http://www.w3.org/2001/XMLSchema#short") {
        d->dataType = *XSD::Short();
        setValue(value.toInt(&ok));
    } else if (s == "http://www.w3.org/2001/XMLSchema#long") {
        d->dataType = *XSD::Long();
        setValue(value.toLongLong(&ok));
    } else if (s == "http://www.w3.org/2001/XMLSchema#unsignedLong") {
        d->dataType = *XSD::UnsignedLong();
        setValue(value.toULongLong(&ok));
    } else if (s == "http://www.w3.org/2001/XMLSchema#boolean") {
        d->dataType = *XSD::Boolean();
        setValue(value.toLower() == QLatin1String("true") || value.toLower() == QLatin1String("yes") || value.toInt() != 0);
    } else if (s == "http://www.w3.org/2001/XMLSchema#double") {
        d->dataType = *XSD::Double();
        setValue(value.toDouble(&ok));
    } else if (s == "http://www.w3.org/2001/XMLSchema#float") {
        d->dataType = *XSD::Float();
        setValue(value.toDouble(&ok));
    } else if (s == "http://www.w3.org/2001/XMLSchema#string") {
        d->dataType = *XSD::String();
        setValue(value);
    } else if (s == "http://www.w3.org/2001/XMLSchema#date") {
        d->dataType = *XSD::Date();
        // xsd:dates can have timezones which aren't supported by QDate,
        // so convert to UTC time and use the derived date
        QString v(value);
        int adjustment = extractTimezone(v);
        QDateTime dt = QDateTime::fromString(v, Qt::ISODate);
        dt = dt.addSecs(adjustment);
        setValue(dt.date());
    } else if (s == "http://www.w3.org/2001/XMLSchema#time") {
        d->dataType = *XSD::Time();
        // xsd:times can have timezones which aren't supported by QTime,
        // so convert to UTC time and use that
        QString v(value);
        int adjustment = extractTimezone(v);
        setValue(QTime::fromString(v, Qt::ISODate).addSecs(adjustment));
    } else if (s == "http://www.w3.org/2001/XMLSchema#dateTime") {
        d->dataType = *XSD::DateTime();
        setValue(QDateTime::fromString(value, Qt::ISODate));
    } else if (s == "http://www.w3.org/2001/XMLSchema#base64Binary") {
        d->dataType = *XSD::Base64Binary();
        setValue(QByteArray::fromBase64(value.toLatin1()));
    } else {
        d->dataType = dataTypeUri;
        setValue(value);
    }

    if (!ok)
        qWarning() << "QSparqlBinding::setValue(): Conversion error:" << value << "type:" << d->dataType.toString();
}

/*!
    Returns a string representation of the node in a form suitable for
    using in a SPARQL query.
*/

QString QSparqlBinding::toString() const
{
    if (d->nodetype == QSparqlBindingPrivate::Uri)
        return QLatin1Char('<') + QString::fromLatin1(val.toUrl().toEncoded()) + QLatin1Char('>');

    if (d->nodetype == QSparqlBindingPrivate::Blank)
        return QLatin1String("_:") + val.toString();

    if (d->nodetype == QSparqlBindingPrivate::Literal) {
        QString literal;

        bool quoted = false;
        switch (val.type()) {
        case QVariant::Int:
        case QVariant::LongLong:
        case QVariant::UInt:
        case QVariant::ULongLong:
            literal = val.toString();
            break;
        case QVariant::Bool:
            literal = val.toBool() ? QLatin1String("true") : QLatin1String("false");
            break;
        case QVariant::Double:
            if (dataTypeUri() == QUrl::fromEncoded("http://www.w3.org/2001/XMLSchema#decimal")) {
                literal = QString::number(val.toDouble(), 'f', 10);
                literal.replace(QRegExp(QString::fromLatin1("0+$")), QString::fromLatin1(""));
            } else {
                literal = QString::number(val.toDouble(), 'e', 10);
            }
            break;
        case QVariant::String:
        {
            quoted = true;
            literal.append(QLatin1Char('\"'));
            Q_FOREACH (const QChar ch, val.toString()) {
                if (ch == QLatin1Char('\t'))
                    literal.append(QLatin1String("\\t"));
                else if (ch == QLatin1Char('\n'))
                    literal.append(QLatin1String("\\n"));
                else if (ch == QLatin1Char('\r'))
                    literal.append(QLatin1String("\\r"));
                else if (ch == QLatin1Char('\b'))
                    literal.append(QLatin1String("\\b"));
                else if (ch == QLatin1Char('\f'))
                    literal.append(QLatin1String("\\f"));
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
        {
            quoted = true;
            QDate dt = val.toDate();
            // Date format has to be "yyyy-MM-dd", with leading zeroes if month or day < 10
            literal = QLatin1Char('\"') + QString::number(dt.year()) + QLatin1Char('-') +
                QString::number(dt.month()).rightJustified(2, QLatin1Char('0'), true) +
                QLatin1Char('-') +
                QString::number(dt.day()).rightJustified(2, QLatin1Char('0'), true) + QLatin1Char('\"');
            break;
        }
        case QVariant::Time:
        {
            quoted = true;
            QTime tm = val.toTime();
            // Time format has to be "hh:mm:ss"
            literal = QLatin1Char('\"') + tm.toString() + QLatin1Char('\"');
            break;
        }
        case QVariant::DateTime:
        {
            quoted = true;
            QDateTime dt = val.toDateTime();
            int offset = dt.utcOffset();
            QDate date = val.toDateTime().date();
            QTime time = val.toDateTime().time();
            // DateTime format has to be "yyyy-MM-ddThh:mm:ss", with leading zeroes if month or day < 10
            literal = QLatin1Char('\"') + QString::number(date.year()) + QLatin1Char('-') +
                QString::number(date.month()).rightJustified(2, QLatin1Char('0'), true) +
                QLatin1Char('-') +
                QString::number(date.day()).rightJustified(2, QLatin1Char('0'), true) +
                QLatin1Char('T') +
                time.toString();

            if (offset != 0) {
                QTime zone(0, 0, 0);
                zone.addSecs(offset);
                if (offset > 0)
                    literal.append(zone.toString(QLatin1String("+HH:mm")));
                else
                    literal.append(zone.toString(QLatin1String("-HH:mm")));
            }

            literal.append(QLatin1Char('\"'));
            break;
        }
        case QVariant::ByteArray:
            quoted = true;
            literal = QLatin1Char('\"') + QString::fromLatin1(val.toByteArray().toBase64()) + QLatin1Char('\"');
            break;
        default:
            break;
        }

        if (!d->lang.isEmpty())
            literal.append(QLatin1Char('@') + d->lang);

        if (!d->dataType.isEmpty()) {
            if (!quoted) {
                literal.prepend(QLatin1String("\""));
                literal.append(QLatin1String("\""));
            }
            literal.append(QLatin1String("^^<") + QString::fromLatin1(dataTypeUri().toEncoded()) + QLatin1Char('>'));
        }
        return literal;
    }

    return QString();
}

/*!
    Sets the value of the binding to \a value..

    If the data type of \a value differs from the binding's current
    data type, an attempt is made to cast it to the proper type. This
    preserves the data type of the field in the case of assignment,
    e.g. a QString to an integer data type.

    To set the value to isInvalid(), use clear().

    \sa value() isReadOnly() defaultValue()
*/

void QSparqlBinding::setValue(const QVariant& value)
{
    val = value;

    if (value.type() == QVariant::Url)
        d->nodetype = QSparqlBindingPrivate::Uri;
    else
        d->nodetype = QSparqlBindingPrivate::Literal;
}

/*!
    Sets the label name and RDF type of a blank node,
    and isBlank() will return true.

    \sa isBlank() toString()
*/

void QSparqlBinding::setBlankNodeLabel(const QString& id)
{
    detach();
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
    val = QVariant();
    d->nodetype = QSparqlBindingPrivate::Invalid;
    d->dataType = QUrl();
    d->lang = QString();
}

/*!
    Sets the name of the binding variable to \a name.

    \sa name()
*/

void QSparqlBinding::setName(const QString& name)
{
    detach();
    d->nm = name;
}

/*!
    \fn QVariant QSparqlBinding::value() const

    Returns the value of the binding as a QVariant.

    Use isValid() to check if the binding's value has been set.

    \sa setValue()
*/

/*!
    Returns the name of the binding's variable name.

    \sa setName()
*/
QString QSparqlBinding::name() const
{
    return d->nm;
}

/*!
    If the binding is a literal, returns the data type Uri of the RDF type

    \sa setDataTypeUri()
*/
QUrl QSparqlBinding::dataTypeUri() const
{
    if (d->nodetype != QSparqlBindingPrivate::Literal)
        return QUrl();

    if (!d->dataType.isEmpty()) {
        return d->dataType;
    }

    switch (val.type()) {
    case QVariant::Int:
        return *XSD::Int();
    case QVariant::LongLong:
        return *XSD::Long();
    case QVariant::UInt:
        return *XSD::UnsignedInt();
    case QVariant::ULongLong:
        return *XSD::UnsignedLong();
    case QVariant::Bool:
        return *XSD::Boolean();
    case QVariant::Double:
        return *XSD::Double();
    case QVariant::String:
        return *XSD::String();
    case QVariant::Date:
        return *XSD::Date();
    case QVariant::Time:
        return *XSD::Time();
    case QVariant::DateTime:
        return *XSD::DateTime();
    case QVariant::ByteArray:
        return *XSD::Base64Binary();
    default:
        return QUrl();
    }
}


/*!
    Returns true if the value is a Uri representing an RDF resource node.

    \sa setValue()
*/
bool QSparqlBinding::isUri() const
{
    return d->nodetype == QSparqlBindingPrivate::Uri;
}

/*!
    Returns true if the value is a literal node.

    \sa setValue()
*/
bool QSparqlBinding::isLiteral() const
{
    return d->nodetype == QSparqlBindingPrivate::Literal;
}

/*!
    Returns true if the value is a blank node.

    \sa setBlankNodeLabel()
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
    Returns the binding's languageTag.

    \sa setLanguageTag() dataTypeUri()
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
    return val.type() != QVariant::Invalid;
}

#ifndef QT_NO_DEBUG_STREAM
// LCOV_EXCL_START
QDebug operator<<(QDebug dbg, const QSparqlBinding &f)
{
#ifndef Q_BROKEN_DEBUG_STREAM
    dbg.nospace() << "QSparqlBinding(" << f.name() << ", " << f.toString();
    dbg.nospace() << ')';
    return dbg.space();
#else
    qWarning("This compiler doesn't support streaming QSparqlBinding to QDebug");
    return dbg;
    Q_UNUSED(f);
#endif
}
// LCOV_EXCL_STOP
#endif

QT_END_NAMESPACE
