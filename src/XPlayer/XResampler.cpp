#include <iostream>

#include "XResampler.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
}

#pragma comment(lib, "avcodec.lib")


XResampler::XResampler()
{
}

XResampler::~XResampler()
{
}

bool XResampler::open(AVCodecParameters* params)
{
    int ret_code = -1;
    {
        std::lock_guard<std::mutex> locker{mux};

        // if actx is nullptr, the allocation will be executed automatically by swr_alloc_set_opts.
        actx = swr_alloc_set_opts(actx,
            // output data.
            av_get_default_channel_layout(2), // 2 channels for output. (specify what you want).
            static_cast<AVSampleFormat>(sample_format), // sample format for output (16 bits default)
            params->sample_rate,  // sample rate for output

            // input data.
            av_get_default_channel_layout(params->channel_layout), // inoputchannel layout.
            static_cast<AVSampleFormat>(params->format), // sample format of input audio stream.
            params->sample_rate, // sample rate of input audio stream.
            0, nullptr
        );

        ret_code = swr_init(actx);  // initialize first, otherwise won't compile.
    }

    if (ret_code != 0) {
        char buf[256];
        std::cout << "swr_init failed, message: " << av_strerror(ret_code, buf, sizeof(buf) - 1) << std::endl;
        return false;
    }

    return true;
}

void XResampler::close()
{
    std::lock_guard<std::mutex> locker{mux};
    if (!actx) swr_free(&actx);
}

int XResampler::resample(AVFrame* frame, uint8_t* data)
{
    if (!data || !frame) {
        av_frame_free(&frame);
        return 0;
    }

    uint8_t *dat[2] = { 0 };
    dat[0] = data;

    int sample_numbers = swr_convert(actx,
        dat, frame->nb_samples,       // output data
        const_cast<const uint8_t**>(frame->data), frame->nb_samples // input data
    );

    // failed. return a negative number.
    if (sample_numbers <= 0) return sample_numbers;

    return av_samples_get_buffer_size(nullptr, 
        av_frame_get_channels(frame),
        frame->nb_samples,
        (AVSampleFormat) sample_format, 1);

    // calculate the size by ourself.
    //auto return_size = sample_numbers * frame->channels
    //    * av_get_bytes_per_sample(static_cast<AVSampleFormat>(sample_format));
}