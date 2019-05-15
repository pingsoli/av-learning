// Add watermark on picture
// references
// 1. https://blog.csdn.net/leixiaohua1020/article/details/29368911
// 2. https://blog.csdn.net/li_wen01/article/details/62442162
//
// The process the filter
// source buffer filter 0 (main)
//   -> source buffer 1 (logo)
//   -> overlay filter (real process for overlaying)
//   -> sink buffer filter (result you want get)
//
// Tips: yuvplayer will be helpful, you can downloading it from sourceforge.

#include <iostream>

#include "utils.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersrc.h"
#include "libavfilter/buffersink.h"
}

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avfilter.lib")

AVFilterContext *mainsrc_ctx;
AVFilterContext *logosrc_ctx;
AVFilterContext *resultsink_ctx;
AVFilterGraph *filter_graph;

//int init_filters(const char* filters_desc, const AVFrame* frame)
//{
//  char args[512] = { 0 };
//  int ret = 0;
//  char error_msg[256] = { 0 };
// 
//  const AVFilter *buffersrc = avfilter_get_by_name("buffer"); // input buffer filter
//  const AVFilter *buffersink = avfilter_get_by_name("buffersink"); // output buffer filter
//  AVFilterInOut *inputs = avfilter_inout_alloc();
//  AVFilterInOut *outputs = avfilter_inout_alloc();
//  AVRational time_base = { 1, 25 }; // It's suitable for this example, maybe not suitable for you.
//
//  filter_graph = avfilter_graph_alloc();
//
//  // buffer video source: the decoded frames from the decoder will be inserted here
//  snprintf(args, sizeof(args),
//    "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
//    frame->width, frame->height, frame->format,
//    time_base.num, time_base.den, 1, 1);
//  ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in", args, nullptr, filter_graph);
//  if (ret < 0) {
//    std::cerr << "Cannot create buffer source" << std::endl;
//    goto cleanup_and_return;
//  }
//
//  ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
//                                     nullptr, nullptr, filter_graph);
//  if (ret < 0) {
//    std::cerr << "Cannot create buffer sink: " <<
//      av_make_error_string(error_msg, sizeof(error_msg), ret) << std::endl;
//    goto cleanup_and_return;
//  }
//
//  outputs->name = av_strdup("in");
//  outputs->filter_ctx = buffersrc_ctx;
//  outputs->pad_idx = 0;
//  outputs->next = nullptr;
//
//  inputs->name = av_strdup("out");
//  inputs->filter_ctx = buffersink_ctx;
//  inputs->pad_idx = 0;
//  inputs->next = nullptr;
//
//  if ((ret = avfilter_graph_parse_ptr(filter_graph, filters_desc,
//                                      &inputs, &outputs, nullptr)) < 0)
//  {
//    std::cerr << "Parse graph failed: " <<
//      av_make_error_string(error_msg, sizeof(error_msg), ret) << std::endl;
//    goto cleanup_and_return;
//  }
//
//  if ((ret = avfilter_graph_config(filter_graph, nullptr)) < 0) {
//    std::cerr << "Graph config failed" << std::endl;
//    goto cleanup_and_return;
//  }
//
//cleanup_and_return:
//  avfilter_inout_free(&inputs);
//  avfilter_inout_free(&outputs);
//
//  return ret;
//}

int init_filters_ex(const AVFrame* inFrame1, const AVFrame* inFrame2, int x, int y)
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
    "buffer=video_size=%dx%d:pix_fmt=%d:time_base=1/25:pixel_aspect=%d/%d[main];" // Parsed_buffer_0
    "buffer=video_size=%dx%d:pix_fmt=%d:time_base=1/25:pixel_aspect=%d/%d[logo];" // Parsed_bufer_1
    "[main][logo]overlay=%d:%d[result];" // Parsed_overlay_2
    "[result]buffersink", // Parsed_buffer_sink_3
    inFrame1->width, inFrame1->height, inFrame1->format, inFrame1->sample_aspect_ratio.num, inFrame1->sample_aspect_ratio.den,
    inFrame2->width, inFrame2->height, inFrame2->format, inFrame2->sample_aspect_ratio.num, inFrame2->sample_aspect_ratio.den,
    x, y);

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

int main(int argc, char* argv[])
{
  AVFrame *mainFrame = GetFrameFromPicture("out.jpeg");
  AVFrame *logoFrame = GetFrameFromPicture("logo.jpg");

  int ret = 0;
  // filter_desc can be the following filter
  //
  // lutyuv='u=128:v=128'
  // hfilp
  // boxblur
  // crop=2/3*in_w:2/3*in_h
  //const char filter_desc[] = "lutyuv='u=128:v=128'";
  //ret = init_filters(filter_desc, frame);

  init_filters_ex(mainFrame, logoFrame, 100, 200);

  // Get AVFilterContext from AVFilterGraph parsing from string
  mainsrc_ctx = avfilter_graph_get_filter(filter_graph, "Parsed_buffer_0");
  logosrc_ctx = avfilter_graph_get_filter(filter_graph, "Parsed_buffer_1");
  resultsink_ctx = avfilter_graph_get_filter(filter_graph, "Parsed_buffersink_3");

  // Fill data to buffer filter context(main, logo)
  av_buffersrc_add_frame(mainsrc_ctx, mainFrame);
  av_buffersrc_add_frame(logosrc_ctx, logoFrame);

  // Get AVFrame from sink buffer filter(result)
  AVFrame* result_frame = av_frame_alloc();
  ret = av_buffersink_get_frame(resultsink_ctx, result_frame);

  // Save AVFrame to picture and check
  SaveFrameToJPEG("test-output.jpeg", result_frame);

  av_frame_free(&mainFrame);
  av_frame_free(&logoFrame);
  av_frame_free(&result_frame);
  avfilter_graph_free(&filter_graph);
  
  return 0;
}