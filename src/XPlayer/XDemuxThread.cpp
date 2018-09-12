#include <iostream>

#include "XDemuxThread.h"
#include "IVideoCall.h"
#include "XDemuxer.h"
#include "XDecoder.h" // the code is messed up.
#include "XVideoThread.h"
#include "XAudioThread.h"
#include "IVideoCall.h"

extern "C" {
#include "libavcodec/avcodec.h"
}

XDemuxThread::XDemuxThread()
{
}

XDemuxThread::~XDemuxThread()
{
    isExit = true;
    wait();
}

void XDemuxThread::close()
{
    isExit = true;
    // set quit flag, and wait for current thread quit from run() method.
    wait();

    std::lock_guard<std::mutex> locker{ mux };
    if (audioThread) audioThread->close();
    if (videoThread) videoThread->close();

    delete audioThread;
    delete videoThread;

    audioThread = nullptr;
    videoThread = nullptr;
}

void XDemuxThread::clear()
{
    std::lock_guard<std::mutex> locker{mux};
    if (demuxer) demuxer->clear();
    if (audioThread) audioThread->clear();
    if (videoThread) videoThread->clear();
}

bool XDemuxThread::open(const std::string& url, IVideoCall* videoPlayer)
{
    std::lock_guard<std::mutex> locker{mux};
    if (!demuxer) demuxer = new XDemuxer();
    if (!audioThread) audioThread = new XAudioThread();

    // if we want to play the video, then initialize video and start to play.
    if (videoPlayer && !videoThread) videoThread = new XVideoThread();

    if (!demuxer->open(url)) {
        std::cout << "demuxer open failed!" << std::endl;
        return false;
    }

    // NOTE: exist memory leak, copyAudioParams() will allocated new memory space.
    // we must free it from outside.
    audioParams = demuxer->copyAudioParams();
    if (!audioThread->open(audioParams)) {
        std::cout << "audio thread open failed!" << std::endl;
        return false;
    }

    // satisfy we only want to play audio, not playing video.
    if (videoThread) {
        videoParams = demuxer->copyVideoParams();
        if (!videoThread->open(videoParams, videoPlayer)) {
            std::cout << "video thread open failed!" << std::endl;
            return false;
        }
    }

    totalMs = demuxer->totalMs;

    // free the AVCodecParameters. (avoid memory leak)
    avcodec_parameters_free(&audioParams);
    avcodec_parameters_free(&videoParams);

    return true;
}

void XDemuxThread::start()
{
    // start current thread.
    QThread::start();

    std::lock_guard<std::mutex> locker{mux};
    // make sure audioThread and videoThread has been initialized.
    if (audioThread) audioThread->start();
    if (videoThread) videoThread->start();
}

void XDemuxThread::run()
{
    AVPacket *pkt = nullptr;
    while (!isExit) {

        // set stop demuxing.
        if (isPause) {
            msleep(5);
            continue;
        }

        std::unique_lock<std::mutex> locker{mux};

        // synchronize audio and video. (very simple)
        // not consider the complex situation yet.
        //if (videoThread && audioThread) {
        //}

        if (!demuxer) {
            locker.unlock();
            msleep(5);
            continue;
        }

        pkt = demuxer->read();
        if (!pkt) {
            locker.unlock();
            msleep(5);
            continue;
        }

        // NOTE: push() method may block the current thread when AVPacket list is full.
        // because push() will wait for enough space to put the AVPacket to list.
        // so the outside mutex will be occupied a long time, and other thread cannot
        // get the mutex. If audio or video AVPacket comsumer has already stopped.
        // the push() will block forever.
        // so, if we press 'pause' push button, we will discard current AVPacket.
        if (demuxer->isAudio(pkt)) {
            // audio
            if (audioThread) audioThread->push(pkt);
        } else {
            // video
            if (videoThread) videoThread->push(pkt);
        }

        locker.unlock();
        // IMPORTANT: make demux thread and video thread context switch.
        // make video playing more fluent.
        msleep(1); // sleep 1 millisecond.
    } // demuxer thread main loop.
}

void XDemuxThread::seek(double pos)
{
    // record the old state, we will recover it later.
    bool old_state = isPause;
    if (!isPause) setPause(true);

    // demux, videoThread and audioThread clear.
    // NOTE: make sure to pause first.
    // because clear() has to empty the video and audio AVPacket list.
    // if video or audio thread access the AVPacket list, may cause crash.
    // NOTE: no guarantee for pausing other thread first, just decrease
    // the chances.
    clear();
    
    std::unique_lock<std::mutex> locker{mux};
    if (demuxer) demuxer->seek(pos);

    int64_t seekPos = pos * demuxer->totalMs;

    // support drag progress bar to draw picture when pausing.
    while (!isExit && videoThread) {
        AVPacket *pkt = demuxer->read();

        // discard audio packet.
        if (demuxer->isAudio(pkt)) {
            av_packet_free(&pkt);
            continue;
        }

        if (!videoThread->decoder->send(pkt)) break;
        // enter drain mode, so recv() will not return EAGAIN.
        // but send nullptr stands for end of file.
        // video thread call avcodec_receive_frame() will return EOF every time.
        //videoThread->decoder->send(nullptr);

        // recv() until success.
        AVFrame *frame  = nullptr;
        frame = videoThread->decoder->recv();
        if (!frame) continue;

        // arrive destination position.
        // NOTE: the frame must be released.
        // repaint() will free AVFrame inside.
        // if same position, we must free the by ourself.
        if (frame->pts != seekPos) {
            videoThread->setPts(frame->pts);
            videoThread->videoPlayer->repaint(frame);
        } else {
            av_frame_free(&frame);
        }

        break;
    };
    locker.unlock();

    // recover the old state.
    setPause(old_state);
}

void XDemuxThread::setPause(bool isPause_)
{
    // Why not use std::lock_guard<std::mutex> ?
    // there has a big delay on playing or stopping the video.
    // Don't try the following code:
    //std::lock_guard<std::mutex> locker(mux); // lock the mutex, may take too much time.
    //isPause = isPause_;
    
    // right way
    isPause = isPause_;

    std::lock_guard<std::mutex> locker{mux};
    if (audioThread) audioThread->setPause(isPause);
    if (videoThread) videoThread->setPause(isPause);
}

int64_t XDemuxThread::getTotalMs() {
    std::lock_guard<std::mutex> locker{mux};
    return totalMs;
}

int64_t XDemuxThread::getPts() {
    std::lock_guard<std::mutex> locker{mux};

    // CAUTION: take care the priority of video and audio pts.
    // use video pts at first, if video thread does not exist,
    // use audio pts as main pts for progress bar.
    if (videoThread) return videoThread->getPts();
    if (audioThread) return audioThread->getPts();
    return 0;
}