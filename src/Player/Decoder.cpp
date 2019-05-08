#include <iostream>

#include "Decoder.h"
#include "MediaInfo.h"

extern "C" {
#include "libavcodec/avcodec.h"
}

#pragma comment(lib, "avcodec.lib")

Decoder::Decoder()
  : videoCodecCtx_(nullptr),
    videoCodec_(nullptr),
    audioCodecCtx_(nullptr),
    audioCodec_(nullptr)
{
}

Decoder::~Decoder()
{
  avcodec_close(videoCodecCtx_);
  avcodec_close(audioCodecCtx_);
}

int Decoder::Open(const MediaInfo& info)
{
  bool video_codec_open_failed = false;
  bool audio_codec_open_failed = false;

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
    video_codec_open_failed = true;
  }

  r = avcodec_open2(audioCodecCtx_, audioCodec_, nullptr);
  if (r < 0)
  {
    std::cerr << "Warning: find audio decoder failed" << std::endl;
    audio_codec_open_failed = true;
  }

  if (video_codec_open_failed && !audio_codec_open_failed)
    return -1;
  else if (!video_codec_open_failed && audio_codec_open_failed)
    return -2;
  else if (video_codec_open_failed && audio_codec_open_failed)
    return -3;
  else
    return 0;
}