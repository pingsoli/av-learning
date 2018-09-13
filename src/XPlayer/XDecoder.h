#pragma once
#include <mutex>

struct AVCodecParameters;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;

class XDecoder
{
public:
    XDecoder();
    virtual ~XDecoder();

    // find decoder and open the decoder (one of video and audio).
    // if failed, free the AVCodecParameters.
    // if succeed, you mush free it by yourself.
    virtual bool open(AVCodecParameters* params);

    // flush and close the handle.
    virtual void clear();
    virtual void close();

    // send a AVPacket to decoder thread, and clear the AVPacket.
    virtual bool send(AVPacket* pkt);

    // get decoded frame, audio AVPacket may exists one-send()-multi-recv().
    // caller free frame space every frame.
    // defects: allocated memory space every time, 
    // should build a AVPacket or AVFrame pool.
    virtual AVFrame* recv();

    // get current pts.
    int64_t getPts();
    //void setPts(int64_t pts_);

private:
    std::mutex mux; // protect AVCodecContext.
    AVCodecContext* codec = nullptr;
    int64_t pts = 0;
};

