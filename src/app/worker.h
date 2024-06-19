#pragma once
#include <functional>
#include <qobject.h>
#include <qobjectdefs.h>



class Worker : public QObject {
    Q_OBJECT

public:
    Worker(std::function<void()> entry);
    ~Worker();
    void execute_work();

signals:
    void work_done();

private:
    std::function<void()> m_entry;
};
