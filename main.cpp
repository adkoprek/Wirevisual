#include "mainwindow.h"
#include "test.h"
#include <QApplication>


int main(int argc, char* argv[]) {
    print_test();
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}

