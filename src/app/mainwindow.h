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

#pragma once
#include <QMainWindow>
#include <map>
#include <qlistwidget.h>
#include <qnamespace.h>
#include <qobjectdefs.h>
#include <QVBoxLayout>
#include <qthread.h>
#include <qwidget.h>
#include <string>
#include <vector>

#include "data_dump.h"
#include "data_fetch.h"
#include "../../forms/ui_mainwindow.h"
#include "loading_widget.h"
#include "qwt_plot.h"
#include "qwt_plot_zoomer.h"
#include "worker.h"


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /************************************************************
    *                       functions
    ************************************************************/

    // Constructor
    // @param Widget parent
    MainWindow(QWidget *parent = nullptr);

    // Deconstructor
    ~MainWindow();

public slots:
    /************************************************************
    *                       slots
    ************************************************************/

    // Called when an elment from the beamline list is clicked
    // @param the item thas is clicked
    void on_beamline_clicked(QListWidgetItem* item);

    // Called when an elment from the beam_line list is checked
    // @param the item thas is clicked
    void on_beamline_selected(QListWidgetItem* item);

    // Called when an elment from the profile list is clicked
    // @param the item thas is clicked
    void on_profile_selected(QListWidgetItem* item);

    // Called when one of the QScrollWidgets scrolls vertically 
    void sync_scroll(int value);

    // Called when the measure button is clicked
    void on_measure_clicked();
    
    // Called when the cancel button is clicked
    void on_cancel_clicked();

    // Called when the stop button is clicked
    void on_stop_clicked();
    
    // Called when the resume button is clicked
    void on_resume_clicked();
    
    // Called when the transport button is clicked
    void on_transport_clicked();
    
    // Called when the MinT button is clicked
    void on_mint_clicked();
    
    // Called when the replay button is clicked
    void on_replay_clicked();
    
    // Called when the open button is clicked
    void on_open_clicked();
    
    // Called when the quad dump button is clicked
    void on_quad_dump_clicked();
    
    // Called to create diagrams after measuring or opening
    void create_diagrams();
    
    // Called to add lienes to existing diagrams
    void add_to_diagrams();

private:
    /************************************************************
    *                       functions
    ************************************************************/

    // Setup the ui
    void custom_ui_setup();

    // Create the overlay widget with LoadingWidget from loading_widget.h
    void create_overlay();

    // Implementation of the QMainWindow resize event
    // @param the event informations
    void resizeEvent(QResizeEvent* event);

    // Setup Worker from worker.h and fetch data with DataFetch from logic/data_fetch.h
    void measure();

    // Save the fetched data to a file with DataDump from data_dumb.h
    // @param if true only data of the quads will be saved
    void save(bool just_quads);

    // Set one of the five legend elements based on m_polt_index
    // @param what legend item to change
    void set_legend(QLabel* legend);

    // Unselect every beamline
    void reset_beamlines();

    // Add a profile to the m_selected vector 
    // @param the beamline of the clicked profile
    // @param the clicked profile
    void add_profile(std::string beam_line, std::string profile);

    /************************************************************
    *                       fields
    ************************************************************/
    Ui::MainWindow ui;                      // Holds the ui
    DataFetch* m_data_fetch;                // Internal instance of DataFetch from logic/data_fetch.h
    Worker* m_worker;                       // Internal instance of Worker from worker.h
    LoadingWidget* m_loading_widget;        // Internal instance of LoadingWidget from loading_widget.h
    DataDump* m_data_dump;                  // Internal instance of DataDump from logic/data_dump.h

    QWidget* m_loading_overlay;             // Internal instance of the parent of m_loading_widget
    QVBoxLayout* m_diagram_layout_1;        // The vertical lyout for the first row of diagrams
    QVBoxLayout* m_diagram_layout_2;        // The vertical lyout for the second row of diagrams
    QThread* m_work_tread = nullptr;        // Holds the pointer to the work thread
    std::map<std::string, QwtPlot*> m_plots;    // Vector with the pointer to the plot
    std::map<std::string, QwtPlotZoomer*> m_plot_zoomer;    // Vector wieht the pointers to the zoomers of the pliot
    // Vector with the colors for the lines stacked above each other                                                         
    std::vector<Qt::GlobalColor> m_colors = {Qt::darkGreen, Qt::blue, Qt::darkMagenta, Qt::darkCyan};
                                            
    std::vector<std::string> m_selected;    // stores selected profiles in form <beamline>/<profile>
    std::vector<std::string> m_to_fetch;    // Stores just the profile names to be fetched
    std::vector<std::string> m_selected_beamlines;  // Stores all the beamlines where at least one lement is selected
    bool m_program_uncheck = false;         // 
    bool m_busy = false;                    // Flag to set if already one fetching action si executed
    bool m_from_file = false;               // Flasg is set when data points come from file
    int m_plot_index = 0;                   // Sets how many data lines are on the screen except for fit
};
