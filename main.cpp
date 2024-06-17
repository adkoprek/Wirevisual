#include "mainwindow.h"
#include "src/logic/data_fetch.h"
#include <QApplication>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>
#include <vector>

int test_data_fetch();

int main(int argc, char* argv[]) {
    // test_data_fetch();

    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();

}

int test_data_fetch() {
    // Create a DataFetch instance
    DataFetch data_fetch;
    std::vector<uint8_t> to_fetch = { 0, 1, 2, 3, 4, 5 };

    // Start the thread
    std::thread fethcing(&DataFetch::fetch, &data_fetch, to_fetch);
    std::cout << "Started Thread" << std::endl;

    // Test the stop functionality after 3s
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    std::cout << "Stopping for 5s" << std::endl;
    data_fetch.stop();

    // Test the resume functionality after 5s
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    std::cout << "Resuming thread" << std::endl;
    data_fetch.resume();

    // Test the cancel functionality after 1s
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << "Cancelingt thread" << std::endl;
    data_fetch.cancel();

    fethcing.join();
    return 0;
}

