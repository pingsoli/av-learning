#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <tuple>
#include <vector>

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
}

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "swscale.lib")

struct EncodePictureConfig {
  std::string ext_name; // picture file extension name(without dot'.'), such as: jpg, jpeg or png and etc.
  AVCodecID codec_id;
  AVPixelFormat codec_fmt; // encoder supported pixel format. MJPEG only support AV_PIX_FMT_YUVJ420P, you cannot use AV_PIX_FMT_YUV420P directly.
  AVPixelFormat frame_fmt; // frame pixel format. you must convert frame to wanted pixel format frame.
} pic_configs[] = {
  { "jpg",  AV_CODEC_ID_MJPEG, AV_PIX_FMT_YUVJ420P, AV_PIX_FMT_YUV420P },
  { "jpeg", AV_CODEC_ID_MJPEG, AV_PIX_FMT_YUVJ420P, AV_PIX_FMT_YUV420P },
  { "png",  AV_CODEC_ID_PNG,   AV_PIX_FMT_RGB24,    AV_PIX_FMT_RGB24   }
};

static std::tuple<AVCodecID, AVPixelFormat, AVPixelFormat>
  GetPictureEncoderInfo(const std::string& extension_name)
{
  for (int i = 0; i < sizeof(pic_configs); ++i) {
    if (extension_name == pic_configs[i].ext_name) {
      return { pic_configs[i].codec_id, pic_configs[i].codec_fmt, pic_configs[i].frame_fmt };
    }
  }
  return { AV_CODEC_ID_NONE, AV_PIX_FMT_NONE, AV_PIX_FMT_NONE };
}

// It's an encode process, encode one frame to another wanted spec frame.
int SaveFrameToPicture(const AVFrame* frame, const std::string& filename)
{
  char error_msg_buf[256] = { 0 };
  AVFrame* dstFrame = nullptr;

  std::string ext_name = filename.substr(filename.find_last_of('.') + 1);
  std::transform(ext_name.begin(), ext_name.end(), ext_name.begin(), ::tolower);

  AVCodecID codecId;
  AVPixelFormat codecFormat;
  AVPixelFormat frameFormat;
  std::tie(codecId, codecFormat, frameFormat) = GetPictureEncoderInfo(ext_name);
  if (codecId == AV_CODEC_ID_NONE 
    || codecFormat == AV_PIX_FMT_NONE
    || frameFormat == AV_PIX_FMT_NONE)
    return -1;

  // converts AVFrame to specified pixel format frame when given pixel format is not suitable
  // yuv420p to rgb24 or rgb24 to yuv420p
  if (frame->format != frameFormat) {
    dstFrame = av_frame_alloc();
    av_image_alloc(dstFrame->data, dstFrame->linesize, frame->width, frame->height, frameFormat, 1);

    SwsContext *swsContext = sws_getContext(
        frame->width, frame->height, (AVPixelFormat)frame->format,
        frame->width, frame->height, frameFormat,
        SWS_BILINEAR, nullptr, nullptr, nullptr);
    sws_scale(swsContext, frame->data, frame->linesize, 0, frame->height, dstFrame->data, dstFrame->linesize);
    sws_freeContext(swsContext);
    dstFrame->width = frame->width;
    dstFrame->height = frame->height;
    dstFrame->format = frameFormat;
  } else {
    dstFrame = const_cast<AVFrame *>(frame);
  }

  AVCodec *codec = avcodec_find_encoder(codecId);
  if (!codec) return -2;
  AVCodecContext *codecCtx = avcodec_alloc_context3(codec);
  if (!codecCtx) return -3;

  codecCtx->pix_fmt = codecFormat;
  codecCtx->width = dstFrame->width;
  codecCtx->height = dstFrame->height;
  codecCtx->time_base = { 1, 25 };
  codecCtx->framerate = { 25, 1 };

  int ret = avcodec_open2(codecCtx, codec, nullptr);
  if (ret < 0) {
    std::cerr << "avcodec open failed" << std::endl;
    ret = -4;
    goto cleanup;
  }

  AVPacket pkt;
  av_init_packet(&pkt);
  pkt.data = nullptr;
  pkt.size = 0;

  ret = avcodec_send_frame(codecCtx, dstFrame);
  if (ret < 0) {
    std::cerr << "Error: " <<
      av_make_error_string(error_msg_buf, sizeof(error_msg_buf), ret) << std::endl;
    ret = -5;
    goto cleanup;
  }

  while (ret >= 0) {
    ret = avcodec_receive_packet(codecCtx, &pkt);
    if (ret == AVERROR(EAGAIN)) continue;
    if (ret == AVERROR_EOF) break;

    std::ofstream outfile(filename, std::ios::binary);
    outfile.write((char*) pkt.data, pkt.size);
    break;
  }

cleanup:
  avcodec_free_context(&codecCtx);
  if (dstFrame != frame) av_frame_free(&dstFrame);
  return ret;
}

int main(int argc, char* argv[])
{
  const char filename[] = "G:/av-learning/bin/win32/test_1280x720_yuv420p.yuv";
  std::ifstream infile(filename, std::ios::binary);
  std::stringstream ss;
  ss << infile.rdbuf();
  std::string data = ss.str();
  int width = 1280;
  int height = 720;

  // fill AVFrame with yuv data
  AVFrame *frame = av_frame_alloc();
  av_image_fill_arrays(frame->data, frame->linesize,
    (const uint8_t*) data.c_str(), (AVPixelFormat) AV_PIX_FMT_YUV420P, width, height, 1);
  frame->width = width;
  frame->height = height;
  frame->format = AV_PIX_FMT_YUV420P;

  auto check_ret = [] (const std::string& filename, int ret) {
    std::cout
      << "Save AVFrame to picture(" << filename  << "): "
      << (ret == 0 ? "ok" : "failed") << "!\n"; 
  };

  std::vector<std::string> out_filenames = {
    "test-output.jpg",
    "test-output.jpeg",
    "test-output.png",
    "test-output.gif"
  };

  for (auto &filename : out_filenames) {
    check_ret(filename, SaveFrameToPicture(frame, filename));
  }

  av_frame_free(&frame);
  return 0;
}