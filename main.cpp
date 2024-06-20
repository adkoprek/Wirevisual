#include "data_point.h"
#include "epics_commands.h"
#include "mainwindow.h"
#include "src/logic/data_fetch.h"
#include <QApplication>
#include <cstddef>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

int test_data_fetch();

int main(int argc, char* argv[]) {
    // test_data_fetch();
    // return 0;

    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}

int test_data_fetch() {
    CAFE* cafe = new CAFE();
    cafe->channelOpenPolicy.setTimeout(5.0);
    
    std::string command_x = "MHP11" + PROFILE_PV_X;
    std::vector<std::string> pvs = { command_x };
    std::vector<unsigned int> handles;

    double buffer_1[400];
    double buffer_2[400];

    cafe->open(pvs, handles);
    int status = cafe->get(command_x.c_str(), buffer_1);
    cafe->closeHandlesV(handles);
    std::cout << "Buffer 1: ";
    for (size_t i = 0; i < 400; i++) 
        std::cout << buffer_1[i] << " ";
    std::cout << std::endl;


    cafe->open(pvs, handles);
    status = cafe->get(command_x.c_str(), buffer_2);
    cafe->closeHandlesV(handles);
    std::cout << "Buffer 2: ";
    for (size_t i = 0; i < 400; i++) 
        std::cout << buffer_2[i] << " ";
    std::cout << std::endl;

    return 0;

    // Create a DataFetch instance
    DataFetch data_fetch;
    std::vector<std::string> to_fetch = { "MHP11" };

    // Start the thread
    std::thread fethcing(&DataFetch::fetch, &data_fetch, to_fetch);
    std::cout << "Started Fetching" << std::endl;
    fethcing.join();

    DataPoint* point = data_fetch.get_data_point("MHP11");
    for (size_t i = 0; i < point->y.size(); i++) {
        std::cout << point->y[i] << " ";
    }

    std::thread fethcing_2(&DataFetch::fetch, &data_fetch, to_fetch);
    std::cout << "Started Fetching 2" << std::endl;
    fethcing_2.join();

    DataPoint* point_2 = data_fetch.get_data_point("MHP11");
    for (size_t i = 0; i < point_2->y.size(); i++) {
        std::cout << point_2->y[i] << " ";
    }
}
