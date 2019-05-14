#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libavcodec/avcodec.h"
}

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avcodec.lib")


AVFrame* AVFrameMalloc(int width, int height, int format);
int SaveI420P(const std::string& filename, const AVFrame* frame);
int SaveFrameToJPEG(const std::string& filename, const AVFrame* frame);
AVFrame *GetFrameFromYUVFile(const std::string& filename, int width, int height, int format);


AVFrame *GetFrameFromYUVFile(const std::string& filename, int width, int height, int format)
{
  AVFrame *frame = AVFrameMalloc(width, height, format);

  std::ifstream infile;
  infile.open(filename, std::ios::binary);
  if (!infile) {
    av_frame_free(&frame);
    return nullptr;
  }
  std::stringstream ss;
  ss << infile.rdbuf();
  std::string yuv_data = ss.str();
  
  uint8_t* data = (uint8_t*)av_malloc(yuv_data.size());
  if (!data) {
    av_frame_free(&frame);
    return nullptr;
  }
  memcpy(data, yuv_data.c_str(), yuv_data.length());

  av_image_fill_arrays(frame->data, frame->linesize,
    data, (AVPixelFormat)format, width, height, 1);

  return frame;
}

int SaveI420P(const std::string& filename, const AVFrame* frame)
{
  if (frame->format != AV_PIX_FMT_YUV420P) return -1;
  std::ofstream outfile(filename, std::ios::binary);
  size_t y_size = frame->width * frame->height;
  size_t u_size = y_size / 4;
  outfile.write((const char*)frame->data[0], y_size);
  outfile.write((const char*)frame->data[1], u_size);
  outfile.write((const char*)frame->data[2], u_size); // u size is equal to v
  return 0;
}

int SaveFrameToJPEG(const std::string& filename, const AVFrame* frame)
{
  char error_msg_buf[256] = { 0 };

  AVCodec *jpegCodec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
  if (!jpegCodec) return -1;
  AVCodecContext *jpegCodecCtx = avcodec_alloc_context3(jpegCodec);
  if (!jpegCodecCtx) return -2;

  jpegCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P; // NOTE: Don't use AV_PIX_FMT_YUV420P
  jpegCodecCtx->width = frame->width;
  jpegCodecCtx->height = frame->height;
  jpegCodecCtx->time_base = { 1, 25 };
  jpegCodecCtx->framerate = { 25, 1 };

  int ret = avcodec_open2(jpegCodecCtx, jpegCodec, nullptr);
  if (ret < 0) {
    std::cerr << "avcodec open failed" << std::endl;
    avcodec_free_context(&jpegCodecCtx);
    return -3;
  }
  AVPacket pkt;
  av_init_packet(&pkt);
  pkt.data = nullptr;
  pkt.size = 0;

  ret = avcodec_send_frame(jpegCodecCtx, frame);
  if (ret < 0) {
    std::cerr << "Error: " <<
      av_make_error_string(error_msg_buf, sizeof(error_msg_buf), ret) << std::endl;
    avcodec_free_context(&jpegCodecCtx);
    return -4;
  }

  while (ret >= 0) {
    ret = avcodec_receive_packet(jpegCodecCtx, &pkt);
    if (ret == AVERROR(EAGAIN)) continue;
    if (ret == AVERROR_EOF) break;

    std::ofstream outfile(filename, std::ios::binary);
    outfile.write((char*) pkt.data, pkt.size);
    break;
  }

  avcodec_free_context(&jpegCodecCtx);
  return 0;
}

AVFrame* AVFrameMalloc(int width, int height, int format)
{
  AVFrame *frame = av_frame_alloc();
  frame->width = width;
  frame->height = height;
  frame->format = format;
  av_frame_get_buffer(frame, 1);
  return frame;
}

#endif