#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>
#include <sstream>
#include <fstream>

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
}

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")

AVFrame *GetFrameFromYUVFile(const std::string& filename, int width, int height, int format)
{
  AVFrame *frame = av_frame_alloc();
  frame->width = width;
  frame->height = height;
  frame->format = format;
  av_frame_get_buffer(frame, 1);

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

#endif