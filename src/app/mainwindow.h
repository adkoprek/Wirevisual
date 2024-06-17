#pragma once
#include <QMainWindow>
#include <qlistwidget.h>
#include <qobjectdefs.h>
#include <string>
#include <vector>
#include "../../forms/ui_mainwindow.h"


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

public slots:
    void on_beamline_clicked(QListWidgetItem* item);
    void on_beamline_selected(QListWidgetItem* item);

private:
    void custom_ui_setup();
    
    Ui::MainWindow ui;
    std::vector<std::string> m_selected;
};

