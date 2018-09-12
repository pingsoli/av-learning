#pragma once
#include <mutex>
#include <list>

#include <QThread>

#include "XDecodeThread.h"

struct AVCodecParameters;
class XDecoder;
class XResampler;
class XAudioPlayer;

class XAudioThread : public XDecodeThread
{
public:
    XAudioThread();
    virtual ~XAudioThread();

    // NOTE: call open function will not release the AVCodecParameters object.
    virtual bool open(AVCodecParameters *params);

    // inherited from QThread.
    // call start() method will invoke run() in the behind.
    virtual void run() override;

    // stop current thread, and clean up.
    virtual void close();
    virtual void clear();

    // set pause flag.
    void setPause(bool isPause_);

    int64_t getPts();

private:
    XResampler *audioResampler = nullptr;
    XAudioPlayer *audioPlayer = nullptr;
    int64_t pts = 0;

    std::mutex amux; // protect all members exclude inherited members.
};
