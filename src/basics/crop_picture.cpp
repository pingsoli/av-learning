#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "utils.h"

extern "C" {
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersrc.h"
#include "libavfilter/buffersink.h"
#include "libavutil/imgutils.h"
}

#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "avutil.lib")

AVFrame* CropFrame(const AVFrame* src, int x, int y, int w, int h)
{
  AVFilterContext *srcFilterCtx = nullptr;
  AVFilterContext *sinkFilterCtx = nullptr;
  AVFilterGraph* avFilterGraph = avfilter_graph_alloc();
  AVFilterInOut *inputs = nullptr;
  AVFilterInOut *outputs = nullptr;
  AVFrame* frame = nullptr;
  int ret = 0;

  char args[512] = { 0 };
  snprintf(args, sizeof(args),
    "buffer=video_size=%dx%d:pix_fmt=%d:time_base=1/1:pixel_aspect=1/1[in];"
    "[in]crop=x=%d:y=%d:out_w=%d:out_h=%d[out];"
    "[out]buffersink",
    src->width, src->height, src->format,
    x, y, w, h);
  ret = avfilter_graph_parse2(avFilterGraph, args, &inputs, &outputs);
  if (ret < 0) goto cleanup;

  ret = avfilter_graph_config(avFilterGraph, nullptr);
  if (ret < 0) {
    char error_msg[256] = { 0 };
    std::cout << "Error: " << av_make_error_string(error_msg, sizeof(error_msg), ret) << std::endl;
    goto cleanup;
  }
  srcFilterCtx = avfilter_graph_get_filter(avFilterGraph, "Parsed_buffer_0");
  sinkFilterCtx = avfilter_graph_get_filter(avFilterGraph, "Parsed_buffersink_2");

  frame = av_frame_clone(src);
  ret = av_buffersrc_add_frame(srcFilterCtx, frame);
  if (ret < 0) {
    av_frame_free(&frame);
    goto cleanup;
  }
  ret = av_buffersink_get_frame(sinkFilterCtx, frame);
  if (ret < 0) {
    av_frame_free(&frame);
    goto cleanup;
  }

cleanup:
  avfilter_graph_free(&avFilterGraph);
  return frame;
}

int main(int argc, char* argv[])
{
  const char filename[] = "F:/av-learning/bin/win32/thor_640x360_yuv420p.yuv";
  std::ifstream infile(filename, std::ifstream::binary);
  std::stringstream iss;
  iss << infile.rdbuf();
  std::string yuv_data = iss.str();

  int width = 640;
  int height = 360;
  AVPixelFormat format = AV_PIX_FMT_YUV420P;
  // auto pic_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, 640, 360, 1);

  AVFrame *frame = av_frame_alloc();
  int required_size = av_image_fill_arrays(frame->data, frame->linesize,
    (const uint8_t*) yuv_data.c_str(), format, width, height, 1);
  frame->width = width;
  frame->height = height;
  frame->format = format;

  AVFrame* cropped_frame = CropFrame(frame, 20, 20, 300, 200);
  SaveFrameToJPEG( "thor_out.jpg", cropped_frame);

  av_frame_free(&frame);
  av_frame_free(&cropped_frame);
  return 0;
}