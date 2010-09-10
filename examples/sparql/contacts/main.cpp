/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (ivan.frade@nokia.com)
**
** This file is part of the examples of the QtSparql module (not yet part of the Qt Toolkit).
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

#include "ui_list.h"
#include "ui_contact.h"
#include "ui_add.h"
#include "ui_main.h"

#include <QSparqlConnection>
#include <QSparqlBinding>
#include <QSparqlQuery>
#include <QSparqlQueryModel>
#include <QSparqlResult>
#include <QSparqlError>

#include <QObject>
#include <QApplication>
#include <QUrl>

#include <QDebug>

class ContactView : public QObject
{
    Q_OBJECT

public:
    ContactView(Ui::ListUI* ui, QWidget* w, QSparqlConnection& c);

signals:
    // Signals used for exiting this view
    void details(QString uri);
    void addContact();

public slots:
    void showContactList();

private slots:
    void onContactClicked(const QModelIndex& index);

private:
    QSparqlConnection& conn;
    QSparqlQuery getContacts;
    QSparqlQuery getDetails;
    QSparqlQueryModel contactModel;
    Ui::ListUI* ui;
    QWidget* widget;
};

ContactView::ContactView(Ui::ListUI* ui, QWidget* w, QSparqlConnection& c)
    : conn(c),
      getContacts("select ?u fn:string-join((?ng, ?nf), ' ') "
                  "{ ?u a nco:PersonContact ; "
                  "nco:nameGiven ?ng ; "
                  "nco:nameFamily ?nf . } order by ?ng ?nf"),
      ui(ui),
      widget(w)
{
    ui->setupUi(widget);
    widget->hide();

    ui->contactTable->setModel(&contactModel);
    // There is no data yet; tell the model we have 2 columns
    contactModel.insertColumns(0, 2);
    // So that we can hide one of them
    ui->contactTable->setColumnHidden(0, true);
    ui->contactTable->setColumnWidth(1, ui->contactTable->size().width());
    ui->contactTable->verticalHeader()->setDefaultSectionSize(60);

    connect(ui->contactTable, SIGNAL(clicked(const QModelIndex&)),
            this, SLOT(onContactClicked(const QModelIndex&)));

    connect(ui->addButton, SIGNAL(clicked(bool)),
            widget, SLOT(hide()));
    connect(ui->addButton, SIGNAL(clicked(bool)),
            this, SIGNAL(addContact()));
}

void ContactView::onContactClicked(const QModelIndex& ix)
{
    QString uri = ui->contactTable->model()->data(
        ui->contactTable->model()->index(ix.row(), 0)).toString();
    widget->hide();
    emit details(uri);
}

void ContactView::showContactList()
{
    contactModel.setQuery(getContacts, conn);
    // The model forgets its column count if we setQuery again.
    contactModel.insertColumns(0, 2);
    ui->contactTable->setColumnHidden(0, true);
    ui->contactTable->setColumnWidth(1, ui->contactTable->size().width());
    widget->show();
}

class DetailView : public QObject
{
    Q_OBJECT
public:
    DetailView(Ui::ContactUI* ui, QWidget* w, QSparqlConnection& c);
    ~DetailView();
signals:
    void backToList();
public slots:
    void showDetails(const QString& u);
    void removeContact();
private slots:
    void nameQueryFinished();
    void removeFinished();
private:
    QSparqlConnection& conn;
    QSparqlQuery nameQuery;
    QSparqlQuery phoneNumberQuery;
    QSparqlQuery removeQuery;
    QSparqlQueryModel phoneNumberModel;
    QSparqlResult* nameResult;
    QSparqlResult* removeResult;
    QString uri;
    Ui::ContactUI* ui;
    QWidget* widget;
};

/* This is for docs

    QSparqlQuery nameQuery("select ?ng ?nf "
    "{ ?:contact_uri nco:nameGiven ?ng ; nco:nameFamily ?nf . } ");

 */

DetailView::DetailView(Ui::ContactUI* ui, QWidget* w, QSparqlConnection& c)
    : conn(c),
      nameQuery("select fn:string-join((?ng, ?nf), ' ') "
                "{ ?:contact_uri nco:nameGiven ?ng ; "
                "nco:nameFamily ?nf . } "),
      phoneNumberQuery("select ?p { ?:contact_uri nco:hasPhoneNumber ?pn . "
                      "?pn nco:phoneNumber ?p . }"),
      removeQuery("delete { ?pn a rdfs:Resource . }"
                  "WHERE { ?:contact_uri nco:hasPhoneNumber ?pn . } "
                  "delete { ?:contact_uri a rdfs:Resource . }",
                  QSparqlQuery::DeleteStatement),
      nameResult(0),
      removeResult(0),
      ui(ui),
      widget(w)
{
    ui->setupUi(widget);
    widget->hide();

    phoneNumberModel.insertColumns(0, 1);
    ui->phoneNumberTable->setModel(&phoneNumberModel);
    ui->phoneNumberTable->setColumnWidth(0, ui->phoneNumberTable->size().width());
    ui->phoneNumberTable->verticalHeader()->setDefaultSectionSize(60);

    connect(ui->backButton, SIGNAL(clicked(bool)), widget, SLOT(hide()));
    connect(ui->backButton, SIGNAL(clicked(bool)), this, SIGNAL(backToList()));
    connect(ui->removeButton, SIGNAL(clicked(bool)), this, SLOT(removeContact()));
}

void DetailView::showDetails(const QString& u)
{
    uri = u;
    nameQuery.unbindValues();
    nameQuery.bindValue("contact_uri", QUrl(uri));

    delete nameResult;
    nameResult = conn.exec(nameQuery);
    connect(nameResult, SIGNAL(finished()), this, SLOT(nameQueryFinished()));

    phoneNumberQuery.unbindValues();
    phoneNumberQuery.bindValue("contact_uri", QUrl(uri));
    phoneNumberModel.setQuery(phoneNumberQuery, conn);

    ui->removedLabel->hide();
    widget->show();
}

void DetailView::nameQueryFinished()
{
    if (nameResult->hasError()) {
        qDebug() << nameResult->lastError();
        return;
    }
    if (!nameResult->next()) {
        return;
    }
    ui->nameLabel->setText(nameResult->binding(0).value().toString());
}

void DetailView::removeContact()
{
    removeQuery.unbindValues();
    removeQuery.bindValue("contact_uri", QUrl(uri));
    delete removeResult;
    removeResult = conn.exec(removeQuery);
    connect(removeResult, SIGNAL(finished()), this, SLOT(removeFinished()));
}

void DetailView::removeFinished()
{
    if (removeResult->hasError()) {
        qDebug() << removeResult->lastError();
        return;
    }
    ui->removedLabel->show();
}

DetailView::~DetailView()
{
    delete nameResult;
    delete removeResult;
}

class AddView : public QObject
{
    Q_OBJECT
public:
    AddView(Ui::AddUI* ui, QWidget* w, QSparqlConnection& c);
    ~AddView();
signals:
    void backToList();
public slots:
    void showAddView();
private slots:
    void addContact();
    void addFinished();
private:
    QSparqlConnection& conn;
    QSparqlQuery addQuery;
    QSparqlResult* addResult;
    Ui::AddUI* ui;
    QWidget* widget;
};

AddView::AddView(Ui::AddUI* ui, QWidget* w, QSparqlConnection& c)
    : conn(c),
      addQuery("insert { _:c a nco:PersonContact ; "
               "nco:nameGiven ?:user_name_given ; "
               "nco:nameFamily ?:user_name_family ; "
               "nco:hasPhoneNumber _:pn . "
               "_:pn a nco:PhoneNumber ; "
               "nco:phoneNumber ?:user_phone . }",
               QSparqlQuery::InsertStatement),
      addResult(0),
      ui(ui),
      widget(w)
{
    ui->setupUi(widget);
    widget->hide();

    connect(ui->backButton, SIGNAL(clicked(bool)), widget, SLOT(hide()));
    connect(ui->backButton, SIGNAL(clicked(bool)), this, SIGNAL(backToList()));
}

void AddView::showAddView()
{
    ui->addedLabel->hide();
    ui->nameEdit1->setEnabled(true);
    ui->nameEdit1->clear();
    ui->nameEdit2->setEnabled(true);
    ui->nameEdit2->clear();
    ui->phoneEdit->setEnabled(true);
    ui->phoneEdit->clear();
    ui->addButton->setText("Add");
    ui->addedLabel->hide();
    ui->addButton->disconnect();
    connect(ui->addButton, SIGNAL(clicked(bool)), this, SLOT(addContact()));
    widget->show();
}

void AddView::addContact()
{
    addQuery.unbindValues();
    addQuery.bindValue("user_name_given",
                       ui->nameEdit1->text());
    addQuery.bindValue("user_name_family",
                       ui->nameEdit2->text());
    addQuery.bindValue("user_phone",
                       ui->phoneEdit->text());

    delete addResult;
    addResult = conn.exec(addQuery);
    connect(addResult, SIGNAL(finished()), this, SLOT(addFinished()));
}

void AddView::addFinished()
{
    if (addResult->hasError()) {
        qDebug() << addResult->lastError();
        ui->addedLabel->setText("Error adding contact");
        ui->addedLabel->show();
        return;
    }
    ui->addedLabel->setText("Contact added");
    ui->addedLabel->show();
    ui->nameEdit1->setEnabled(false);
    ui->nameEdit2->setEnabled(false);
    ui->phoneEdit->setEnabled(false);
    ui->addButton->setText("Add more");
    ui->addButton->disconnect();
    connect(ui->addButton, SIGNAL(clicked(bool)), this, SLOT(showAddView()));
}

AddView::~AddView()
{
    delete addResult;
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    // A hack; didn't find an elegant way for doing this
    app.setStyleSheet("QScrollBar:vertical { width: 60px; } ");

    Ui::ListUI listUi;
    Ui::ContactUI contactUi;
    Ui::AddUI addUi;
    Ui::MainUI mainUi;
    QWidget mainWindow;
    mainUi.setupUi(&mainWindow);

    QSparqlConnection conn("QTRACKER");

    ContactView contactView(&listUi, mainUi.contactWidget, conn);
    DetailView detailView(&contactUi, mainUi.detailWidget, conn);
    AddView addView(&addUi, mainUi.addWidget, conn);

    QObject::connect(&contactView, SIGNAL(details(QString)),
                     &detailView, SLOT(showDetails(QString)));

    QObject::connect(&contactView, SIGNAL(addContact()),
                     &addView, SLOT(showAddView()));

    QObject::connect(&detailView, SIGNAL(backToList()),
                     &contactView, SLOT(showContactList()));

    QObject::connect(&addView, SIGNAL(backToList()),
                     &contactView, SLOT(showContactList()));

    QObject::connect(listUi.exitButton, SIGNAL(clicked(bool)),
                     &app, SLOT(quit()));
    contactView.showContactList();
    mainWindow.show();

    return app.exec();
}

#include "moc_main.cpp"
