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

int SaveFrameToJPEG(const AVFrame* frame, const char* filename)
{
  char error_msg_buf[256] = { 0 };

  AVCodec *jpegCodec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
  if (!jpegCodec) return -1;
  AVCodecContext *jpegCodecCtx = avcodec_alloc_context3(jpegCodec);
  if (!jpegCodecCtx) return -2;

  AVFrame *copyFrame = av_frame_alloc();
  copyFrame->format = frame->format;
  copyFrame->width = frame->width;
  copyFrame->height = frame->height;
  av_image_alloc(copyFrame->data, copyFrame->linesize,
    copyFrame->width, copyFrame->height, (AVPixelFormat) copyFrame->format, 1);

  jpegCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P; // NOTE: Don't use AV_PIX_FMT_YUV420P
  jpegCodecCtx->width = copyFrame->width;
  jpegCodecCtx->height = copyFrame->height;

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
    av_packet_unref(&pkt);
    break;
  }

  avcodec_free_context(&jpegCodecCtx);
  av_frame_free(&copyFrame);
  return 0;
}

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
    "buffer=video_size=%dx%d:pix_fmt=%d:time_base=1/1:pixel_aspect=0/1[in];"
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
  if (ret < 0) goto cleanup;
  ret = av_buffersink_get_frame(sinkFilterCtx, frame);
  if (ret < 0) goto cleanup;

cleanup:
  avfilter_graph_free(&avFilterGraph);
  if (ret < 0) {
    av_frame_free(&frame);
    frame = nullptr;
  }
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
  SaveFrameToJPEG(cropped_frame, "thor_out.jpg");
  getchar();

  av_frame_free(&frame);
  av_frame_free(&cropped_frame);
  return 0;
}