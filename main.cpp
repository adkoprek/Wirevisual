//  _    _ _                _           _ 
// | |  | (_)              (_)         | |
// | |  | |_ _ __ _____   ___ ___  __ _| |
// | |/\| | | '__/ _ \ \ / / / __|/ _` | |
// \  /\  / | | |  __/\ V /| \__ \ (_| | |
//  \/  \/|_|_|  \___| \_/ |_|___/\__,_|_|
// https://git.psi.ch/hipa_apps/Wirevisual
//
// The main function that should create an Qt Application
//
// @Author: Adam Koprek
// @Maintainer: Jochem Snuvernik

#include <QApplication>

#include "mainwindow.h"


int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}
