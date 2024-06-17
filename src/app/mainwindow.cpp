#include "mainwindow.h"
#include "profiles.h"
#include <QWidget>
#include <QMainWindow>
#include <cstddef>
#include <iterator>
#include <qlistwidget.h>
#include <qnamespace.h>
#include <string>


MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    custom_ui_setup();
}

void MainWindow::custom_ui_setup() {
    ui.setupUi(this);                       
    ui.fit_2sigmafit->click();              
    connect(ui.beamline_list, &QListWidget::itemClicked, this, &MainWindow::on_beamline_clicked);
    connect(ui.beamline_list, &QListWidget::itemChanged, this, &MainWindow::on_beamline_selected);
    on_beamline_clicked(ui.beamline_list->item(0));
}

void MainWindow::on_beamline_clicked(QListWidgetItem* item) {
    std::string selected_item(item->text().toUtf8().constData());
    auto profiles_to_display = &profiles.at(selected_item);
    
    ui.profile_list->clear();
    for (size_t i = 0; i < profiles_to_display->size(); i++) {
        auto profile_name = profiles_to_display->at(i);
        auto item = new QListWidgetItem(tr(profile_name.c_str()));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        
        auto element_index = std::find(m_selected.begin(), m_selected.end(), 
                selected_item + profile_name);
        if (element_index != m_selected.end()) 
            item->setCheckState(Qt::Checked); 
        else
            item->setCheckState(Qt::Unchecked); 

        ui.profile_list->insertItem(i, item);
    }
}

void MainWindow::on_beamline_selected(QListWidgetItem* item) {
    std::string selected_item(item->text().toUtf8().constData());
    auto profiles_to_display = &profiles.at(selected_item);
    
    for (size_t i = 0; i < profiles_to_display->size(); i++) {
        auto profile_name = profiles_to_display->at(i);
        auto element_index = std::find(m_selected.begin(), m_selected.end(), 
                selected_item + profile_name);
        if (element_index != m_selected.end()) 
            m_selected.erase(element_index);
        else
            m_selected.push_back(selected_item + profile_name);
    }

    on_beamline_clicked(ui.beamline_list->item(std::distance(profiles.begin(), profiles.find(selected_item))));
}
