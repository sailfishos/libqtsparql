/****************************************************************************
**
** Copyright (C) 2010-2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (ivan.frade@nokia.com)
**
** This file is part of QtSparqlTracker library.
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

#include <QtGui/QApplication>
#include <QtDeclarative>
#include <QtSparql>
#include <QtDBus/QtDBus>
#include <QString>

class ModelLiveChange : public QObject
{
    Q_OBJECT
public :
    ModelLiveChange(QDeclarativeView *view);
    ~ModelLiveChange();

    QDeclarativeView *view;

public slots:
    void changed(QString);
};

ModelLiveChange::ModelLiveChange(QDeclarativeView *view)
    : view(view)
{
    // Monitor dbus for any notifications from Tracker.
    // More information about Tracker over Dbus can be found at :
    // http://live.gnome.org/Tracker/Documentation/SignalsOnChanges
    // Please note: a more complete implementation, TrackerChangeNotifier,
    // is available in libqtsparql-tracker-extensions
    QString service("org.freedesktop.Tracker1");
    QString basePath("/org/freedesktop/Tracker1");
    QString resourcesInterface("org.freedesktop.Tracker1.Resources");
    QString resourcesPath("/org/freedesktop/Tracker1/Resources");
    QString changedSignal("GraphUpdated");
    QString changedSignature("sa(iiii)a(iiii)");
    QString syncFunction("Sync");
    // We'll need to use the long form of the class name to watch.
    QString className("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#PersonContact");
    QDBusConnection::sessionBus().connect(service, resourcesPath,
                                          resourcesInterface, changedSignal,
                                          QStringList() << className,
                                          changedSignature,
                                          this, SLOT(changed(QString)));
}

ModelLiveChange::~ModelLiveChange()
{
    // We don't need to delete anything here, cleanup of the objects
    // will be dealt with on the QML side
}

void ModelLiveChange::changed(QString className)
{
    Q_UNUSED(className);
    // We got a change notification from DBus on the class
    // we are watching, so now call the "reload" function in the QML
    // file.
    QMetaObject::invokeMethod(view->rootObject(),
                              "reload",
                              Qt::DirectConnection);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::addLibraryPath("../../../plugins");

    // Initialise QML
    QDeclarativeView viewQml;
    // Load the QML plugins from the build tree, if
    // they haven't been install
    viewQml.engine()->addImportPath("../../../imports");

    viewQml.setSource(QUrl::fromLocalFile("main.qml"));

    // The bindings can be cast back to the appropriate QtSparql classes by setting
    // an "objectName" property on them, and casting them, eg :
    // QSparqlQueryModel *model =
    //            qobject_cast<QSparqlQueryModel *>(viewQml.rootObject()->findChild<QObject*>("queryModel"));
    // QSparqlConnection *connection =
    //            qobject_cast<QSparqlConnection *>(viewQml.rootObject()->findChild<QObject*>("sparqlConnection"));
    // However, we do not need to do that for this example

    // Monitor dbus for changes, we will call a function in the QML file when we get them
    ModelLiveChange changeNotifier(&viewQml);
    viewQml.show();

    return app.exec();
}
#include "main.moc"

/*
How to insert/delete example data into tracker:
tracker-sparql -qu "insert { _:u a nco:PersonContact; nie:isLogicalPartOf <qml-example>; nco:nameGiven 'foo'; nco:nameFamily 'bar' .}"
tracker-sparql -qu "delete { ?u a nco:PersonContact } WHERE { ?u a nco:PersonContact; nie:isLogicalPartOf <qml-example> }"
*/
