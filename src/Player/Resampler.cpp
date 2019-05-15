#include "Resampler.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
}

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "swresample.lib")

Resampler::Resampler()
{
}

Resampler::~Resampler()
{
  if (swr_)
    swr_free(&swr_);
}

void Resampler::Init(int64_t out_ch_layout, int out_sample_format, int out_sample_rate,
                     int64_t in_ch_layout, int in_sample_format, int in_sample_rate)
{
    swr_ = swr_alloc_set_opts(nullptr,
      out_ch_layout, (AVSampleFormat)out_sample_format, out_sample_rate,
      in_ch_layout,  (AVSampleFormat)in_sample_format,  in_sample_rate,
      0, nullptr);
    swr_init(swr_);

    target_chanel_layout_ = out_ch_layout;
    target_sample_format_ = out_sample_format;
    target_sample_rate_ = out_sample_rate;
}

int Resampler::Convert(AVFrame* outFrame, const AVFrame* frame)
{
  outFrame->format = target_sample_format_;
  outFrame->channel_layout = target_chanel_layout_;
  outFrame->sample_rate = target_sample_rate_;

  return swr_convert(swr_, outFrame->data, frame->nb_samples,
           (const uint8_t**) frame->data, frame->nb_samples);
}