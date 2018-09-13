#include <iostream>

#include "XVideoThread.h"
#include "XDecoder.h"
#include "IVideoCall.h"

extern "C" {
#include "libavcodec/avcodec.h"
}

XVideoThread::XVideoThread()
{
}

XVideoThread::~XVideoThread()
{
}

bool XVideoThread::open(AVCodecParameters* params, IVideoCall* player)
{
    // if video player is nullptr, that indicates we don't want to display video.
    if (!player) {
        isExit = true; // set flag to make video thread quit.
        return false;
    }

    if (!params) return false;

    // empty AVPacket list at first.
    clear();

    std::lock_guard<std::mutex> locker{vmux};
    videoPlayer = player;
    videoPlayer->init(params->width, params->height);

    // create and open video decoder.
    // Trivial: audio and video have separate decoders.
    if (!decoder) decoder = new XDecoder();
    if (!decoder->open(params)) {
        std::cout << "video decoder open failed!" << std::endl;
        return false;
    }

    return true;
}

void XVideoThread::run()
{
    AVPacket *pkt = nullptr;
    AVFrame *frame = nullptr;

    while (!isExit) {

        if (isPause) {
            msleep(5);
            continue;
        }

        std::unique_lock<std::mutex> locker{vmux};
        if (pktList.empty()) {
            locker.unlock();
            msleep(1);
            continue;
        }

        // get top video packet.
        pkt = pop();

        if (!decoder->send(pkt)) {
            // send failed, and continue to send.
            locker.unlock();
            msleep(1);
            continue;
        }

        // receive a frame, one send() may exist multiple recv() operations.
        while (!isExit) {
            frame = decoder->recv();
            if (!frame) break;

            pts = frame->pts;
            std::cout << "video pts: " << frame->pts << std::endl;

            // draw video picture.
            if (videoPlayer) videoPlayer->repaint(frame);
        }
    }
}

void XVideoThread::close() {
    isExit = true;
    wait();

    XDecodeThread::close();
}

int64_t XVideoThread::getPts() {
    std::lock_guard<std::mutex> locker{vmux};
    return pts;
}

void XVideoThread::setPts(int64_t pts_) {
    std::lock_guard<std::mutex> locker{vmux};
    pts = pts_;
}

void XVideoThread::setPause(bool isPause_) {
    isPause = isPause_;
}
