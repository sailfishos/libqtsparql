#include <QtCore/QtCore>
#include <QtGui/QTextEdit>
#include <QtGui/QTableView>

#include <QtSparql/QSparqlConnection>
#include <QtSparql/QSparqlQuery>
#include <QtSparql/QSparqlResult>
#include <QtSparql/QSparqlQueryModel>

class SparqlQueryText : public QTextEdit
{
    Q_OBJECT

public:
    SparqlQueryText(QSparqlConnection& connection, QWidget *parent = 0);
    
public Q_SLOTS:
    void runQuery();
    void showResults();
    
private:
    QSparqlConnection& connection;
    QSparqlQuery *query;
    QSparqlResult *result;
    QSparqlQueryModel *model;
    QTableView *tableView;
};
