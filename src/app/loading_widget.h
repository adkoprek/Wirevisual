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
    LoadingWidget(DataFetch* data_fetch, QWidget *parent = nullptr);
    ~LoadingWidget();
    void set_text(std::string text);
    void start();
    void stop();

public slots:
    void on_update_profile();

signals:
    void cancel_clicked();
    void stop_clicked();
    void resume_clicked();
    void check_profile();

private:
    Ui::Loading ui;
    DataFetch* m_data_fetch;
    QMovie* m_movie;
    QBuffer* m_buffer;
    std::thread m_update_names_thread;
};

