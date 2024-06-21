#include "data_point.h"
#include "mainwindow.h"
#include "src/logic/data_fetch.h"
#include <QApplication>
#include <iostream>
#include <string>
#include <thread>
#include <vector>


int test_data_fetch();

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}

int test_data_fetch() {
    // Create a DataFetch instance
    DataFetch data_fetch;
    std::vector<std::string> to_fetch = { "MHP11" };

    // Start the thread
    std::thread fethcing(&DataFetch::fetch, &data_fetch, to_fetch);
    std::cout << "Started Fetching" << std::endl;
    fethcing.join();

    DataPoint* point = data_fetch.get_data_point("MHP11");
    for (int i = 0; i < point->y.size(); i++) {
        std::cout << point->y[i] << ", ";
    }
    return 0;
}
