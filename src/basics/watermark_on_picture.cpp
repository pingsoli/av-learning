// Add watermark on picture
// references
// 1. https://blog.csdn.net/leixiaohua1020/article/details/29368911
// 2. https://blog.csdn.net/li_wen01/article/details/62442162
//
// The process the filter
// source buffer filter -> sink buffer filter
//
// Tips: yuvplayer will be helpful, downloading it on sourceforge.

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

extern "C" {
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersrc.h"
#include "libavfilter/buffersink.h"
#include "libavutil/imgutils.h"

// include other file for testing
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}

#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avcodec.lib")

AVFilterContext *buffersink_ctx;
AVFilterContext *buffersrc_ctx;
AVFilterGraph *filter_graph;

int init_filters(const char* filters_desc, const AVFrame* frame)
{
  char args[512] = { 0 };
  int ret = 0;
  char error_msg[256] = { 0 };
 
  const AVFilter *buffersrc = avfilter_get_by_name("buffer"); // input buffer filter
  const AVFilter *buffersink = avfilter_get_by_name("buffersink"); // output buffer filter
  AVFilterInOut *inputs = avfilter_inout_alloc();
  AVFilterInOut *outputs = avfilter_inout_alloc();
  AVRational time_base = { 1, 25 }; // It's suitable for this example, maybe not suitable for you.

  filter_graph = avfilter_graph_alloc();

  // buffer video source: the decoded frames from the decoder will be inserted here
  snprintf(args, sizeof(args),
    "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
    frame->width, frame->height, frame->format,
    time_base.num, time_base.den, 1, 1);
  ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in", args, nullptr, filter_graph);
  if (ret < 0) {
    std::cerr << "Cannot create buffer source" << std::endl;
    goto cleanup_and_return;
  }

  ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
                                     nullptr, nullptr, filter_graph);
  if (ret < 0) {
    std::cerr << "Cannot create buffer sink: " <<
      av_make_error_string(error_msg, sizeof(error_msg), ret) << std::endl;
    goto cleanup_and_return;
  }

  outputs->name = av_strdup("in");
  outputs->filter_ctx = buffersrc_ctx;
  outputs->pad_idx = 0;
  outputs->next = nullptr;

  inputs->name = av_strdup("out");
  inputs->filter_ctx = buffersink_ctx;
  inputs->pad_idx = 0;
  inputs->next = nullptr;

  if ((ret = avfilter_graph_parse_ptr(filter_graph, filters_desc,
                                      &inputs, &outputs, nullptr)) < 0)
  {
    std::cerr << "Parse graph failed: " <<
      av_make_error_string(error_msg, sizeof(error_msg), ret) << std::endl;
    goto cleanup_and_return;
  }

  if ((ret = avfilter_graph_config(filter_graph, nullptr)) < 0) {
    std::cerr << "Graph config failed" << std::endl;
    goto cleanup_and_return;
  }

cleanup_and_return:
  avfilter_inout_free(&inputs);
  avfilter_inout_free(&outputs);

  return ret;
}

int init_filters_ex(const AVFrame* inFrame1, const AVFrame* inFrame2)
{
  int ret = 0;
  AVFilterInOut *inputs = nullptr;
  AVFilterInOut *outputs = nullptr;
  char filter_str[1024] = { 0 };

  filter_graph = avfilter_graph_alloc();
  if (!filter_graph) {
    std::cerr << "Error: allocate filter graph failed" << std::endl;
    return -1;
  }

  snprintf(filter_str, sizeof(filter_str),
    "buffer=video_size=%dx%d:pix_fmt=%d:time_base=1/25:pixel_aspect=1/1 [in_1];"
    "buffer=video_size=%dx%d:pix_fmt=%d:time_base=1/25:pixel_aspect=0/1 [in_2];"
    "[in_1] [in_2] overlay=%d:%d [result]; [result] buffersink",
    inFrame1->width, inFrame1->height, inFrame1->format,
    inFrame2->width, inFrame2->height, inFrame2->format,
    5, 5);

  ret = avfilter_graph_parse2(filter_graph, filter_str, &inputs, &outputs);
  if (ret < 0) {
    std::cerr << "Cannot parse graph" << std::endl;
    return ret;
  }

  ret = avfilter_graph_config(filter_graph, nullptr);
  if (ret < 0) {
    std::cerr << "Cannot configure graph" << std::endl;
    return ret;
  }

  return 0;
}

int SaveToI420P(const std::string& filename, const AVFrame* frame)
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

AVFrame* GetFrameFromYUVFile(const std::string& filename, int width, int height, int format)
{
  std::ifstream infile(filename, std::ifstream::binary);
  std::stringstream iss;
  iss << infile.rdbuf();
  std::string yuv_data = iss.str();

  AVFrame *frame = av_frame_alloc();
  int required_size = av_image_fill_arrays(frame->data, frame->linesize,
    (const uint8_t*)yuv_data.c_str(), (AVPixelFormat) format, width, height, 1);
  frame->width = width;
  frame->height = height;
  frame->format = format;

  return frame;
}

int main(int argc, char* argv[])
{
  const char filename[] = "G:/av-learning/bin/win32/test_1280x720_yuv420p.yuv";
  const char logo[] = "G:/av-learning/bin/win32/logo.yuv";

  AVFrame* mainFrame = GetFrameFromYUVFile(filename, 1280, 719, AV_PIX_FMT_YUV420P);
  AVFrame* logoFrame = GetFrameFromYUVFile(logo, 400, 120, AV_PIX_FMT_YUV420P);
  AVFrame* filte_frame = av_frame_alloc();

  int ret = 0;
  // filter_desc can be the following filter
  //
  // lutyuv='u=128:v=128'
  // hfilp
  // boxblur
  // crop=2/3*in_w:2/3*in_h
  //const char filter_desc[] = "lutyuv='u=128:v=128'";
  //ret = init_filters(filter_desc, frame);

  init_filters_ex(mainFrame, logoFrame);

  // add AVFrame to source buffer filter
  ret = av_buffersrc_add_frame(buffersrc_ctx, mainFrame);
  ret = av_buffersrc_add_frame(buffersrc_ctx, logoFrame);

  // get AVFrame from sink buffer filter
  ret = av_buffersink_get_frame(buffersink_ctx, filte_frame);

  SaveToI420P("test-output.yuv", filte_frame);

  av_frame_free(&mainFrame);
  av_frame_free(&logoFrame);
  av_frame_free(&filte_frame);
  avfilter_graph_free(&filter_graph);
  
  return 0;
}