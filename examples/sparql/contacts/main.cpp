#include "ui_list.h"
#include "ui_contact.h"
#include "ui_add.h"
#include "ui_main.h"

#include <QSparqlConnection>
#include <QSparqlQuery>
#include <QSparqlQueryModel>
#include <QSparqlResult>

#include <QObject>
#include <QApplication>

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

DetailView::DetailView(Ui::ContactUI* ui, QWidget* w, QSparqlConnection& c)
    : conn(c),
      nameQuery("select fn:string-join((?ng, ?nf), ' ') "
                "{ <?:contact_uri> nco:nameGiven ?ng ; "
                "nco:nameFamily ?nf . } "),
      phoneNumberQuery("select ?p { <?:contact_uri> nco:hasPhoneNumber ?pn . "
                      "?pn nco:phoneNumber ?p . }"),
      removeQuery("delete { ?pn a rdfs:Resource . }"
                  "WHERE { <?:contact_uri> nco:hasPhoneNumber ?pn . } "
                  "delete { <?:contact_uri> a rdfs:Resource . }",
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
    nameQuery.bindValue("?:contact_uri", uri);

    delete nameResult;
    nameResult = conn.exec(nameQuery);
    connect(nameResult, SIGNAL(finished()), this, SLOT(nameQueryFinished()));

    phoneNumberQuery.unbindValues();
    phoneNumberQuery.bindValue("?:contact_uri", uri);
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
    ui->nameLabel->setText(nameResult->value(0).toString());
}

void DetailView::removeContact()
{
    removeQuery.unbindValues();
    removeQuery.bindValue("?:contact_uri", uri);
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
    ui->nameGivenEdit->setEnabled(true);
    ui->nameGivenEdit->clear();
    ui->nameFamilyEdit->setEnabled(true);
    ui->nameFamilyEdit->clear();
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
    addQuery.bindValue("?:user_name_given",
                       ui->nameGivenEdit->text().append("'").prepend("'"));
    addQuery.bindValue("?:user_name_family",
                       ui->nameFamilyEdit->text().append("'").prepend("'"));
    addQuery.bindValue("?:user_phone",
                       ui->phoneEdit->text().append("'").prepend("'"));

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
    ui->nameGivenEdit->setEnabled(false);
    ui->nameFamilyEdit->setEnabled(false);
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

    contactView.showContactList();
    mainWindow.show();

    return app.exec();
}

#include "moc_main.cpp"