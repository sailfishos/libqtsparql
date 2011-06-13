#include "qsparql_tracker_direct_result_p.h"
#include <QDebug>


void QTrackerDirectResult::stopAndWait() {
    queryRunner->wait();
}

void QTrackerDirectResult::driverClosing() {
    qWarning() << "QSparqlConnection closed before QSparqlResult with query:" <<
                  query();
    setLastError(QSparqlError(QString::fromUtf8("QSparqlConnection closed before QSparqlResult"),
                    QSparqlError::ConnectionError,
                    -1));
    stopAndWait();
}

