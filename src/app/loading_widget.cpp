#include "loading_widget.h"
#include <qbuffer.h>
#include <qwidget.h>
#include <QMovie>
#include <string>
#include "loading_gif.h"
#include <QBuffer>


LoadingWidget::LoadingWidget(QWidget* parent) : QWidget(parent) {
    ui.setupUi(this);

    m_buffer = new QBuffer();
    m_buffer->setData((char*)loading_gif, sizeof(loading_gif));
    m_buffer->open(QIODevice::ReadOnly);
    m_movie = new QMovie(m_buffer, "GIF");
    ui.animation->setMovie(m_movie);
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
}

void LoadingWidget::stop() {
    m_movie->stop();
}
