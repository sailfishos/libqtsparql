#include "qsparql_tracker_direct_result_p.h"
#include <QDebug>

void QTrackerDirectResult::stopAndWait() {
    queryRunner->wait();
}



