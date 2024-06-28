#include "loading_widget.h"
#include <chrono>
#include <qbuffer.h>
#include <qwidget.h>
#include <QMovie>
#include <string>
#include "data_fetch.h"
#include "loading_gif.h"
#include <QBuffer>
#include <thread>


LoadingWidget::LoadingWidget(DataFetch* data_fetch, QWidget* parent) : QWidget(parent) {
    ui.setupUi(this);

    m_data_fetch = data_fetch;

    m_buffer = new QBuffer();
    m_buffer->setData((char*)loading_gif, sizeof(loading_gif));
    m_buffer->open(QIODevice::ReadOnly);
    m_movie = new QMovie(m_buffer, "GIF");
    ui.animation->setMovie(m_movie);
     
    connect(ui.cancelButton,    &QPushButton::clicked, [this] {emit cancel_clicked();   });
    connect(ui.stopButton,      &QPushButton::clicked, [this] {emit stop_clicked();     });
    connect(ui.resumeButton,    &QPushButton::clicked, [this] {emit resume_clicked();   });
    connect(this, &LoadingWidget::check_profile, this, &LoadingWidget::on_update_profile, Qt::QueuedConnection);
}

LoadingWidget::~LoadingWidget() {
    delete m_movie;
    delete m_buffer;
}

void LoadingWidget::set_text(std::string text) {
    ui.status_text->setText(QString(text.c_str()));
}

void LoadingWidget::start() {
    m_movie->start();
    m_update_names_thread = std::thread([this]{
        while (!m_data_fetch->data_ready && !m_data_fetch->was_canceled()) {
            emit check_profile();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
}

void LoadingWidget::stop() {
    m_movie->stop();
    m_update_names_thread.join();
}

void LoadingWidget::on_update_profile() {
    std::string text = "Fetching profile " + m_data_fetch->get_current_profile(); 
    set_text(text);
}
