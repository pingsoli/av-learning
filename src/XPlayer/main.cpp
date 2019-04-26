#include <string>
#include <iostream>
#include <thread>

#include <QtWidgets/QApplication>
#include <QtConcurrent/QtConcurrent>
#include <QThread>

#if (_WIN32 || _WIN64)
// prompt saving file as unicode format, only in windows platform.
#pragma warning(disable : 4819)
#endif

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swresample.lib") // audio resample

#include "XPlayer.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    XPlayer w;
    w.show();

    std::string url("");
    //url = "cut.mp4"; // short video. (10 seconds)
    //url = "test.mp4"; // long video.
    //url = "rtsp://184.72.239.149/vod/mp4:BigBuckBunny_175k.mov"; // valid rtsp stream url without subtitle.
    //url = "rtmp://live.hkstv.hk.lxdns.com/live/hks"; // valid rtsp stream with subtitle.
    //url = "rtsp://example.com/test"; // invalid rtsp url used for testing timeout behavior.

    w.open(url);

    return a.exec(); 
}
