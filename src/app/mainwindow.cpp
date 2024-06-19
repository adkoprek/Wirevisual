#include "mainwindow.h"
#include "data_fetch.h"
#include "profiles.h"
#include "worker.h"
#include <QWidget>
#include <QMainWindow>
#include <cstddef>
#include <cstdlib>
#include <QThread>
#include <iterator>
#include <QListWidget>
#include <qlist.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qwidget.h>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>
#include <qwt_plot.h>


MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    custom_ui_setup();
    m_data_fetch = new DataFetch();
}

MainWindow::~MainWindow() {
    delete m_data_fetch;
}

void MainWindow::custom_ui_setup() {
    ui.setupUi(this);                       
    ui.fit_2sigmafit->click();              
    connect(ui.beamline_list, &QListWidget::itemClicked, this, &MainWindow::on_beamline_clicked);
    connect(ui.beamline_list, &QListWidget::itemChanged, this, &MainWindow::on_beamline_selected);
    connect(ui.profile_list, &QListWidget::itemChanged, this, &MainWindow::on_profile_selected);
    connect(ui.measure_button, &QPushButton::clicked, this, &MainWindow::on_measure_clicked);
    connect(ui.cancel_button, &QPushButton::clicked, this, &MainWindow::on_cancel_clicked);
    connect(ui.stop_button, &QPushButton::clicked, this, &MainWindow::on_stop_clicked);
    connect(ui.resume_button, &QPushButton::clicked, this, &MainWindow::on_resume_clicked);
    connect(ui.transport_button, &QPushButton::clicked, this, &MainWindow::on_transport_clicked);
    connect(ui.mint_button, &QPushButton::clicked, this, &MainWindow::on_mint_clicked);
    connect(ui.replay_button, &QPushButton::clicked, this, &MainWindow::on_replay_clicked);

    on_beamline_clicked(ui.beamline_list->item(0));

    m_diagram_layout_1 = new QVBoxLayout();
    m_diagram_layout_2 = new QVBoxLayout();
    auto widget1 = new QWidget();
    widget1->setLayout(m_diagram_layout_1);
    ui.scrollArea->setWidget(widget1);
    auto widget2 = new QWidget();
    widget2->setLayout(m_diagram_layout_2);
    ui.scrollArea_2->setWidget(widget2);
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
                selected_item + "/" + profile_name);
        if (element_index != m_selected.end()) 
            item->setCheckState(Qt::Checked); 
        else
            item->setCheckState(Qt::Unchecked); 

        ui.profile_list->insertItem(i, item);
    }
}

void MainWindow::on_beamline_selected(QListWidgetItem* item) {
    ui.beamline_list->setCurrentItem(item);

    if (m_program_uncheck) {
        m_program_uncheck = false;
        return;
    }

    std::string selected_item(item->text().toUtf8().constData());
    auto profiles_to_display = &profiles.at(selected_item);
    bool checked = item->checkState() == Qt::Checked;
    
    for (size_t i = 0; i < profiles_to_display->size(); i++) {
        auto profile_name = profiles_to_display->at(i);
        auto element_index = std::find(m_selected.begin(), m_selected.end(), 
                selected_item + "/" + profile_name);
        if (element_index == m_selected.end()) {
            if (checked)
                m_selected.push_back(selected_item + "/" + profile_name);
        }
        else {
            if (!checked) 
                m_selected.erase(element_index);
        }
        
    }

    on_beamline_clicked(ui.beamline_list->item(std::distance(profiles.begin(), profiles.find(selected_item))));
}

void MainWindow::on_profile_selected(QListWidgetItem* item) {
    ui.profile_list->setCurrentItem(item);
    QListWidgetItem* current_beamline_item = ui.beamline_list->currentItem();
    std::string current_beamline(current_beamline_item->text().toUtf8().constData());
    std::string selected_item(item->text().toUtf8().constData());

    auto element_index = std::find(m_selected.begin(), m_selected.end(), 
                current_beamline + "/" + selected_item);

    if (element_index != m_selected.end()) 
        m_selected.erase(element_index);
    else
        m_selected.push_back(current_beamline + "/" + selected_item);

    if (item->checkState() == Qt::Unchecked) {
        m_program_uncheck = true;
        current_beamline_item->setCheckState(Qt::Unchecked);
    }
}

void MainWindow::on_measure_clicked() {
    m_to_fetch.clear();
    for (int i = 0; i < m_selected.size(); i++) {
        std::stringstream ss(m_selected[i]);
        std::string part;
        int count = 0;
        while (getline(ss, part, '/')) {
            if (count == 1) m_to_fetch.push_back(part);
            count++;
        }
    }
    m_selected.clear();
    reset_beamlines();

    measure();
}

void MainWindow::on_cancel_clicked() {
    m_data_fetch->cancel();
}

void MainWindow::on_stop_clicked() {
    m_data_fetch->stop();
}

void MainWindow::on_resume_clicked() {
    m_data_fetch->resume();
}

void MainWindow::on_transport_clicked() {
    // Open Transport
}

void MainWindow::on_mint_clicked() {
    // Open Mint
}

void MainWindow::on_replay_clicked() {
    measure();
}

void MainWindow::measure() {
    if (m_busy) return;
    m_busy = true;

    QThread* thread = new QThread();
    Worker* worker = new Worker([this]{ m_data_fetch->fetch(m_to_fetch); });
    worker->moveToThread(thread);
    connect(thread, &QThread::started, worker, &Worker::execute_work);
    connect(worker, &Worker::work_done, this, &MainWindow::create_diagrams, Qt::QueuedConnection);
    thread->start();
}

void MainWindow::create_diagrams() {
    m_busy = false;

    for (size_t i = 0; i < m_to_fetch.size(); i++) {
        QwtPlot* plot = new QwtPlot();
        m_diagram_layout_1->addWidget(plot);
    }
}

void MainWindow::reset_beamlines() {
    for (size_t i = 0; i < 12; i++) {
        ui.beamline_list->item(i)->setCheckState(Qt::Unchecked);
    }
}
