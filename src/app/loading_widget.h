#pragma once
#include <QMainWindow>
#include <qbuffer.h>
#include <qlistwidget.h>
#include <qobjectdefs.h>
#include <QVBoxLayout>
#include <qthread.h>
#include <qwidget.h>
#include <qwindowdefs.h>
#include <string>
#include "../../forms/ui_loading.h"


class LoadingWidget : public QWidget 
{
    Q_OBJECT

public:
    LoadingWidget(QWidget *parent = nullptr);
    ~LoadingWidget();
    void set_text(std::string text);
    void start();
    void stop();

private:
    Ui::Loading ui;
    QMovie* m_movie;
    QBuffer* m_buffer;
};

