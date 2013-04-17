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

#ifndef MESSAGERECORDER_H
#define MESSAGERECORDER_H

class MessageRecorder {
public:
    MessageRecorder()
    {
        selfPtr = this;
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        prevMsgHandler = qInstallMessageHandler(&MessageRecorder::msgHandler);
#else
        prevMsgHandler = qInstallMsgHandler(&MessageRecorder::msgHandler);
#endif
    }

    ~MessageRecorder()
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        qInstallMessageHandler(prevMsgHandler);
#else
        qInstallMsgHandler(prevMsgHandler);
#endif
        selfPtr = 0;
    }

    void addMsgTypeToRecord(QtMsgType type)      { msgsToRecord.insert(type);    }
    bool hasMsgsOfType(QtMsgType type) const     { return !msgs[type].isEmpty(); }
    QStringList msgsOfType(QtMsgType type) const { return msgs[type];            }
    QStringList operator[](QtMsgType type) const { return msgs[type];            }

private:
    static MessageRecorder* selfPtr;

    template<typename T>
    QString toString(T msg) { return msg; }

    QString toString(const char *msg) { return QString::fromLatin1(msg); }

    template<typename MsgType>
    bool handleMsg(QtMsgType type, MsgType msg)
    {
        if (msgsToRecord.contains(type)) {
            msgs[type] << toString(msg);
            return true;
        }
        return false;
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    static void msgHandler(QtMsgType type, const QMessageLogContext &ctxt, const QString &msg)
#else
    static void msgHandler(QtMsgType type, const char *msg)
#endif
    {
        if (!selfPtr->handleMsg(type, msg)) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
            (*selfPtr->prevMsgHandler)(type, ctxt, msg);
#else
            (*selfPtr->prevMsgHandler)(type, msg);
#endif
        }
    }

private:
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QtMessageHandler prevMsgHandler;
#else
    QtMsgHandler prevMsgHandler;
#endif
    QSet<QtMsgType> msgsToRecord;
    QMap<QtMsgType, QStringList> msgs;
};
MessageRecorder* MessageRecorder::selfPtr = 0;

#endif
