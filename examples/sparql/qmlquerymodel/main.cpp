#include <qdeclarative.h>
#include <QDeclarativeView>
#include <QDeclarativeEngine>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.addLibraryPath("../../../plugins");

    QDeclarativeView view;
    view.engine()->addImportPath("../../../imports");
    view.setSource(QUrl::fromLocalFile("albums.qml"));
    view.show();
    return app.exec();
}
