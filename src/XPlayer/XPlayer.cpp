#include <fstream>
#include <iostream>
#include <string>

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

#include "XPlayer.h"
#include "XDemuxThread.h"

static XDemuxThread demuxThread;
static std::ofstream logFile;

XPlayer::XPlayer(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    std::string url("");
    //url = "cut.mp4"; // short video. (10 seconds)
    //url = "test.mp4"; // long video.
    url = "rtsp://184.72.239.149/vod/mp4:BigBuckBunny_175k.mov"; // valid rtsp stream url without subtitle.
    //url = "rtmp://live.hkstv.hk.lxdns.com/live/hks"; // valid rtsp stream with subtitle.
    //url = "rtsp://example.com/test"; // invalid rtsp url used for testing timeout behavior.

    ui.urlLineEdit->setPlaceholderText("url address bar");
    ui.urlLineEdit->setText(QString::fromStdString(url));

    // make the place of progress bar more accurate.
    ui.slider->setMaximum(1000);

    // wrong try.
    //connect(ui.urlLineEdit, SIGNAL(QLineEdit::returnPressed()), this, SLOT(openUri(const std::string&)));

    // set fixed size, though cannot change the window size, but you can also set full screen mode.
    setFixedSize(size());

    // triggered every 40 ms for progress bar displaying.
    startTimer(40);
}

XPlayer::~XPlayer() {
    demuxThread.close();
}

void XPlayer::timerEvent(QTimerEvent *te) {
    if (isSliderPressed) return;
    int64_t total = demuxThread.getTotalMs();
    if (total) {
        double pos = (double) demuxThread.getPts() / (double) total;
        //std::cout << "pos: " << pos << std::endl;
        ui.slider->setValue(ui.slider->maximum() * pos);
    }
}

void XPlayer::resizeEvent(QResizeEvent *re) {
    ui.video->resize(this->size());
}

void XPlayer::mouseDoubleClickEvent(QMouseEvent * me)
{
    if (isFullScreen()) {
        showNormal();
        ui.slider->showNormal();
    } else {
        showFullScreen();
        // hide slider in full screen mode.
        ui.slider->setHidden(true);
    }
}

void XPlayer::setPause(int isPause) {
    ui.playBtn->setText(isPause ? "Play" : "Pause");
}

void XPlayer::open(const std::string & url) {
    start(QString::fromStdString(url));
}

void XPlayer::start(const QString& uri)
{
    if (uri.isEmpty()) return;
    this->setWindowTitle(uri);

    // print standard output and error output to a file.
    // NOTE: cannot output to console and file at same time, you must choose one.
    //logToFile("log.txt");
    //logToConsole();

    if (!demuxThread.open(uri.toLocal8Bit().data(), ui.video)) // play video and audio
    //if (!demuxThread.open(uri.toLocal8Bit().data(), nullptr)) // only play audio
    {
        QMessageBox::information(nullptr, "ERROR", "open file failed!");
        return;
    }

    demuxThread.start();
    setPause(isPause); // set push buttion display text.
}

void XPlayer::openFile() {
    QString filename = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("select video file"));
    start(filename);
}

void XPlayer::openUri() {
    start(ui.urlLineEdit->text());
}

void XPlayer::playOrPause() {
    isPause = !isPause;
    setPause(isPause); // change push button text.
    demuxThread.setPause(isPause);
}

void XPlayer::sliderPressed() {
    isSliderPressed = true;
}

void XPlayer::sliderReleased() {
    isSliderPressed = false;
    // calculate the ratio.
    double pos = (double) ui.slider->value() / (double) ui.slider->maximum();
    demuxThread.seek(pos);
}

void XPlayer::logToFile(const std::string& filename) {
    logFile.open(filename, std::ios::out | std::ios::trunc);
    file_streambuf = logFile.rdbuf();
    cout_streambuf = std::cout.rdbuf(file_streambuf); // save old streambuf and redirect.
    cerr_streambuf = std::cerr.rdbuf(file_streambuf);
}

void XPlayer::logToConsole() {
    std::cout.rdbuf(cout_streambuf);
    std::cerr.rdbuf(cerr_streambuf);
}