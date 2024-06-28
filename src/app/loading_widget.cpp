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

#include <chrono>
#include <qbuffer.h>
#include <qwidget.h>
#include <QMovie>
#include <string>
#include <QBuffer>
#include <thread>

#include "loading_widget.h"
#include "data_fetch.h"
#include "loading_gif.h"


/************************************************************
*                       public
************************************************************/

// Constructor
LoadingWidget::LoadingWidget(DataFetch* data_fetch, QWidget* parent) : QWidget(parent) {
    ui.setupUi(this);

    m_data_fetch = data_fetch;

    // Load the gif from the header buffer and convert it to a QMovie
    m_buffer = new QBuffer();
    m_buffer->setData((char*)loading_gif, sizeof(loading_gif));
    m_buffer->open(QIODevice::ReadOnly);
    m_movie = new QMovie(m_buffer, "GIF");
    ui.animation->setMovie(m_movie);
     
    // Connect the button events 
    connect(ui.cancelButton,    &QPushButton::clicked, [this] {emit cancel_clicked();   });
    connect(ui.stopButton,      &QPushButton::clicked, [this] {emit stop_clicked();     });
    connect(ui.resumeButton,    &QPushButton::clicked, [this] {emit resume_clicked();   });

    // Connect the thread save implementation to update names of the profiles
    connect(this, &LoadingWidget::check_profile, this, &LoadingWidget::on_update_profile, Qt::QueuedConnection);
}

// Destructor
LoadingWidget::~LoadingWidget() {
    delete m_movie;
    delete m_buffer;
}

// Set the main text to be diplayed in the overlay
void LoadingWidget::set_text(std::string text) {
    ui.status_text->setText(QString(text.c_str()));
}

// Start the animation and updating names
void LoadingWidget::start() {
    m_movie->start();

    // Start the thread that updates the profile names
    m_update_names_thread = std::thread([this]{
        // Dont finish until the data_fetch hasn't finished
        while (!m_data_fetch->data_ready && !m_data_fetch->was_canceled()) {
            // Emit the signal every 100ms to updat the ui in the main thread
            emit check_profile();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
}

// Stop the animation and updating names
void LoadingWidget::stop() {
    m_movie->stop();
    m_update_names_thread.join();
}

/************************************************************
*                       slots
************************************************************/

// Slot to be called the signal check_profile is called
void LoadingWidget::on_update_profile() {
    std::string text = "Fetching profile " + m_data_fetch->get_current_profile(); 
    set_text(text);
}
