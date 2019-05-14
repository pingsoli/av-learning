#include <iostream>

#include "utils.h"

#define USE_LIBYUV

#if !defined(USE_LIBYUV)

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")

#elif defined(USE_LIBYUV)

extern "C" {
#include "libyuv.h"
#pragma comment(lib, "yuv.lib")
}

#endif

int main(int argc, char *argv[])
{
  AVFrame *frame = GetFrameFromYUVFile(
    "F:/av-learning/bin/win32/thor_640x360_yuv420p.yuv",
    640, 360, AV_PIX_FMT_YUV420P
  );

  int dstWidth = static_cast<int>(frame->width * 1);
  int dstHeight = static_cast<int>(frame->height * 0.5);

  AVFrame *outFrame = AVFrameMalloc(dstWidth, dstHeight, frame->format);

#if !defined(USE_LIBYUV)
  SwsContext* swsContext = sws_getContext(
   frame->width, frame->height, (AVPixelFormat) frame->format,
   outFrame->width, outFrame->height, (AVPixelFormat) outFrame->format,
   SWS_BICUBIC, nullptr, nullptr, nullptr);

  // Copy top-half picture to bottom-half position
  // sws_scale(swsContext, frame->data, frame->linesize,   0, frame->height / 2, outFrame->data, outFrame->linesize);
  // sws_scale(swsContext, frame->data, frame->linesize, frame->height / 2, frame->height / 2, outFrame->data, outFrame->linesize);

  // Copy whole picture to destination picture we want.
  sws_scale(swsContext, frame->data, frame->linesize, 0, frame->height, outFrame->data, outFrame->linesize);
  sws_freeContext(swsContext);
#elif defined(USE_LIBYUV)
  libyuv::I420Scale(
    frame->data[0], frame->linesize[0],
    frame->data[1], frame->linesize[1],
    frame->data[2], frame->linesize[2],
    frame->width, frame->height,
    outFrame->data[0], outFrame->linesize[0],
    outFrame->data[1], outFrame->linesize[1],
    outFrame->data[2], outFrame->linesize[2],
    outFrame->width, outFrame->height, libyuv::kFilterNone);
#endif

  char outFilename[256] = { 0 };
  snprintf(outFilename, sizeof(outFilename), "test-output-%dx%d.yuv", outFrame->width, outFrame->height);
  SaveI420P(outFilename, outFrame);
  SaveFrameToJPEG("test-output.jpeg", outFrame);

  av_frame_free(&frame);
  av_frame_free(&outFrame);

  return 0;
}