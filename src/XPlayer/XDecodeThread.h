#pragma once

#include <mutex>
#include <list>
#include <atomic>

#include <QThread>

struct AVPacket;
class XDecoder;

// XDecodeThread is not a real thread.
// XVideoThread and XAudioThread are both inherited from XDecodeThread.

class XDecodeThread : public QThread
{
public:
    XDecodeThread();
    virtual ~XDecodeThread();

    virtual void push(AVPacket *pkt);
    virtual AVPacket* pop();

    // close decoder.
    virtual void close();
    // clean up the AVPacket list, and clear the decoder.
    virtual void clear();

    XDecoder *decoder = nullptr;

protected:
    std::mutex mux;

    // flag for exiting audio or video thread.
    std::atomic<bool> isExit = false;
    // flag for suspending audio or video thread.
    std::atomic<bool> isPause = false;

    std::list<AVPacket*> pktList;
    const unsigned int max_size = 100;
};