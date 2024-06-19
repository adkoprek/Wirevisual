#include "worker.h"
#include <functional>
#include <qobjectdefs.h>


Worker::Worker(std::function<void()> entry) {
    m_entry = entry;
}

Worker::~Worker() {}

void Worker::execute_work() {
    m_entry();
    emit work_done();
}
