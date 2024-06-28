//  _    _ _                _                 _ 
// | |  | (_)              (_)               | |
// | |  | |_ _ __ _____   ___ ___ _   _  __ _| |
// | |/\| | | '__/ _ \ \ / / / __| | | |/ _` | |
// \  /\  / | | |  __/\ V /| \__ \ |_| | (_| | |
//  \/  \/|_|_|  \___| \_/ |_|___/\__,_|\__,_|_|
//    https://git.psi.ch/hipa_apps/Wirevisual
//
// Implements the main window UI and is responsible
// for all proceses
//
//
// @Author: Adam Koprek
// @Maintainer: Jochem Snuverink

#include <QWidget>
#include <QScrollBar>
#include <QMainWindow>
#include <QMessageBox>
#include <QFileDialog>
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <QThread>
#include <cstring>
#include <iomanip>
#include <ios>
#include <iterator>
#include <QListWidget>
#include <qcoreevent.h>
#include <qlist.h>
#include <qlistwidget.h>
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

#include "mainwindow.h"
#include "data_dump.h"
#include "data_fetch.h"
#include "fits.h"
#include "loading_widget.h"
#include "profiles.h"
#include "worker.h"

#define BD_PATH "/hipa/bd/bin/"


/************************************************************
*                       public
************************************************************/

// Constructor
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    m_data_fetch = new DataFetch();
    custom_ui_setup();
    m_data_dump = new DataDump(m_data_fetch);
}

// Deconstructor
MainWindow::~MainWindow() {
    delete m_data_dump;
    delete m_data_fetch;
    delete m_diagram_layout_1;
    delete m_diagram_layout_2;
    delete m_loading_overlay;
}

// Called when an elment from the beamline list is clicked
void MainWindow::on_beamline_clicked(QListWidgetItem* item) {
    std::string selected_item(item->text().toUtf8().constData());
    auto profiles_to_display = &PROFILES.at(selected_item);

    // Add the profile names to the ui
    ui.profile_list->clear();
    for (size_t i = 0; i < profiles_to_display->size(); i++) {
        auto profile_name = profiles_to_display->at(i);
        auto item = new QListWidgetItem(tr(profile_name.c_str()));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);

        // Check if profile is selected and addapt to it
        auto element_index = std::find(m_selected.begin(), m_selected.end(),
                selected_item + "/" + profile_name);
        if (element_index != m_selected.end())
            item->setCheckState(Qt::Checked);
        else
            item->setCheckState(Qt::Unchecked);

        ui.profile_list->insertItem(i, item);
    }
}

// Called when an elment from the beam_line list is checked
void MainWindow::on_beamline_selected(QListWidgetItem* item) {
    ui.beamline_list->setCurrentItem(item);

    if (m_program_uncheck) {
        m_program_uncheck = false;
        return;
    }

    std::string selected_item(item->text().toUtf8().constData());
    auto profiles_to_display = &PROFILES.at(selected_item);
    bool checked = item->checkState() == Qt::Checked;

    // Add every profile of that beamline
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

    // Also switch to that item
    on_beamline_clicked(ui.beamline_list->item(std::distance(PROFILES.begin(), PROFILES.find(selected_item))));
}

// Called when an elment from the profile list is clicked
void MainWindow::on_profile_selected(QListWidgetItem* item) {
    ui.profile_list->setCurrentItem(item);
    QListWidgetItem* current_beamline_item = ui.beamline_list->currentItem();
    std::string current_beamline(current_beamline_item->text().toUtf8().constData());
    std::string selected_item(item->text().toUtf8().constData());

    // Add the profile and its beamlien name to the selected
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

// Called when one of the QScrollWidgets scrolls vertically 
void MainWindow::sync_scroll(int value) {
    if (!ui.scrollTogether->isChecked()) return;
    if (sender() == ui.scrollArea->verticalScrollBar())
        ui.scrollArea_2->verticalScrollBar()->setValue(value);
    else if (sender() == ui.scrollArea_2->verticalScrollBar())
        ui.scrollArea->verticalScrollBar()->setValue(value);
}

// Called when the measure button is clicked
void MainWindow::on_measure_clicked() {
    if (m_busy) return;
    if (!m_selected.size()) return;
    m_to_fetch.clear();
    m_selected_beamlines.clear();

    // Remove the beamline prefix and only append the profiles once
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

    // Measure should always replace everything
    ui.keep->setCheckState(Qt::Unchecked);

    measure();
}

// Called when the cancel button is clicked
// @note only possible in the beginning
void MainWindow::on_cancel_clicked() {
    if (!m_busy) return;
    m_loading_widget->set_text("Canceling");
    m_loading_widget->stop();
    m_data_fetch->cancel();
    m_data_fetch->resume();
}

// Called when the stop button is clicked
void MainWindow::on_stop_clicked() {
    if (!m_busy) return;
    m_loading_widget->set_text("Stopped, tap resume to continue");
    m_loading_widget->stop();
    m_data_fetch->stop();
}

// Called when the resume button is clicked
void MainWindow::on_resume_clicked() {
    if (!m_busy) return;
    m_data_fetch->resume();
    m_loading_widget->set_text("Loading the profiles");
    m_loading_widget->start();
}

// Called when the transport button is clicked
void MainWindow::on_transport_clicked() {
    pid_t pid = fork();

    // Open transport in a child process
    if (pid == 0) {
        char command[128];
        const char* program = "loadtrans";
        strcpy(command, BD_PATH);
        strcat(command, program);
        char* options[2];
        options[0] = command;
        options[1] = NULL;
        execvp(command, options);

        std::cerr << "Failed to open transport";
        exit(1);
    } else {}
}

// Called when the MinT button is clicked
void MainWindow::on_mint_clicked() {
    pid_t pid = fork();

    // Open the last gernerated .mint file of this session
    if (pid == 0) {
        char mint_file[512];
        char* file_folder = getenv("TRANSMESS");
        strcpy(mint_file, file_folder);
        strcat(mint_file, "/");
        strcat(mint_file, m_selected_beamlines[0].c_str());
        strcat(mint_file, "/");
        strcat(mint_file, m_selected_beamlines[0].c_str());
        strcat(mint_file, "_");
        strcat(mint_file, m_data_dump->get_last_date().c_str());
        strcat(mint_file, ".mint");

        std::cout << "MinT file: " << mint_file << std::endl;

        char* options[2];
        options[0] = mint_file;
        options[1] = NULL;
        execvp(mint_file, options);

        std::cerr << "Failed to open MinT" << std::endl;
        exit(1);
    } else {}
}

// Called when the replay button is clicked
void MainWindow::on_replay_clicked() {
    // This function doesn't look for what is selected
    // but takes the old m_to_fetch

    if (m_busy) return;
    if (ui.keep->isChecked() && m_plot_index > 3) return;
    if (m_to_fetch.size() == 0) return;
    measure();
}

// Called when the open button is clicked
void MainWindow::on_open_clicked() {
    if (ui.keep->isChecked() && m_plot_index > 3) return;
    if (ui.keep->isChecked() && m_to_fetch.size() == 0) return;

    // Open file explorer to open a .mes file
    static QString selected_filter = "All Transprof Files (*.mes)";
    QString file_name = QFileDialog::getOpenFileName(this,
                                          "Open Transprof File",
                                          getenv("TRANSMESS"),
                                          "860 (b860*.mes);;"
                                          "BCE (bce*.mes);;"
                                          "BW2 (bw2*.mes);;"
                                          "BX1 (bx1*.mes);;"
                                          "BX2 (bx2*.mes);;"
                                          "IP2 (ip2*.mes);;"
                                          "IW2 (iw2*.mes);;"
                                          "PKANAL (pkan*.mes);;"
                                          "PKANAL & DUMP (pkbhe*.mes);;"
                                          "PKANAL & SINQ (pksinq*.mes);;"
                                          "TARGET E (pktebhe*.mes);;"
                                          "TARGET M (pktm_*.mes);;"
                                          "TARGET E & M (pktmte*.mes);;"
                                          "SINQ (sinq*.mes);;"
                                          "UCN (ucn_*.mes);;"
                                          "All Transprof Files (*.mes)",
                                          &selected_filter);

    if (file_name.isEmpty() == true) return;
    QFileInfo fileInfo(file_name);
    QString file_name_measurement = fileInfo.baseName();
    std::string data_file = file_name.toStdString();
    int status = m_data_fetch->load(data_file);

    if (status != 0) {
        QMessageBox msg(QMessageBox::Warning,"",
                    QString("The file is corrupted"),
                    QMessageBox::NoButton,this);
        msg.exec();
        return;
    }

    // Add to the plots
    FileHeader* header = m_data_fetch->get_file_header();
    m_to_fetch = header->profile_names;
    m_from_file = true;
    if (ui.keep->isChecked()) add_to_diagrams();
    else create_diagrams();
}

// Called when the quad dump button is clicked
void MainWindow::on_quad_dump_clicked() {
    if (m_busy) return;
    if (!m_selected.size()) return;
    m_selected_beamlines.clear();
    for (int i = 0; i < ui.beamline_list->count(); i++) {
        QListWidgetItem* item = ui.beamline_list->item(i);
        if (item->checkState() == Qt::Checked)
            m_selected_beamlines.push_back(item->text().toStdString());
    }
    m_selected.clear();
    reset_beamlines();
    save(true);
    QMessageBox* message_box = new QMessageBox();
    message_box->setText("Quads saved");
    message_box->setStandardButtons(QMessageBox::Ok);
    message_box->exec();
}

// Called to create diagrams after measuring or opening
void MainWindow::create_diagrams() {
    if (!m_from_file) {
        delete m_worker;
        m_work_tread->quit();
        m_work_tread->wait();
        delete m_work_tread;
    }

    if (m_data_fetch->was_canceled()) {
        m_loading_widget->stop();
        m_loading_widget->stop();
        m_loading_overlay->hide();
        m_busy = false;
        return;
    }

    m_loading_widget->set_text("Creating the diagrams");
    m_plot_index = 0;

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

    m_plots.clear();
    m_plot_zoomer.clear();

    std::vector<std::string> failed_profiles;

    for (int i = m_to_fetch.size() - 1; i > -1; i--) {
        DataPoint* point = m_data_fetch->get_data_point(m_to_fetch[i]);

        QwtPlot* plot = new QwtPlot();

        // Only plot the data if valid data otherwise show that corrupted
        if (point->valid_data) {
            plot->setTitle(QString(point->name.c_str()));
            plot->insertLegend(new QwtLegend(), QwtPlot::BottomLegend);
            plot->setAxisAutoScale(QwtPlot::xBottom);
            plot->setAxisAutoScale(QwtPlot::yLeft);

            QwtPlotCurve* curve_real = new QwtPlotCurve("Curve 1");
            curve_real->setSamples(point->x.data(), point->y.data(), static_cast<int>(point->x.size()));
            std::stringstream stream;
            stream << "2σ = " << std::fixed << std::setprecision(1);
            stream << round(point->sigma_4_fit / 2 * 10) / 10;
            stream << ", μ = " << round(point->mean_fit * 10) / 10;
            curve_real->setTitle(QString(stream.str().c_str()));
            curve_real->attach(plot);

            QwtPlotCurve* curve_fit = new QwtPlotCurve("Curve 2");
            curve_fit->setSamples(point->x.data(), point->fit.data(), static_cast<int>(point->x.size()));
            curve_fit->setTitle("Fit");
            curve_fit->setPen(QPen(Qt::red));
            curve_fit->attach(plot);
        }
        else {
            plot->setTitle(QString((point->name + " - Corrupted").c_str()));
            plot->insertLegend(new QwtLegend(), QwtPlot::BottomLegend);
            failed_profiles.push_back(point->name);
        }

        QwtPlotZoomer* zoomer = new QwtPlotZoomer(QwtPlot::xBottom, QwtPlot::yLeft, plot->canvas());
        zoomer->setZoomBase(true);

        plot->setMinimumHeight(this->height() * 0.4);
        plot->setMaximumHeight(this->height() * 0.4);
        plot->replot();
        m_plots.insert({point->name, plot});
        m_plot_zoomer.insert({point->name, zoomer});

        int profile_number = point->name[3] * 10 + point->name[4];
        if (profile_number % 2 == 0)
            m_diagram_layout_1->addWidget(plot);
        else
            m_diagram_layout_2->addWidget(plot);
    }

    ui.legend_2->setText("Fit");
    ui.legend_3->setText("");
    ui.legend_4->setText("");
    ui.legend_5->setText("");
    set_legend(ui.legend_1);
    auto palette = ui.legend_2->palette();
    palette.setColor(ui.legend_2->foregroundRole(), Qt::red);
    ui.legend_2->setPalette(palette);


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
    if (!m_from_file) {
        m_loading_overlay->hide();
        m_loading_widget->stop();
    }
    m_busy = false;
}

// Called to add lienes to existing diagrams
void MainWindow::add_to_diagrams() {
    if (!m_from_file) {
        delete m_worker;
        m_work_tread->quit();
        m_work_tread->wait();
        delete m_work_tread;
    }



    if (m_data_fetch->was_canceled()) {
        m_loading_overlay->hide();
        m_loading_widget->stop();
        m_busy = false;
        return;
    }

    m_loading_widget->set_text("Creating the diagrams");

    // Whwn more than one plot is presented remove the fit
    for (auto const& [profile, plot] : m_plots) {
        if (!m_data_fetch->point_exists(profile)) continue;
        auto point = m_data_fetch->get_data_point(profile);
        // Remove Fit
        auto curves = m_plots.at(profile)->itemList(QwtPlotItem::Rtti_PlotCurve);
        for (QwtPlotItem* item : curves) {
            QwtPlotCurve* curve = dynamic_cast<QwtPlotCurve*>(item);
            if (curve->title().text() == "Fit") {
                curve->detach();
                delete curve;
            }
        }

        QwtPlotCurve* curve_real = new QwtPlotCurve("Curve");
        curve_real->setSamples(point->x.data(), point->y.data(), static_cast<int>(point->x.size()));
        std::stringstream stream;
        stream << "2σ = " << std::fixed << std::setprecision(1);
        stream << round(point->sigma_4_fit / 2 * 10) / 10;
        stream << ", μ = " << round(point->mean_fit * 10) / 10;
        curve_real->setTitle(QString(stream.str().c_str()));
        curve_real->setPen(QPen(m_colors[m_plot_index]));
        curve_real->attach(plot);

        plot->setAxisAutoScale(QwtPlot::xBottom);
        plot->setAxisAutoScale(QwtPlot::yLeft);
        plot->updateAxes();
        plot->replot();
        auto zoomer = m_plot_zoomer.at(profile);
        zoomer->setZoomBase(true);

        plot->replot();
    }

    if  (m_plot_index == 0) {
        set_legend(ui.legend_2);
        auto palette = ui.legend_2->palette();
        palette.setColor(ui.legend_2->foregroundRole(), m_colors[0]);
        ui.legend_2->setPalette(palette);
        palette = ui.legend_2->palette();
    }
    else if (m_plot_index == 1) set_legend(ui.legend_3);
    else if (m_plot_index == 2) set_legend(ui.legend_4);
    else if (m_plot_index == 3) set_legend(ui.legend_5);

    m_plot_index++;
    if (!m_from_file) {
        m_loading_overlay->hide();
        m_loading_widget->stop();
    }
    m_busy = false;
}

/************************************************************
*                       private
************************************************************/

// Setup the ui
void MainWindow::custom_ui_setup() {
    ui.setupUi(this);
    ui.fit_2sigmafit->click();

    // Connect ui elements with the slots
    connect(ui.beamline_list,    &QListWidget::itemClicked, this, &MainWindow::on_beamline_clicked);
    connect(ui.beamline_list,    &QListWidget::itemChanged, this, &MainWindow::on_beamline_selected);
    connect(ui.profile_list,     &QListWidget::itemChanged, this, &MainWindow::on_profile_selected);
    connect(ui.measure_button,   &QPushButton::clicked,     this, &MainWindow::on_measure_clicked);
    connect(ui.transport_button, &QPushButton::clicked,     this, &MainWindow::on_transport_clicked);
    connect(ui.mint_button,      &QPushButton::clicked,     this, &MainWindow::on_mint_clicked);
    connect(ui.remeasure_button, &QPushButton::clicked,     this, &MainWindow::on_replay_clicked);
    connect(ui.open_button,      &QPushButton::clicked,     this, &MainWindow::on_open_clicked);
    connect(ui.quad_button,      &QPushButton::clicked,     this, &MainWindow::on_quad_dump_clicked);
    connect(ui.scrollArea->verticalScrollBar(),     &QScrollBar::valueChanged, this, &MainWindow::sync_scroll);
    connect(ui.scrollArea_2->verticalScrollBar(),   &QScrollBar::valueChanged, this, &MainWindow::sync_scroll);

    // Click on first beamline for the profiles to appear
    on_beamline_clicked(ui.beamline_list->item(0));

    // Create the layout for the diagrams
    m_diagram_layout_1 = new QVBoxLayout();
    m_diagram_layout_2 = new QVBoxLayout();
    ui.scrollAreaWidgetContents->setLayout(m_diagram_layout_1);
    ui.scrollAreaWidgetContents_2->setLayout(m_diagram_layout_2);

    // Set the color for every legend element
    auto palette = ui.legend_1->palette();
    palette.setColor(ui.legend_1->foregroundRole(), Qt::black);
    ui.legend_1->setPalette(palette);
    palette = ui.legend_2->palette();
    palette.setColor(ui.legend_2->foregroundRole(), m_colors[0]);
    ui.legend_2->setPalette(palette);
    palette = ui.legend_3->palette();
    palette.setColor(ui.legend_3->foregroundRole(), m_colors[1]);
    ui.legend_3->setPalette(palette);
    palette = ui.legend_4->palette();
    palette.setColor(ui.legend_4->foregroundRole(), m_colors[2]);
    ui.legend_4->setPalette(palette);
    palette = ui.legend_5->palette();
    palette.setColor(ui.legend_5->foregroundRole(), m_colors[3]);
    ui.legend_5->setPalette(palette);

    create_overlay();
}

// Create the overlay widget with LoadingWidget from loading_widget.h
void MainWindow::create_overlay() {
    // Create a parent
    m_loading_overlay = new QWidget(this);
    m_loading_overlay->setGeometry(this->geometry());
    m_loading_overlay->setStyleSheet("background-color: rgba(0, 0, 0, 0.5)");
    auto loading_layout = new QGridLayout();
    m_loading_overlay->setLayout(loading_layout);

    m_loading_widget = new LoadingWidget(m_data_fetch);
    m_loading_widget->set_text("Loading the profiles");
    loading_layout->addWidget(m_loading_widget);

    // Connect the buttons from the LoadingWidget with the slots
    connect(m_loading_widget, &LoadingWidget::cancel_clicked,   this, &MainWindow::on_cancel_clicked);
    connect(m_loading_widget, &LoadingWidget::stop_clicked,     this, &MainWindow::on_stop_clicked);
    connect(m_loading_widget, &LoadingWidget::resume_clicked,   this, &MainWindow::on_resume_clicked);

    m_loading_overlay->hide();
}

// Implementation of the QMainWindow resize event
void MainWindow::resizeEvent(QResizeEvent* event) {
    // Resize the overlay
    m_loading_overlay->setGeometry(0, 0, this->width(), this->height());

    // Resize the height of the plot so that every plot has the same size
    for (auto const& [profile, plot] : m_plots) {
        plot->setMinimumHeight(this->height() * 0.4);
        plot->setMaximumHeight(this->height() * 0.4);
    }
}

// Setup Worker from worker.h and fetch data with DataFetch from logic/data_fetch.h
void MainWindow::measure() {
    m_from_file = false;
    m_busy = true;
    m_loading_widget->set_text("Loading the profiles");
    m_loading_widget->start();
    m_loading_overlay->show();

    // Create a new thread and worker, move the worker on the thread
    m_work_tread = new QThread();
    m_worker = new Worker([this]{
        m_data_fetch->fetch(m_to_fetch);
        if (!m_data_fetch->was_canceled()) save(false);
    });
    m_worker->moveToThread(m_work_tread);

    // Connect the appropriate on finish signals
    connect(m_work_tread, &QThread::started, m_worker, &Worker::execute_work);
    if (ui.keep->isChecked())
        connect(m_worker, &Worker::work_done, this, &MainWindow::add_to_diagrams, Qt::QueuedConnection);
    else
        connect(m_worker, &Worker::work_done, this, &MainWindow::create_diagrams, Qt::QueuedConnection);
    m_work_tread->start();
}

// Save the fetched data to a file with DataDump from data_dumb.h
void MainWindow::save(bool just_quads) {
    FITS fit;

    if (ui.fit_2sigma->isChecked()) fit = FITS::TWO_SIGMA;
    else if (ui.fit_2sigmareduced->isChecked()) fit = FITS::TWO_SIGMA_RED;
    else if (ui.fit_2sigmafit->isChecked()) fit = FITS::TWO_SIGMA_FIT;
    else if (ui.fit_fwhm->isChecked()) fit = FITS::FWHM;
    else if (ui.fit_fwhm_fit->isChecked()) fit = FITS::FWHM_FIT;

    if (just_quads) m_data_dump->dump_quads(m_selected_beamlines, fit);
    else m_data_dump->dump(m_selected_beamlines, fit);
}


// Set one of the five legend elements based on m_polt_index
void MainWindow::set_legend(QLabel* legend) {
    if (m_from_file) {
        FileHeader* header = m_data_fetch->get_file_header();
        std::string text = header->date + " " + header->time;
        legend->setText(QString(text.c_str()));
    }
    else
        legend->setText(QString(m_data_dump->get_last_human_date().c_str()));
}

// Unselect every beamline
void MainWindow::reset_beamlines() {
    for (size_t i = 0; i < BEAM_LINES.size(); i++) {
        ui.beamline_list->item(i)->setCheckState(Qt::Unchecked);
    }
    on_beamline_clicked(ui.beamline_list->currentItem());
}

// Add a profile to the m_selected vector 
// This functions sorts the data in the same order as on the screen
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
