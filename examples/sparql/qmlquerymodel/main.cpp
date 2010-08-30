#include <qdeclarative.h>
#include <QDeclarativeView>
#include <QApplication>
#include "sparqlresultmodel.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.addLibraryPath("../../../../qsparql/plugins");

    qmlRegisterType<SparqlResultModel>("Test", 1, 0, "SparqlModel");

    QDeclarativeView view;
    view.setSource(QUrl::fromLocalFile("/home/rdale/src/qsparql/examples/sparql/qmlquerymodel/albums.qml"));
    view.show();
    return app.exec();
}
