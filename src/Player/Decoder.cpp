#include <iostream>

#include "Decoder.h"
#include "MediaInfo.h"

extern "C" {
#include "libavcodec/avcodec.h"
}

#pragma comment(lib, "avcodec.lib")

Decoder::Decoder(const MediaInfo& info)
  : videoCodecCtx_(nullptr),
    videoCodec_(nullptr),
    audioCodecCtx_(nullptr),
    audioCodec_(nullptr)
{
  videoCodecCtx_ = avcodec_alloc_context3(nullptr);
  avcodec_parameters_to_context(videoCodecCtx_, info.GetVideoCodecParameters());

  audioCodecCtx_ = avcodec_alloc_context3(nullptr);
  avcodec_parameters_to_context(audioCodecCtx_, info.GetAudioCodecParameters());

  videoCodec_ = avcodec_find_decoder(videoCodecCtx_->codec_id);
  audioCodec_ = avcodec_find_decoder(audioCodecCtx_->codec_id);

  int r = avcodec_open2(videoCodecCtx_, videoCodec_, nullptr);
  if (r < 0)
  {
    std::cerr << "Warning: find video decoder failed" << std::endl;
  }

  r = avcodec_open2(audioCodecCtx_, audioCodec_, nullptr);
  if (r < 0)
  {
    std::cerr << "Warning: find audio decoder failed" << std::endl;
  }
}