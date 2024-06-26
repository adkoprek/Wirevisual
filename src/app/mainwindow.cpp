#include "mainwindow.h"
#include "data_dump.h"
#include "data_fetch.h"
#include "fits.h"
#include "loading_widget.h"
#include "profiles.h"
#include "worker.h"
#include <QWidget>
#include <QScrollBar>
#include <QMainWindow>
#include <QMessageBox>
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <QThread>
#include <cstring>
#include <iterator>
#include <QListWidget>
#include <qcoreevent.h>
#include <qlist.h>
#include <qmessagebox.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qwidget.h>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>
#include <qwt_plot_zoomer.h>


MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    custom_ui_setup();
    m_data_fetch = new DataFetch();
    m_data_dump = new DataDump(m_data_fetch);
}

MainWindow::~MainWindow() {
    delete m_data_dump;
    delete m_data_fetch;
    delete m_diagram_layout_1;
    delete m_diagram_layout_2;
    delete m_loading_overlay;
}

void MainWindow::custom_ui_setup() {
    ui.setupUi(this);                       
    ui.fit_2sigmafit->click();              
    connect(ui.beamline_list, &QListWidget::itemClicked, this, &MainWindow::on_beamline_clicked);
    connect(ui.beamline_list, &QListWidget::itemChanged, this, &MainWindow::on_beamline_selected);
    connect(ui.profile_list, &QListWidget::itemClicked, this, &MainWindow::on_profile_clicked);
    connect(ui.profile_list, &QListWidget::itemChanged, this, &MainWindow::on_profile_selected);
    connect(ui.measure_button, &QPushButton::clicked, this, &MainWindow::on_measure_clicked);
    connect(ui.transport_button, &QPushButton::clicked, this, &MainWindow::on_transport_clicked);
    connect(ui.mint_button, &QPushButton::clicked, this, &MainWindow::on_mint_clicked);
    connect(ui.replay_button, &QPushButton::clicked, this, &MainWindow::on_replay_clicked);
    connect(ui.scrollArea->verticalScrollBar(), &QScrollBar::valueChanged, this, &MainWindow::sync_scroll);
    connect(ui.scrollArea_2->verticalScrollBar(), &QScrollBar::valueChanged, this, &MainWindow::sync_scroll);

    on_beamline_clicked(ui.beamline_list->item(0));

    m_diagram_layout_1 = new QVBoxLayout();
    m_diagram_layout_2 = new QVBoxLayout();
    ui.scrollAreaWidgetContents->setLayout(m_diagram_layout_1);
    ui.scrollAreaWidgetContents_2->setLayout(m_diagram_layout_2);


    create_overlay();
}

void MainWindow::create_overlay() {
    m_loading_overlay = new QWidget(this);
    m_loading_overlay->setGeometry(this->geometry());
    m_loading_overlay->setStyleSheet("background-color: rgba(0, 0, 0, 0.5)");
    auto loading_layout = new QGridLayout();
    m_loading_overlay->setLayout(loading_layout);

    m_loading_widget = new LoadingWidget(); 
    m_loading_widget->set_text("Loading the profiles");
    m_loading_widget->start();
    loading_layout->addWidget(m_loading_widget);
    
    connect(m_loading_widget, &LoadingWidget::cancel_clicked, this, &MainWindow::on_cancel_clicked);
    connect(m_loading_widget, &LoadingWidget::stop_clicked, this, &MainWindow::on_stop_clicked);
    connect(m_loading_widget, &LoadingWidget::resume_clicked, this, &MainWindow::on_resume_clicked);

    m_loading_overlay->hide();
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    m_loading_overlay->setGeometry(0, 0, this->width(), this->height());
}

void MainWindow::on_beamline_clicked(QListWidgetItem* item) {
    std::string selected_item(item->text().toUtf8().constData());
    auto profiles_to_display = &PROFILES.at(selected_item);
    
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
    auto profiles_to_display = &PROFILES.at(selected_item);
    bool checked = item->checkState() == Qt::Checked;
    
    for (size_t i = 0; i < profiles_to_display->size(); i++) {
        auto profile_name = profiles_to_display->at(i);
        auto element_index = std::find(m_selected.begin(), m_selected.end(), 
                selected_item + "/" + profile_name);
        if (element_index == m_selected.end()) {
            if (checked)
                add_profile(selected_item, profile_name);
        }
        else {
            if (!checked) 
                m_selected.erase(element_index);
        }
        
    }

    on_beamline_clicked(ui.beamline_list->item(std::distance(PROFILES.begin(), PROFILES.find(selected_item))));
}

void MainWindow::on_profile_clicked(QListWidgetItem* item) {
    on_profile_selected(item);
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
        add_profile(current_beamline, selected_item);

    if (item->checkState() == Qt::Unchecked) {
        m_program_uncheck = true;
        current_beamline_item->setCheckState(Qt::Unchecked);
    }
}

void MainWindow::sync_scroll(int value) {
    if (!ui.scrollTogether->isChecked()) return;
    if (sender() == ui.scrollArea->verticalScrollBar()) 
        ui.scrollArea_2->verticalScrollBar()->setValue(value);
    else if (sender() == ui.scrollArea_2->verticalScrollBar()) 
        ui.scrollArea->verticalScrollBar()->setValue(value);
}

void MainWindow::on_measure_clicked() {
    if (m_busy) return;
    if (!m_selected.size()) return;
    m_to_fetch.clear();
    m_selected_beamlines.clear();
    for (int i = 0; i < m_selected.size(); i++) {
        std::stringstream ss(m_selected[i]);
        std::string part;
        int count = 0;
        while (getline(ss, part, '/')) {
            if (count == 0) {
                auto index = std::find(m_selected_beamlines.begin(), m_selected_beamlines.end(), part);
                if (index == m_selected_beamlines.end())
                    m_selected_beamlines.push_back(part);
            }
            if (count == 1) {
                auto index = std::find(m_to_fetch.begin(), m_to_fetch.end(), part);
                if (index == m_to_fetch.end()) {
                    m_to_fetch.push_back(part);
                }
            }
            count++;
        }
    }
    m_selected.clear();
    reset_beamlines();

    measure();
}

void MainWindow::on_cancel_clicked() {
    if (!m_busy) return;
    m_loading_widget->set_text("Canceling");
    m_loading_widget->stop();
    m_data_fetch->cancel();
    m_data_fetch->resume();
}

void MainWindow::on_stop_clicked() {
    if (!m_busy) return;
    m_loading_widget->set_text("Stoped, tap resume to continue");
    m_loading_widget->stop();
    m_data_fetch->stop();
}

void MainWindow::on_resume_clicked() {
    if (!m_busy) return;
    m_data_fetch->resume();
    m_loading_widget->set_text("Loading the profiles");
    m_loading_widget->start();
}

void MainWindow::on_transport_clicked() {
    pid_t pid = fork();

    if (pid == 0) {
        char command[128];
        const char* program = "loadtrans";
        strcpy(command, BD_PATH);
        strcpy(command, program);
        char* options[2];
        options[0] = command;
        options[1] = NULL;
        execvp(command, options);

        std::cerr << "Failed to open transport";
        exit(1);
    } else {}
} 

void MainWindow::on_mint_clicked() {
    pid_t pid = fork();

    if (pid == 0) {
        char mint_file[512];
        char* file_folder = getenv("TRANSMESS");
        strcpy(mint_file, file_folder);
        strcpy(mint_file, "/");
        strcpy(mint_file, m_selected_beamlines[0].c_str());
        strcpy(mint_file, "/");
        strcpy(mint_file, m_selected_beamlines[0].c_str());
        strcpy(mint_file, "_");
        strcpy(mint_file, m_data_dump->get_last_data().c_str());
        strcpy(mint_file, ".mint");

        char* options[2];
        options[0] = mint_file;
        options[1] = NULL;
        execvp(mint_file, options);

        std::cerr << "Failed to open MinT";
        exit(1);
    } else {}
}

void MainWindow::on_replay_clicked() {
    if (m_busy) return;
    measure();
}

void MainWindow::measure() {
    m_busy = true;
    m_loading_widget->set_text("Loading the profiles");
    m_loading_widget->start();
    m_loading_overlay->show();

    m_work_tread = new QThread();
    m_worker = new Worker([this]{ 
        m_data_fetch->fetch(m_to_fetch); 
        if (!m_data_fetch->was_canceled()) save();
    });
    m_worker->moveToThread(m_work_tread);
    connect(m_work_tread, &QThread::started, m_worker, &Worker::execute_work);
    connect(m_worker, &Worker::work_done, this, &MainWindow::create_diagrams, Qt::QueuedConnection);
    m_work_tread->start();
}

void MainWindow::save() {
    FITS fit;

    if (ui.fit_2sigma->isChecked()) fit = FITS::TWO_SIGMA;
    else if (ui.fit_2sigmareduced->isChecked()) fit = FITS::TWO_SIGMA_RED;
    else if (ui.fit_2sigmafit->isChecked()) fit = FITS::TWO_SIGMA_FIT;
    else if (ui.fit_fwhm->isChecked()) fit = FITS::FWHM;
    else if (ui.fit_fwhm_fit->isChecked()) fit = FITS::FWHM_FIT;

    m_data_dump->dump(m_selected_beamlines, fit);
}

void MainWindow::create_diagrams() {
    delete m_worker;
    m_work_tread->quit();
    m_work_tread->wait();
    delete m_work_tread;


    if (m_data_fetch->was_canceled()) {
        m_loading_overlay->hide();
        m_busy = false;
        return;
    }

    m_loading_widget->set_text("Createing the diagrams");

    while (auto item = m_diagram_layout_1->takeAt(0)) {
        auto widget = item->widget();
        widget->setParent(nullptr);
        delete widget;
        delete item;
    }

    while (auto item = m_diagram_layout_2->takeAt(0)) {
        auto widget = item->widget();
        widget->setParent(nullptr);
        delete widget;
        delete item;
    }

    std::vector<std::string> failed_profiles;

    for (int i = m_to_fetch.size() - 1; i > -1; i--) {
        DataPoint* point = m_data_fetch->get_data_point(m_to_fetch[i]);

        QwtPlot* plot = new QwtPlot();

        if (point->valid_data) {
            plot->setTitle(QString(point->name.c_str()));
            plot->insertLegend(new QwtLegend(), QwtPlot::BottomLegend);

            QwtPlotCurve* curve_real = new QwtPlotCurve("Curve 1");
            curve_real->setSamples(point->x.data(), point->y.data(), static_cast<int>(point->x.size()));
            curve_real->setTitle("Measured Data");
            curve_real->attach(plot);

            QwtPlotCurve* curve_fit = new QwtPlotCurve("Curve 2");
            curve_fit->setSamples(point->x.data(), point->fit.data(), static_cast<int>(point->x.size()));
            curve_fit->setTitle("Fitted Data");
            curve_fit->setPen(QPen(Qt::red));
            curve_fit->attach(plot);
        }
        else {
            plot->setTitle(QString((point->name + " - Corrupted").c_str()));
            failed_profiles.push_back(point->name);
        } 

        QwtPlotZoomer* zoomer = new QwtPlotZoomer(plot->canvas());

        plot->setMinimumHeight(this->height() * 0.4);
        plot->setMaximumHeight(this->height() * 0.4);
        plot->replot();

        int profile_number = point->name[3] * 10 + point->name[4];
        if (profile_number % 2 == 0) 
            m_diagram_layout_1->addWidget(plot);
        else
            m_diagram_layout_2->addWidget(plot);
    }

    if (failed_profiles.size() != 0) {
        std::string message = "The program faild to get the profile";
        if (failed_profiles.size() > 1) message += "s";
        message += ": \n\n";
        for (int i = 0; i < (failed_profiles.size() - 1); i++) {
            message += failed_profiles[i] + "\n";
        }
        message += failed_profiles[failed_profiles.size() - 1];
        QMessageBox* message_box = new QMessageBox();
        message_box->setText(QString(message.c_str()));
        message_box->setStandardButtons(QMessageBox::Ok);
        m_loading_overlay->hide();
        message_box->exec();
    }
    m_loading_overlay->hide();
    m_busy = false;
}

void MainWindow::reset_beamlines() {
    for (size_t i = 0; i < BEAM_LINES.size(); i++) {
        ui.beamline_list->item(i)->setCheckState(Qt::Unchecked);
    }
    on_beamline_clicked(ui.beamline_list->currentItem());
}

void MainWindow::add_profile(std::string beam_line, std::string profile) {
    if (m_selected.size() == 0) {
        m_selected.push_back(beam_line + "/" + profile);
        return;
    }

    int index = 0;
    std::string current_beamline;
    auto index_of_selected = std::find(BEAM_LINES.begin(), BEAM_LINES.end(), beam_line);
    for (int i = 0; i < m_selected.size(); i++) {
        std::stringstream ss(m_selected[i]);
        getline(ss, current_beamline, '/');
        auto index_of_current = std::find(BEAM_LINES.begin(), BEAM_LINES.end(), current_beamline);

        if (index_of_selected <= index_of_current) break;
        index++;
    }


    if (index < m_selected.size()) {
        bool ca_excec = true; 

        if (index < (m_selected.size() - 1)) {
            std::stringstream ss(m_selected[index + 1]);
            std::string next_beamline;
            getline(ss, next_beamline, '/');
            if (next_beamline == current_beamline) ca_excec = false;
        }
        if (index == 0 && current_beamline != beam_line) ca_excec = false;

        if (ca_excec) {
            auto current_profiles = PROFILES.at(current_beamline);
            std::string current_profile;
            index_of_selected = std::find(current_profiles.begin(), current_profiles.end(), profile);
            for (int i = 0; i < m_selected.size(); i++) {
                std::stringstream ss(m_selected[i]);
                int count = 0;
                while (getline(ss, current_profile, '/')) {
                    if (count == 1) break; 
                    count++;
                }
                auto index_of_current = std::find(current_profiles.begin(), current_profiles.end(), current_profile);

                if (index_of_selected <= index_of_current) break;
                index++;
            }
        }
    }

    m_selected.insert(m_selected.begin() + index, beam_line + "/" + profile);
}
