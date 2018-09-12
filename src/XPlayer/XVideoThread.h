#pragma once

#include <mutex>
#include <list>

#include <QThread>

#include "XDecodeThread.h"

struct AVCodecParameters;
struct AVPacket;
class XDecoder;
class XVideoWidget;
class IVideoCall;

// decode and display video.
class XVideoThread : public XDecodeThread
{
public:
    XVideoThread();
    virtual ~XVideoThread();

    // open decoder.
    // NOTE: remember to free AVCodecParameters by yourself.
    virtual bool open(AVCodecParameters* params, IVideoCall* videoPlayer);

    // close current thread.
    virtual void close();

    // inherited from QThread, called when start() method invoked.
    virtual void run() override;

    int64_t getPts();
    void setPts(int64_t pts_);

    // set flag, and stop sending video packet and rendering.
    void setPause(bool isPause_);

    // set to public for outside.
    IVideoCall *videoPlayer = nullptr;

private:
    std::mutex vmux; // protect all members exclude inherited members.
    int64_t pts = 0; // video pts.
};

