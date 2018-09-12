#pragma once
#include <mutex>
#include <string>

class AVFormatContext;
class AVPacket;
class AVCodecParameters;

class XDemuxer
{
public:
    XDemuxer();
    ~XDemuxer();

    // open a video file or rtsp, rtmp, http stream, and get the relative parameters.
    virtual bool open(const std::string& url);

    // read, and get a AVPacket for giving it to decoder.
    // NOTE: remember release AVPacket (by av_packet_free()).
    virtual AVPacket* read();

    // get video parameters allocated the memory on heap.
    // NOTE: remember to release the AVCodecParameters by yourself.
    virtual AVCodecParameters* copyVideoParams();

    // get audio parameters allocated the memory on heap.
    // NOTE: remember to release the AVCodecParameters by yourself.
    virtual AVCodecParameters* copyAudioParams();

    // seek operation (position range: 0.0 ~ 1.0), stands for ratio or progress bar.
    virtual bool seek(double position);

    // flush the read buffer, not free the AVFormatContext handle.
    virtual void clear();

    // free AVFormatContext handle.
    virtual void close();

    // NOTE: It's defective, is only one of video or audio stream.
    // In such situation, subtitle streasm is audio too.
    virtual bool isAudio(AVPacket* pkt);

    // video only.
    int width = 0;
    int height = 0;

    // audio only.
    int sample_rate = 0;
    int channels = 0;

    int64_t totalMs; // video duraiton(ms precision).

protected:
    std::mutex mux; // protect AVFormatContext
    AVFormatContext *fctx = nullptr;

    int videoStream = -1;
    int audioStream = -1;

private:
    // reuse the public interface. both copyAudioParams() and copyVideoParams()
    // use copyParams().
    AVCodecParameters* copyParams(int stream_index);
};