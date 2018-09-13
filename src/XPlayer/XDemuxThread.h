#pragma once

#include <string>
#include <mutex>
#include <atomic>

#include <QThread>

class IVideoCall;
class XDemuxer;
class XVideoThread;
class XAudioThread;
struct AVCodecParameters;

class XDemuxThread : public QThread
{
public:
    XDemuxThread();
    virtual ~XDemuxThread();

    // initialize audio and video thread.
    virtual bool open(const std::string& url, IVideoCall *videoPlayer);

    // close demuxing, audio and video thread.
    // NOTE: make sure to close() by yourself at last, or memory leak.
    virtual void close();

    // demuxer, video and audio thread do clearing job.
    virtual void clear();

    // start all threads, make sure to open() first.
    virtual void start();

    // inherited from QThread.
    virtual void run() override;

    // seek to specific position.
    virtual void seek(double pos);

    // set flag, that will stop demuxing, video and audio thread will suspend.
    void setPause(bool isPause_);

    // video file duration (ms precision).
    int64_t getTotalMs();

    // get video or audio pts.
    int64_t getPts();

protected:
    std::mutex mux; // protect all members.

    XDemuxer *demuxer = nullptr;
    XVideoThread *videoThread = nullptr;
    XAudioThread *audioThread = nullptr;

    AVCodecParameters *audioParams = nullptr;
    AVCodecParameters *videoParams = nullptr;
    std::atomic<bool> isExit = false;

private:
    int64_t totalMs; // video file total ms (duration).
    std::atomic<bool> isPause = false;
};
