//  _    _ _                _                 _ 
// | |  | (_)              (_)               | |
// | |  | |_ _ __ _____   ___ ___ _   _  __ _| |
// | |/\| | | '__/ _ \ \ / / / __| | | |/ _` | |
// \  /\  / | | |  __/\ V /| \__ \ |_| | (_| | |
//  \/  \/|_|_|  \___| \_/ |_|___/\__,_|\__,_|_|
//    https://git.psi.ch/hipa_apps/Wirevisual
//
// Loading UI implementation that displayes what
// profiel is being fetched
//
// This class emits signals when the buttons stop, 
// resume and cancel are clicked. It also plays an
// animation stored in the forms directory. When start
// is called a thread is created that checks ever 100ms
// what profile is being fetched and displays the name
//
// @Author: Adam Koprek
// @Maintainer: Jochem Snuverink

#pragma once
#include <QMainWindow>
#include <csignal>
#include <qbuffer.h>
#include <qlistwidget.h>
#include <qobjectdefs.h>
#include <QVBoxLayout>
#include <qthread.h>
#include <qwidget.h>
#include <qwindowdefs.h>
#include <string>
#include <thread>

#include "../../forms/ui_loading.h"
#include "data_fetch.h"


class LoadingWidget : public QWidget 
{
    Q_OBJECT

public:
    /************************************************************
    *                       functions
    ************************************************************/

    // Constructor
    // @param pointer to data_fetch instance
    // @param Widget parent
    LoadingWidget(DataFetch* data_fetch, QWidget *parent = nullptr);

    // Destructor
    ~LoadingWidget();

    // Set the main text to be diplayed in the overlay
    // @param the text
    void set_text(std::string text);

    // Start the animation and updating names
    void start();
    
    // Stop the animation and updating names
    void stop();

public slots:
    /************************************************************
    *                       slots
    ************************************************************/

    // Slot to be called when the signal check_profile is called
    void on_update_profile();

signals:
    /************************************************************
    *                       signals
    ************************************************************/

    // Signal to be emited when the cancel button is clicked
    void cancel_clicked();

    // Signal to be called when the stop button is clicked
    void stop_clicked();
    
    // Signal to be called when the resume button is clicked
    void resume_clicked();

    // Signal to be emitted from thread to signal to check for what profiel is being fetched
    void check_profile();

private:
    /************************************************************
    *                       fields
    ************************************************************/

    Ui::Loading ui;                     // Holds the ui
    DataFetch* m_data_fetch;            // Holds the external DataFetch instance
    QMovie* m_movie;                    // Holds the gif loading animation
    QBuffer* m_buffer;                  // Holds the loaded byte buffer with the gif 
    std::thread m_update_names_thread;  // Holds the thread that checks what profile is being fetched
};

