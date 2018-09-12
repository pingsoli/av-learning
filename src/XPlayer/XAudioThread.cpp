#include <iostream>

#include "XAudioThread.h"
#include "XResampler.h"
#include "XDecoder.h"
#include "XAudioPlayer.h"

extern "C" {
#include "libavcodec/avcodec.h"
}

XAudioThread::XAudioThread()
{
}

XAudioThread::~XAudioThread()
{
}

void XAudioThread::close()
{
    isExit = true;
    wait();

    XDecodeThread::close();

    if (audioResampler) {
        audioResampler->close();
        std::lock_guard<std::mutex> locker{amux};
        delete audioResampler;
        audioResampler = nullptr;
    }

    if (audioPlayer) {
        audioPlayer->close();
        std::lock_guard<std::mutex> locker{amux};
        // audio player is singleton(static object).
        audioPlayer = nullptr;
    }
}

void XAudioThread::clear()
{
    XDecodeThread::clear();
    std::lock_guard<std::mutex> locker{amux};
    if (audioPlayer) audioPlayer->clear();
}

void XAudioThread::setPause(bool isPause_)
{
    isPause = isPause_;
    std::lock_guard<std::mutex> locker{amux};
    if (audioPlayer)
        audioPlayer->setPause(isPause);
}

int64_t XAudioThread::getPts() {
    std::lock_guard<std::mutex> locker{amux};
    return pts;
}

bool XAudioThread::open(AVCodecParameters *params)
{
    if (!params) return false;

    clear();

    std::lock_guard<std::mutex> locker{amux};
    if (!decoder)        decoder = new XDecoder;
    if (!audioResampler) audioResampler = new XResampler;
    if (!audioPlayer)    audioPlayer = XAudioPlayer::get_instance();

    if (!decoder->open(params)) {
        std::cout << "audio decoder open failed!" << std::endl;
        return false;
    }

    if (!audioResampler->open(params)) {
        std::cout << "audio resample open failed!" << std::endl;
        return false;
    }

    if (!audioPlayer->open()) {
        std::cout << "audio player open failed!" << std::endl;
        return false;
    }

    audioPlayer->setChannels(params->channels);
    audioPlayer->setSampleRate(params->sample_rate);

    pts = 0; // pts initialization.

    return true;
}

void XAudioThread::run()
{
    uint8_t *pcm = new uint8_t[10 << 20]; // 10M memory space.
    //std::fill_n(pcm, 10 << 20, 0); // zero the memory space.
    AVPacket *pkt = nullptr; // the packet for decoder to send to pending list.
    AVFrame *frame = nullptr; // the frame for received.

    while (!isExit) {

        if (isPause) {
            msleep(5);
            continue;
        }

        std::unique_lock<std::mutex> locker{amux};
        if (pktList.empty()) {
            locker.unlock();
            msleep(1);
            continue;
        }
        
        // get top audio packet.
        pkt = pop();

        // audio decoer send a packet.
        if (!decoder->send(pkt)) {
            // send failed, and continue to send.
            locker.unlock();
            msleep(1);
            continue;
        }

        // decode audio frame. one send multiple times recv.
        while (!isExit) {
            frame = decoder->recv();
            if (!frame) break;

            // subtract unplay buffer time(ms)
            pts = decoder->getPts() - audioPlayer->getNoPlayMs();
            //std::cout << "audio pts = " << pts << std::endl; // print pts.

            int size = audioResampler->resample(frame, pcm);

            //std::cout << "audio resample size: " << size << std::endl;
            std::cout << "audio pts: " << frame->pts << std::endl;
            pts = frame->pts;

            // IMPORTANT: free current audio frame, otherwise memory leaking.
            av_frame_free(&frame);

            // play raw audio data.
            while (!isExit) {
                if (size <= 0) break;

                // audio IO context free space smaller than the audio frame size.
                if (audioPlayer->get_free() < size || isPause) {

                    // press 'pause', and unlock the lock, let other thread run().
                    locker.unlock(); // unlock to avoid dead lock.
                    while (isPause) {
                        msleep(1);
                        continue;
                    }
                    locker.lock(); // recover the lock

                    msleep(1);
                    continue;
                }

                // write the raw data to voice card.
                audioPlayer->write(pcm, size);
                break;
            }
        }
    }

    delete [] pcm;
}