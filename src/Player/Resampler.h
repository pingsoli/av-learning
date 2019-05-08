#ifndef _RESAMPLER_H_
#define _RESAMPLER_H_

#include <cstdint>
struct SwrContext;
struct AVFrame;

class Resampler {
public:
  Resampler();
  ~Resampler();

  void Init(int64_t out_ch_layout, int out_sample_format, int out_sample_rate,
            int64_t in_ch_layout, int in_sample_format, int in_sample_rate);
  int Convert(uint8_t* out_pcm, AVFrame* frame);

private:
  SwrContext *swr_;
};

#endif