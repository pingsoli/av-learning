#include "MediaInfo.h"

#include <iostream>
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avcodec.lib")

MediaInfo::MediaInfo()
  : avFormatCtx_(nullptr),
    audioStreamIdx_(-1),
    videoStreamIdx_(-1)
{
}

MediaInfo::~MediaInfo()
{
  if (avFormatCtx_) avformat_close_input(&avFormatCtx_);
  avformat_free_context(avFormatCtx_);
}

bool MediaInfo::Open(const std::string& filename)
{
  filename_ = filename;

  int ret_code = -1;
  if ((ret_code = avformat_open_input(&avFormatCtx_, filename.c_str(), nullptr, nullptr)) != 0) {
    char error_msg_buf[256] = { 0 };
    av_strerror(ret_code, error_msg_buf, sizeof(error_msg_buf));
    std::cerr << "Error: avformat_open_input failed: " << error_msg_buf << std::endl;
    return false;
  }

  for (std::size_t i = 0; i < avFormatCtx_->nb_streams; ++i)
  {
    switch (avFormatCtx_->streams[i]->codecpar->codec_type)
    {
      case AVMEDIA_TYPE_AUDIO:    audioStreamIdx_ = i; break;
      case AVMEDIA_TYPE_VIDEO:    videoStreamIdx_ = i; break;
      case AVMEDIA_TYPE_SUBTITLE: break;
      default: break;
    }
  }

  duration_ = avFormatCtx_->duration / (AV_TIME_BASE / 1000);
  avformat_find_stream_info(avFormatCtx_, nullptr);

  width_ = avFormatCtx_->streams[videoStreamIdx_]->codecpar->width;
  height_ = avFormatCtx_->streams[videoStreamIdx_]->codecpar->height;

  return true;
}

AVCodecParameters* MediaInfo::GetVideoCodecParameters() const {
  return (avFormatCtx_ ?
    avFormatCtx_->streams[videoStreamIdx_]->codecpar : nullptr
  );
}
AVCodecParameters* MediaInfo::GetAudioCodecParameters() const {
  return (avFormatCtx_ ?
    avFormatCtx_->streams[audioStreamIdx_]->codecpar : nullptr
  );
}