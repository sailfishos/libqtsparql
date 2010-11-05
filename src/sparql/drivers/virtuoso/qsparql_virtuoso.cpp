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

#include "qsparql_virtuoso_p.h"
#include "sparqlext_p.h"

#if defined (Q_OS_WIN32)
#include <qt_windows.h>
#endif

#include <QtCore/qcoreapplication.h>
#include <QtCore/qvariant.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qvarlengtharray.h>
#include <QtCore/qvector.h>
#include <QtCore/qurl.h>

#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QThread>

#include <QtSparql/qsparqlerror.h>
#include <QtSparql/qsparqlbinding.h>
#include <QtSparql/qsparqlresultrow.h>
#include <QtSparql/qsparqlquery.h>
#include <QtSparql/private/qsparqlntriples_p.h>

#include <QDebug>

QT_BEGIN_NAMESPACE

// newer platform SDKs use SQLLEN instead of SQLINTEGER
#if defined(WIN32) && (_MSC_VER < 1300)
# define QSQLLEN SQLINTEGER
# define QSQLULEN SQLUINTEGER
#else
# define QSQLLEN SQLLEN
# define QSQLULEN SQLULEN
#endif

namespace XSD {
Q_GLOBAL_STATIC_WITH_ARGS(QUrl, Decimal,
                          (QLatin1String("http://www.w3.org/2001/XMLSchema#decimal")))

Q_GLOBAL_STATIC_WITH_ARGS(QUrl, Date,
                          (QLatin1String("http://www.w3.org/2001/XMLSchema#date")))

Q_GLOBAL_STATIC_WITH_ARGS(QUrl, Time,
                          (QLatin1String("http://www.w3.org/2001/XMLSchema#time")))

Q_GLOBAL_STATIC_WITH_ARGS(QUrl, DateTime,
                          (QLatin1String("http://www.w3.org/2001/XMLSchema#dateTime")))
}

static const int COLNAMESIZE = 256;

class QVirtuosoFetcherPrivate : public QThread
{
public:
    QVirtuosoFetcherPrivate(QVirtuosoResult *res) : result(res) { }

    void run()
    {
        setTerminationEnabled(false);
        if (result->exec()) {
            if (result->isTable()) {
                while (result->fetchNextResult()) {
                    setTerminationEnabled(true);
                    setTerminationEnabled(false);
                }
            } else if (result->isBool()) {
                result->fetchBoolResult();
            } else if (result->isGraph()) {
                result->fetchGraphResult();
            } else {
                result->terminate();
            }
        }
        setTerminationEnabled(true);
    }

private:
    QVirtuosoResult *result;
};

class QVirtuosoDriverPrivate
{
public:
    QVirtuosoDriverPrivate()
    : hEnv(0), hDbc(0), disconnectCount(0), mutex(QMutex::Recursive)
    {
    }

    SQLHANDLE hEnv;
    SQLHANDLE hDbc;

    int disconnectCount;
    // This mutex is for ensuring that only one thread at a time
    // is using the connection to make odbc queries
    QMutex mutex;
};

class QVirtuosoResultPrivate
{
public:
    QVirtuosoResultPrivate(const QVirtuosoDriver* d, QVirtuosoDriverPrivate *dpp, QVirtuosoFetcherPrivate *f) :
        driver(d), hstmt(0), numResultCols(0), hdesc(0),
        resultColIdx(0), driverPrivate(dpp), fetcher(f),
        mutex(QMutex::Recursive), isFinished(false), loop(0)
    {
    }

    ~QVirtuosoResultPrivate()
    {
        if (fetcher->isRunning()) {
            fetcher->terminate();
            if (!fetcher->wait(500))
                return;
        }
        delete fetcher;
    }

    inline void clearValues()
    {
        QSparqlResultRow resultRow;
        results.append(resultRow);
        resultColIdx = 0;
    }

    SQLHANDLE dpEnv() const { return driverPrivate ? driverPrivate->hEnv : 0;}
    SQLHANDLE dpDbc() const { return driverPrivate ? driverPrivate->hDbc : 0;}

    const QVirtuosoDriver* driver;
    SQLHANDLE hstmt;
    SQLSMALLINT numResultCols;
    SQLHDESC hdesc;

    QByteArray query;
    QVector<QSparqlResultRow> results;
    QStringList bindingNames;
    int resultColIdx;
    int disconnectCount;
    QVirtuosoDriverPrivate *driverPrivate;
    QVirtuosoFetcherPrivate *fetcher;
    // This mutex is for ensuring that only one thread at a time
    // is accessing the results array
    QMutex mutex;
    bool isFinished;
    QEventLoop *loop;

    bool isStmtHandleValid(const QSparqlDriver *driver);
    void updateStmtHandleState(const QSparqlDriver *driver);
};

bool QVirtuosoResultPrivate::isStmtHandleValid(const QSparqlDriver *driver)
{
    const QVirtuosoDriver *odbcdriver = static_cast<const QVirtuosoDriver*> (driver);
    return disconnectCount == odbcdriver->d->disconnectCount;
}

void QVirtuosoResultPrivate::updateStmtHandleState(const QSparqlDriver *driver)
{
    const QVirtuosoDriver *odbcdriver = static_cast<const QVirtuosoDriver*> (driver);
    disconnectCount = odbcdriver->d->disconnectCount;
}

static QString qWarnODBCHandle(int handleType, SQLHANDLE handle, int *nativeCode = 0)
{
    SQLINTEGER nativeCode_ = 0;
    SQLSMALLINT msgLen = 0;
    SQLRETURN r = SQL_NO_DATA;
    SQLTCHAR state_[SQL_SQLSTATE_SIZE+1];
    QVarLengthArray<SQLTCHAR> description_(SQL_MAX_MESSAGE_LENGTH);
    QString result;
    int i = 1;

    description_[0] = 0;
    SQLCHAR dummyBuffer[1]; // dummy buffer only used to determine length
    r = SQLGetDiagRec(handleType, handle, i, state_, &nativeCode_, dummyBuffer, NULL, &msgLen);
    if (r == SQL_NO_DATA)
        return QString();
    description_.resize(msgLen+1);
    do {
        r = SQLGetDiagRec(handleType, handle, i, state_, &nativeCode_, description_.data(), description_.size(), &msgLen);
        if (r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) {
            if (nativeCode)
                *nativeCode = nativeCode_;
            QString tmpstore;
            tmpstore = QString::fromLatin1((const char *) description_.data(), msgLen);
            if (result != tmpstore) {
                if (!result.isEmpty())
                    result += QLatin1Char(' ');
                result += tmpstore;
            }
        } else if (r == SQL_ERROR || r == SQL_INVALID_HANDLE) {
            return result;
        }
        ++i;
    } while (r != SQL_NO_DATA);
    return result;
}

static QString qVirtuosoWarn(const QVirtuosoResultPrivate* odbc, int *nativeCode = 0)
{
    return (qWarnODBCHandle(SQL_HANDLE_ENV, odbc->dpEnv()) + QLatin1Char(' ')
             + qWarnODBCHandle(SQL_HANDLE_DBC, odbc->dpDbc()) + QLatin1Char(' ')
             + qWarnODBCHandle(SQL_HANDLE_STMT, odbc->hstmt, nativeCode));
}

static QString qVirtuosoWarn(const QVirtuosoDriverPrivate* odbc, int *nativeCode = 0)
{
    return (qWarnODBCHandle(SQL_HANDLE_ENV, odbc->hEnv) + QLatin1Char(' ')
             + qWarnODBCHandle(SQL_HANDLE_DBC, odbc->hDbc, nativeCode));
}

static void qSparqlWarning(const QString& message, const QVirtuosoResultPrivate* odbc)
{
    qWarning() << message << "\tError:" << qVirtuosoWarn(odbc);
}

static void qSparqlWarning(const QString &message, const QVirtuosoDriverPrivate *odbc)
{
    qWarning() << message << "\tError:" << qVirtuosoWarn(odbc);
}

static QSparqlError qMakeError(const QString& err, QSparqlError::ErrorType type, const QVirtuosoResultPrivate* p)
{
    int nativeCode = -1;
    QString message = qVirtuosoWarn(p, &nativeCode);
    return QSparqlError(QString::fromLatin1("Virtuoso: %1 %2").arg(err).arg(message), type, nativeCode);
}

static QSparqlError qMakeError(const QString& err, QSparqlError::ErrorType type,
                            const QVirtuosoDriverPrivate* p)
{
    int nativeCode = -1;
    QString message = qVirtuosoWarn(p, &nativeCode);
    return QSparqlError(QLatin1String("QVirtuoso: ") + err + QLatin1String(" ") + qVirtuosoWarn(p), type, nativeCode);
}


////////////////////////////////////////////////////////////////////////////

QVirtuosoResult::QVirtuosoResult(const QVirtuosoDriver * db, QVirtuosoDriverPrivate* p)
: QSparqlResult()
{
    d = new QVirtuosoResultPrivate(db, p, new QVirtuosoFetcherPrivate(this));
    d->disconnectCount = p->disconnectCount;
}

QVirtuosoResult::~QVirtuosoResult()
{
    QMutexLocker connectionLocker(&(d->driverPrivate->mutex));

    if (d->hstmt && d->isStmtHandleValid(d->driver) && d->driver->isOpen()) {
        SQLRETURN r = SQLFreeHandle(SQL_HANDLE_STMT, d->hstmt);
        if (r != SQL_SUCCESS)
            qSparqlWarning(QLatin1String("QVirtuosoDriver: Unable to free statement handle ")
                         + QString::number(r), d);
    }

    delete d;
}

// This is just a temporary hack; eventually this should be refactored so that
// the work is done here instead of Result::exec.
QVirtuosoResult* QVirtuosoDriver::exec(const QString& query, QSparqlQuery::StatementType type)
{
    QVirtuosoResult* res = createResult();
    res->exec(query, type, prefixes());
    return res;
}

void QVirtuosoResult::exec(const QString& sparqlQuery, QSparqlQuery::StatementType type, const QString& prefixes)
{
    setQuery(sparqlQuery);
    setStatementType(type);

    d->query = "SPARQL ";

    // Note that NTriples output support depends on a patch from Ivan Mikhailov
    // of OpenLink software. It should be in the next release (ie after 6.1.2)
    if (isGraph())
        d->query.append("define output:format \"NT\" ");

    d->query.append(prefixes.toUtf8());
    d->query.append(sparqlQuery.toUtf8());

    setPos(QSparql::BeforeFirstRow);
    d->bindingNames.clear();
    d->fetcher->start();
}

bool QVirtuosoResult::exec()
{
    QMutexLocker connectionLocker(&(d->driverPrivate->mutex));

    // Always reallocate the statement handle - the statement attributes
    // are not reset if SQLFreeStmt() is called which causes some problems.
    SQLRETURN r;
    if (d->hstmt && d->isStmtHandleValid(d->driver)) {
        r = SQLFreeHandle(SQL_HANDLE_STMT, d->hstmt);
        if (r != SQL_SUCCESS) {
            qSparqlWarning(QLatin1String("QVirtuosoResult::exec: Unable to free statement handle"), d);
            terminate();
            return false;
        }
    }

    r  = SQLAllocHandle(SQL_HANDLE_STMT, d->dpDbc(), &d->hstmt);
    if (r != SQL_SUCCESS) {
        qSparqlWarning(QLatin1String("QVirtuosoResult::exec: Unable to allocate statement handle"), d);
        terminate();
        return false;
    }

    d->updateStmtHandleState(d->driver);

    r = SQLExecDirect(d->hstmt, (UCHAR*) d->query.data(), d->query.length());
    if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO && r!= SQL_NO_DATA) {
        setLastError(qMakeError(QCoreApplication::translate("QVirtuosoResult", "Unable to execute statement"),
                                QSparqlError::StatementError,
                                d));
        terminate();
        return false;
    }

    if (r == SQL_NO_DATA) {
        terminate();
        return true;
    }

    SQLINTEGER isScrollable, bufferLength;
    r = SQLGetStmtAttr(d->hstmt, SQL_ATTR_CURSOR_SCROLLABLE, &isScrollable, SQL_IS_INTEGER, &bufferLength);
    r = SQLGetStmtAttr(d->hstmt, SQL_ATTR_IMP_ROW_DESC, &(d->hdesc), SQL_IS_POINTER, 0);

    SQLNumResultCols(d->hstmt, &(d->numResultCols));
    if (d->numResultCols > 0) {
        for (int i = 0; i < d->numResultCols; ++i) {
            SQLSMALLINT colNameLen;
            SQLTCHAR colName[COLNAMESIZE];
            r = SQLDescribeCol(d->hstmt, i+1, colName, (SQLSMALLINT)COLNAMESIZE, &colNameLen, 0, 0, 0, 0);

            if (r != SQL_SUCCESS) {
                qSparqlWarning(QString::fromLatin1("qMakeField: Unable to describe column %1").arg(i), d);
                terminate();
                return false;
            }

            d->bindingNames.append(QString::fromLatin1((const char*) colName));
        }
    }

    return true;
}

void QVirtuosoResult::waitForFinished()
{
    if (d->isFinished)
        return;

    QEventLoop loop;
    d->loop = &loop;
    loop.exec();
    d->loop = 0;
}

bool QVirtuosoResult::isFinished() const
{
    return d->isFinished;
}

void QVirtuosoResult::terminate()
{
    d->isFinished = true;
    emit finished();
    if (d->loop != 0)
        d->loop->exit();
}

static QSparqlBinding qMakeBinding(const QVirtuosoResultPrivate* p, int colNum)
{
    QSparqlBinding b;
    int r;
    SQLLEN length = 0;
    int bufferLength = 0;
    SQLCHAR dummyBuffer[1]; // dummy buffer only used to determine length
    r = SQLGetData(p->hstmt, colNum, SQL_C_CHAR, dummyBuffer, 0, &length);
    if ((r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) && length > 0)
        bufferLength = length / sizeof(SQLTCHAR) + 1;

    QVarLengthArray<char> buffer(bufferLength);  // The real buffer
    r = SQLGetData(p->hstmt, colNum, SQL_C_CHAR, buffer.data(), buffer.size(), 0);

    int dvtype = 0;
    r = SQLGetDescField(p->hdesc, colNum, SQL_DESC_COL_DV_TYPE, &dvtype, SQL_IS_INTEGER, 0);

    switch (dvtype) {
    case VIRTUOSO_DV_TIMESTAMP:
    case VIRTUOSO_DV_TIMESTAMP_OBJ:
    case VIRTUOSO_DV_DATE:
    case VIRTUOSO_DV_TIME:
    case VIRTUOSO_DV_DATETIME:
        {
            SQLINTEGER dv_dt_type = 0;
            SQLGetDescField(p->hdesc, colNum, SQL_DESC_COL_DT_DT_TYPE, &dv_dt_type, SQL_IS_INTEGER, NULL);
            switch (dv_dt_type) {
            case VIRTUOSO_DT_TYPE_DATETIME:
                b.setValue(QString::fromUtf8(buffer.constData()), *XSD::DateTime());
                break;
            case VIRTUOSO_DT_TYPE_DATE:
                b.setValue(QString::fromUtf8(buffer.constData()), *XSD::Date());
                break;
            case VIRTUOSO_DT_TYPE_TIME:
                b.setValue(QString::fromUtf8(buffer.constData()), *XSD::Time());
                break;
            default:
                break;
            }
            break;
        }
    case VIRTUOSO_DV_DOUBLE_FLOAT:
        b.setValue(QString::fromUtf8(buffer.constData()).toDouble());
        break;
    case VIRTUOSO_DV_IRI_ID:
        break;
    case VIRTUOSO_DV_LONG_INT:
        b.setValue(QString::fromUtf8(buffer.constData()).toInt());
        break;
    case VIRTUOSO_DV_NUMERIC:
        b.setValue(QString::fromUtf8(buffer.constData()).toDouble());
        b.setDataTypeUri(*XSD::Decimal());
        break;
    case VIRTUOSO_DV_RDF:
        {
            SQLCHAR langBuf[100];
            SQLCHAR typeBuf[100];
            SQLINTEGER langBufLen = 0;
            SQLINTEGER typeBufLen = 0;
            SQLGetDescField(p->hdesc, colNum, SQL_DESC_COL_LITERAL_LANG, langBuf, sizeof(langBuf), &langBufLen);
            SQLGetDescField(p->hdesc, colNum, SQL_DESC_COL_LITERAL_TYPE, typeBuf, sizeof(typeBuf), &typeBufLen);
            b.setValue(QString::fromUtf8(buffer.constData()),
                       QUrl::fromEncoded(QByteArray::fromRawData(reinterpret_cast<const char*>(typeBuf), typeBufLen)));

            if (langBufLen > 0)
                b.setLanguageTag(QString::fromLatin1(reinterpret_cast<const char*>(langBuf), langBufLen));
            break;
        }
    case VIRTUOSO_DV_SINGLE_FLOAT:
        b.setValue(QVariant(QString::fromUtf8(buffer.constData()).toDouble()));
        break;
    case VIRTUOSO_DV_STRING:
        {
            int boxFlags = 0;
            SQLGetDescField(p->hdesc, colNum, SQL_DESC_COL_BOX_FLAGS, &boxFlags, SQL_IS_INTEGER, 0);

            if ((boxFlags & VIRTUOSO_BF_IRI) != 0) {
                if (qstrncmp(buffer.constData(), "nodeID://", 9) == 0) {
                    b.setBlankNodeLabel(QString::fromUtf8(buffer.constData() + 9));
                } else {
                    b.setValue(QUrl(QString::fromUtf8(buffer.constData())));
                }
            } else {
                if (qstrncmp(buffer.constData(), "_:", 2) == 0) {
                    b.setBlankNodeLabel(QString::fromUtf8(buffer.constData() + 2));
                } else if ((boxFlags & VIRTUOSO_BF_UTF8) != 0) {
                    b.setValue(QString::fromUtf8(buffer.constData()));
                } else {
                    b.setValue(QString::fromLatin1(buffer.constData()));
                }
            }
            break;
        }
    }

    b.setName(p->bindingNames[colNum - 1]);
    return b;
}

bool QVirtuosoResult::fetchNextResult()
{
    QMutexLocker connectionLocker(&(d->driverPrivate->mutex));
    SQLRETURN r;
    r = SQLFetch(d->hstmt);

    if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
        if (r != SQL_NO_DATA)
            setLastError(qMakeError(QCoreApplication::translate("QVirtuosoResult",
                "Unable to fetch next"), QSparqlError::BackendError, d));
        terminate();
        return false;

    }

    QMutexLocker resultLocker(&(d->mutex));
    d->clearValues();

    for (d->resultColIdx = 1; d->resultColIdx <= d->numResultCols; ++(d->resultColIdx)) {
        d->results[d->results.count() - 1].append(qMakeBinding(d, d->resultColIdx));
    }

    emit dataReady(d->results.count());
    return true;
}

bool QVirtuosoResult::fetchBoolResult()
{
    QMutexLocker connectionLocker(&(d->driverPrivate->mutex));
    SQLRETURN r = SQLFetch(d->hstmt);
    QMutexLocker resultLocker(&(d->mutex));

    if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
        if (r != SQL_NO_DATA)
            setLastError(qMakeError(QCoreApplication::translate("QVirtuosoResult",
                "Unable to fetch next"), QSparqlError::BackendError, d));
        setBoolValue(false);
        terminate();
        return false;

    }

    QSparqlBinding retval = qMakeBinding(d, 1);
    setBoolValue(retval.name().toUpper() == QLatin1String("__ASK_RETVAL") && retval.value().toInt() == 1);
    terminate();
    return true;
}

bool QVirtuosoResult::fetchGraphResult()
{
    QMutexLocker connectionLocker(&(d->driverPrivate->mutex));
    SQLRETURN r = SQLFetch(d->hstmt);
    QMutexLocker resultLocker(&(d->mutex));

    if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
        if (r != SQL_NO_DATA)
            setLastError(qMakeError(QCoreApplication::translate("QVirtuosoResult",
                "Unable to fetch next"), QSparqlError::BackendError, d));
        setBoolValue(false);
        terminate();
        return false;

    }

    QSparqlBinding retval = qMakeBinding(d, 1);

    if (retval.name().toUpper() == QLatin1String("FMTAGGRET-NT")) {
        QByteArray buffer = retval.value().toString().toLatin1();
        QSparqlNTriples parser(buffer);
        d->results = parser.parse();
    }

    terminate();
    return true;
}

QSparqlBinding QVirtuosoResult::binding(int field) const
{
    QMutexLocker resultLocker(&(d->mutex));

    if (!isValid()) {
        return QSparqlBinding();
    }

    if (field >= d->results[pos()].count() || field < 0) {
        qWarning() << "QVirtuosoResult::data[" << pos() << "]: column" << field << "out of range";
        return QSparqlBinding();
    }

    return d->results[pos()].binding(field);
}

QVariant QVirtuosoResult::value(int field) const
{
    QMutexLocker resultLocker(&(d->mutex));

    if (!isValid()) {
        return QVariant();
    }

    if (field >= d->results[pos()].count() || field < 0) {
        qWarning() << "QVirtuosoResult::data[" << pos() << "]: column" << field << "out of range";
        return QVariant();
    }
    return d->results[pos()].value(field);
}

int QVirtuosoResult::size() const
{
    // TODO: this lock sometimes seg fault here. Is that because
    // size() can be called before the QSparqlResult is fully
    // constructed?
    QMutexLocker resultLocker(&(d->mutex));
    return d->results.count();
}

QSparqlResultRow QVirtuosoResult::current() const
{
    QMutexLocker resultLocker(&(d->mutex));

    if (!isValid()) {
        return QSparqlResultRow();
    }

    if (pos() < 0 || pos() >= d->results.count())
        return QSparqlResultRow();

    return d->results[pos()];
}

QVariant QVirtuosoResult::handle() const
{
    return QVariant(qRegisterMetaType<SQLHANDLE>("SQLHANDLE"), &d->hstmt);
}

////////////////////////////////////////


QVirtuosoDriver::QVirtuosoDriver(QObject *parent)
    : QSparqlDriver(parent)
{
    init();
}

QVirtuosoDriver::QVirtuosoDriver(SQLHANDLE env, SQLHANDLE con, QObject * parent)
    : QSparqlDriver(parent)
{
    init();
    d->hEnv = env;
    d->hDbc = con;
    if (env && con) {
        setOpen(true);
        setOpenError(false);
    }
}

void QVirtuosoDriver::init()
{
    d = new QVirtuosoDriverPrivate();
}

QVirtuosoDriver::~QVirtuosoDriver()
{
    cleanup();
    delete d;
}

bool QVirtuosoDriver::hasFeature(QSparqlConnection::Feature f) const
{
    switch (f) {
    case QSparqlConnection::AskQueries:
        return true;
    case QSparqlConnection::ConstructQueries:
        return true;
    case QSparqlConnection::UpdateQueries:
        return true;
    case QSparqlConnection::DefaultGraph:
        return false;
    }
    return false;
}

bool QVirtuosoDriver::open(const QSparqlConnectionOptions& options)
{
    QMutexLocker connectionLocker(&(d->mutex));

    if (isOpen())
      close();
    SQLRETURN r;
    r = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &d->hEnv);
    if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
        qSparqlWarning(QLatin1String("QVirtuosoDriver::open: Unable to allocate environment"), d);
        setOpenError(true);
        return false;
    }

        // set odbc version
    SQLSetEnvAttr(d->hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_UINTEGER);

    r = SQLAllocHandle(SQL_HANDLE_DBC, d->hEnv, &d->hDbc);
    if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
        qSparqlWarning(QLatin1String("QVirtuosoDriver::open: Unable to allocate connection"), d);
        setOpenError(true);
        return false;
    }

    // Create the connection string
    QString connectString;
    // support the "DRIVER={SQL SERVER};SERVER=blah" syntax
    if (options.databaseName().contains(QLatin1String(".dsn"), Qt::CaseInsensitive))
        connectString = QLatin1String("FILEDSN=") + options.databaseName();
    else if (options.databaseName().contains(QLatin1String("DRIVER="), Qt::CaseInsensitive)
            || options.databaseName().contains(QLatin1String("SERVER="), Qt::CaseInsensitive))
        connectString = options.databaseName();
    else
        connectString = QLatin1String("DSN=") + options.databaseName();

    QString hostName;
    int port;
    if (options.hostName().isEmpty())
        hostName = QLatin1String("localhost");
    else
        hostName = options.hostName();

    if (options.port() == -1)
        port = 1111;
    else
        port = options.port();

    connectString += QString(QLatin1String(";HOST=%1:%2")).arg(hostName).arg(port);

    if (options.userName().isEmpty())
        connectString += QLatin1String(";UID=dba");
    else
        connectString += QLatin1String(";UID=") + options.userName();

    if (options.password().isEmpty())
        connectString += QLatin1String(";PWD=dba");
    else
        connectString += QLatin1String(";PWD=") + options.password();

    SQLSMALLINT cb;
    SQLTCHAR connectionOut[4097];
    connectionOut[4096] = 0;

    r = SQLDriverConnect( d->hDbc,
                          0,
                          (UCHAR*) connectString.toUtf8().data(),
                          SQL_NTS,
                          connectionOut,
                          4096,
                          &cb,
                          SQL_DRIVER_COMPLETE);
    if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
        setLastError(qMakeError(tr("Unable to connect"), QSparqlError::ConnectionError, d));
        setOpenError(true);
        return false;
    }

    setOpen(true);
    setOpenError(false);
    return true;
}

void QVirtuosoDriver::close()
{
    cleanup();
    setOpen(false);
    setOpenError(false);
}

void QVirtuosoDriver::cleanup()
{
    QMutexLocker connectionLocker(&(d->mutex));

    SQLRETURN r;
    if (!d)
        return;

    if(d->hDbc) {
        // Open statements/descriptors handles are automatically cleaned up by SQLDisconnect
        if (isOpen()) {
            r = SQLDisconnect(d->hDbc);
            if (r != SQL_SUCCESS)
                qSparqlWarning(QLatin1String("QVirtuosoDriver::disconnect: Unable to disconnect datasource"), d);
            else
                d->disconnectCount++;
        }

        r = SQLFreeHandle(SQL_HANDLE_DBC, d->hDbc);
        if (r != SQL_SUCCESS)
            qSparqlWarning(QLatin1String("QVirtuosoDriver::cleanup: Unable to free connection handle"), d);
        d->hDbc = 0;
    }

    if (d->hEnv) {
        r = SQLFreeHandle(SQL_HANDLE_ENV, d->hEnv);
        if (r != SQL_SUCCESS)
            qSparqlWarning(QLatin1String("QVirtuosoDriver::cleanup: Unable to free environment handle"), d);
        d->hEnv = 0;
    }
}

QVirtuosoResult *QVirtuosoDriver::createResult() const
{
    return new QVirtuosoResult(this, d);
}

bool QVirtuosoDriver::beginTransaction()
{
    QMutexLocker connectionLocker(&(d->mutex));

    if (!isOpen()) {
        qWarning() << "QVirtuosoDriver::beginTransaction: Database not open";
        return false;
    }
    SQLUINTEGER ac(SQL_AUTOCOMMIT_OFF);
    SQLRETURN r  = SQLSetConnectAttr(d->hDbc,
                                      SQL_ATTR_AUTOCOMMIT,
                                      (SQLPOINTER)ac,
                                      sizeof(ac));
    if (r != SQL_SUCCESS) {
        setLastError(qMakeError(tr("Unable to disable autocommit"),
                     QSparqlError::TransactionError, d));
        return false;
    }
    return true;
}

bool QVirtuosoDriver::commitTransaction()
{
    QMutexLocker connectionLocker(&(d->mutex));

    if (!isOpen()) {
        qWarning() << "QVirtuosoDriver::commitTransaction: Database not open";
        return false;
    }
    SQLRETURN r = SQLEndTran(SQL_HANDLE_DBC,
                              d->hDbc,
                              SQL_COMMIT);
    if (r != SQL_SUCCESS) {
        setLastError(qMakeError(tr("Unable to commit transaction"),
                     QSparqlError::TransactionError, d));
        return false;
    }
    return endTrans();
}

bool QVirtuosoDriver::rollbackTransaction()
{
    QMutexLocker connectionLocker(&(d->mutex));

    if (!isOpen()) {
        qWarning() << "QVirtuosoDriver::rollbackTransaction: Database not open";
        return false;
    }
    SQLRETURN r = SQLEndTran(SQL_HANDLE_DBC,
                              d->hDbc,
                              SQL_ROLLBACK);
    if (r != SQL_SUCCESS) {
        setLastError(qMakeError(tr("Unable to rollback transaction"),
                     QSparqlError::TransactionError, d));
        return false;
    }
    return endTrans();
}

bool QVirtuosoDriver::endTrans()
{
    QMutexLocker connectionLocker(&(d->mutex));

    SQLUINTEGER ac(SQL_AUTOCOMMIT_ON);
    SQLRETURN r  = SQLSetConnectAttr(d->hDbc,
                                      SQL_ATTR_AUTOCOMMIT,
                                      (SQLPOINTER)ac,
                                      sizeof(ac));
    if (r != SQL_SUCCESS) {
        setLastError(qMakeError(tr("Unable to enable autocommit"), QSparqlError::TransactionError, d));
        return false;
    }
    return true;
}


QVariant QVirtuosoDriver::handle() const
{
    return QVariant(qRegisterMetaType<SQLHANDLE>("SQLHANDLE"), &d->hDbc);
}

QT_END_NAMESPACE
