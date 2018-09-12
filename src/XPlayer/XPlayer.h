#pragma once

#include <iostream>

#include <QtWidgets/QWidget>
#include <QTimerEvent>
#include <QResizeEvent>
#include "ui_XPlayer.h"

// where to release the UI components ?
// video (QOpenGLWidget)
// lineEdit, pushButton, or slider.
// they are pointers, but they are not released by myself.

class XPlayer : public QWidget
{
    Q_OBJECT

public:
    XPlayer(QWidget *parent = Q_NULLPTR);

    // close window and the destructor will be called.
    virtual ~XPlayer();

    // for progress bar dispaly, refresh display per 40ms.
    void timerEvent(QTimerEvent *te) override;

    // resize window.
    void resizeEvent(QResizeEvent* re) override;

    // double click set or quit full screen window.
    void mouseDoubleClickEvent(QMouseEvent* me) override;

    // set text for push button. 'play' or 'pause'.
    void setPause(int isPause);

    // test interface for outside.
    void open(const std::string& url);

public slots:
    // click button, choose the file we want to open.
    void openFile();

    // enter the uri, hit `Enter` key.
    void openUri();

    // play button toggle.
    void playOrPause();

    // slider bar, support drag progress bar button.
    void sliderPressed();
    void sliderReleased();
    
private:
    // open a uri, and start demux thread.
    void start(const QString& uri);

    // default save to current directory.
    void logToFile(const std::string& filename);

    // log to console.
    void logToConsole();

    // print log to console or file, they save the streambuf.
    std::streambuf *cout_streambuf = nullptr;
    std::streambuf *file_streambuf = nullptr;
    std::streambuf *cerr_streambuf = nullptr;

    Ui::XPlayerClass ui;
    bool isPause = false;
    bool isSliderPressed = false;

    bool enable_audio = true;
    bool enable_video = true;
};