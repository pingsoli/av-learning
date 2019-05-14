#include "utils.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")

int main(int argc, char *argv[])
{
  AVFrame *frame = GetFrameFromYUVFile(
    "G:/av-learning/bin/win32/test_1280x720_yuv420p.yuv",
    1280, 720, AV_PIX_FMT_YUV420P
  );

  int dstWidth = static_cast<int>(frame->width * 1);
  int dstHeight = static_cast<int>(frame->height * 0.5);

  AVFrame *outFrame = av_frame_alloc();
  outFrame->width = dstWidth;
  outFrame->height = dstHeight;
  outFrame->format = frame->format;
  av_frame_get_buffer(outFrame, 1);

  SwsContext* swsContext = sws_getContext(
   frame->width, frame->height, (AVPixelFormat) frame->format,
   outFrame->width, outFrame->height, (AVPixelFormat) outFrame->format,
   SWS_BICUBIC, nullptr, nullptr, nullptr);

  // Copy top-half picture to bottom-half position
  // sws_scale(swsContext, frame->data, frame->linesize,   0, frame->height / 2, outFrame->data, outFrame->linesize);
  // sws_scale(swsContext, frame->data, frame->linesize, frame->height / 2, frame->height / 2, outFrame->data, outFrame->linesize);

  // Copy whole picture to destination picture we want.
  sws_scale(swsContext, frame->data, frame->linesize, 0, frame->height, outFrame->data, outFrame->linesize);

  char outFilename[256] = { 0 };
  snprintf(outFilename, sizeof(outFilename), "test-output-%dx%d.yuv", outFrame->width, outFrame->height);
  SaveI420P(outFilename, outFrame);

  av_frame_free(&frame);
  av_frame_free(&outFrame);
  sws_freeContext(swsContext);
  return 0;
}