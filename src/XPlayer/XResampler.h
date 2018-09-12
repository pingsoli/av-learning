#pragma once

#include <mutex>

struct AVCodecParameters;
struct AVFrame;
struct SwrContext;

class XResampler
{
public:
    XResampler();
    virtual ~XResampler();

    // resampling with concrete audio parameters.
    // NOTE: AV_SAMPLE_FMT_S16 is default sample format.
    // AVCodecParameters needs to be freed (there exists memory leak).
    virtual bool open(AVCodecParameters* params);

    // free SwrContext object.
    virtual void close();

    // resample and return the resampling size.
    virtual int resample(AVFrame* frame, uint8_t* data);

private:
    std::mutex mux; // protect all members.

    int sample_format = 1; // AV_SAMPLE_FMT_S16 is default.
    SwrContext *actx = nullptr; // main object of resampling.
};