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


#define BD_PATH "/hipa/bd/bin/"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void on_beamline_clicked(QListWidgetItem* item);
    void on_beamline_selected(QListWidgetItem* item);
    void on_profile_selected(QListWidgetItem* item);

    void sync_scroll(int value);

    void on_measure_clicked();
    void on_cancel_clicked();
    void on_stop_clicked();
    void on_resume_clicked();
    void on_transport_clicked();
    void on_mint_clicked();
    void on_replay_clicked();
    void on_open_clicked();
    void on_quad_dump_clicked();

    void create_diagrams();
    void add_to_diagrams();

private:
    void custom_ui_setup();
    void create_overlay();
    void resizeEvent(QResizeEvent* event);
    void measure();
    void reset_beamlines();
    void add_profile(std::string beam_line, std::string profile);
    void save(bool just_quads);
    void set_legend(QLabel* legend);
    
    Ui::MainWindow ui;
    DataFetch* m_data_fetch;
    std::vector<std::string> m_selected;
    std::vector<std::string> m_to_fetch;
    std::vector<std::string> m_selected_beamlines;
    bool m_program_uncheck = false;
    bool m_busy = false;
    bool m_from_file = false;
    QVBoxLayout* m_diagram_layout_1;
    QVBoxLayout* m_diagram_layout_2;
    QThread* m_work_tread = nullptr;
    Worker* m_worker;
    QWidget* m_loading_overlay;
    LoadingWidget* m_loading_widget;
    DataDump* m_data_dump;
    int m_plot_index = 0;
    std::map<std::string, QwtPlot*> m_plots;
    std::map<std::string, QwtPlotZoomer*> m_plot_zoomer;
    std::vector<Qt::GlobalColor> m_colors = {Qt::darkGreen, Qt::blue, Qt::darkMagenta, Qt::darkCyan};
};

