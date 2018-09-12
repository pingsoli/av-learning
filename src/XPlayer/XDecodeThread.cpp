#include <iostream>
#include "XDecodeThread.h"
#include "XDecoder.h"

extern "C" {
#include "libavcodec/avcodec.h"
}

#pragma comment(lib, "avcodec.lib")

XDecodeThread::XDecodeThread()
    : isExit(false)
{
}

XDecodeThread::~XDecodeThread()
{
}

void XDecodeThread::push(AVPacket *pkt)
{
    if (!pkt) return;

    while (!isExit) {
        std::unique_lock<std::mutex> locker(mux);
        if (pktList.size() < max_size) {
            pktList.push_back(pkt);
            locker.unlock();
            break;
        }
        locker.unlock();

        // If pause, then discard the AVPacket. the AVPacket list may be
        // full, current thread enter endless loop to wait for free space.
        // on higher level, other thread won't get the ownershiop of the mutex.
        if (isPause) break;

        msleep(1);
    }
}

AVPacket* XDecodeThread::pop()
{
    std::lock_guard<std::mutex> locker{mux};
    AVPacket *pkt = pktList.front();
    pktList.pop_front();
    return pkt;
}

void XDecodeThread::close()
{
    // empty the packet list.
    clear();

    std::lock_guard<std::mutex> locker{mux};
    // close AVCodecContext and free it.
    decoder->close();

    delete decoder;
    decoder = nullptr;
}

void XDecodeThread::clear()
{
    std::lock_guard<std::mutex> locker{mux};
    if (decoder) decoder->clear();

    // empty pktList.
    while (!pktList.empty()) {
        AVPacket* pkt = pktList.front();
        pktList.pop_front();
        av_packet_free(&pkt);
    }
}