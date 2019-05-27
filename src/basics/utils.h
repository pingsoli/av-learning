#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include <chrono>
#include <ctime>

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

AVFrame* GetFrameFromPicture(const std::string& filename)
{
  int ret_code = 0;
  AVFormatContext *avFormatCtx = nullptr;
  if ((ret_code = avformat_open_input(&avFormatCtx, filename.c_str(), nullptr, nullptr)) != 0) {
    char error_msg_buf[256] = {0};
    av_strerror(ret_code, error_msg_buf, sizeof(error_msg_buf));
    std::cerr << "Error: avformat_open_input failed: " << error_msg_buf << std::endl;
    return false;
  }

  avformat_find_stream_info(avFormatCtx, nullptr);

  AVCodec *codec = nullptr;
  int videoStreamIdx = -1;
  videoStreamIdx = av_find_best_stream(avFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);
  if (videoStreamIdx < 0) goto cleanup_and_return;

  AVCodecContext *avCodecCtx = avcodec_alloc_context3(codec);

  ret_code = avcodec_open2(avCodecCtx, codec, nullptr);
  if (ret_code < 0) goto cleanup_and_return;

  AVPacket pkt;
  av_init_packet(&pkt);
  pkt.data = nullptr;
  pkt.size = 0;
  ret_code = av_read_frame(avFormatCtx, &pkt);
  if (ret_code < 0) goto cleanup_and_return;
  ret_code = avcodec_send_packet(avCodecCtx, &pkt);
  if (ret_code < 0) goto cleanup_and_return;

  AVFrame* frame = av_frame_alloc();
  ret_code = avcodec_receive_frame(avCodecCtx, frame);
  if (ret_code < 0) av_frame_free(&frame);

cleanup_and_return:
  if (avFormatCtx) avformat_close_input(&avFormatCtx);
  if (avCodecCtx) avcodec_free_context(&avCodecCtx);
  return frame;
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

// NOTE: not thread-safe
char* CurrentTimeStr()
{
  auto now = std::chrono::system_clock::now();
  auto mills = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
  std::time_t t = std::time(nullptr);

  static char buf[24] = { 0 };
  std::size_t len = std::strftime(buf, sizeof(buf), "%F %T", std::localtime(&t));
  std::sprintf(buf + len, ".%03lld", mills % 1000);
  return buf;
}

#endif