#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersrc.h"
#include "libavfilter/buffersink.h"
}

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avfilter.lib")

extern "C" {
#include "libyuv.h"
}

#pragma comment(lib, "yuv.lib")

int SaveFrameToJPEG(const AVFrame* frame, const char* filename)
{
  char error_msg_buf[256] = { 0 };

  AVCodec *jpegCodec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
  if (!jpegCodec) return -1;
  AVCodecContext *jpegCodecCtx = avcodec_alloc_context3(jpegCodec);
  if (!jpegCodecCtx) return -2;

  AVFrame *copyFrame = av_frame_alloc();
  copyFrame->format = frame->format;
  copyFrame->width = frame->width / 2;
  copyFrame->height = frame->height / 2;
  av_image_alloc(copyFrame->data, copyFrame->linesize,
    copyFrame->width, copyFrame->height, (AVPixelFormat) copyFrame->format, 1);

  auto start = std::chrono::high_resolution_clock::now();
  libyuv::I420Scale(
    frame->data[0], frame->linesize[0],
    frame->data[1], frame->linesize[1],
    frame->data[2], frame->linesize[2],
    frame->width, frame->height,
    copyFrame->data[0], copyFrame->linesize[0],
    copyFrame->data[1], copyFrame->linesize[1],
    copyFrame->data[2], copyFrame->linesize[2],
    frame->width / 2, frame->height / 2, libyuv::kFilterNone);
  auto end = std::chrono::high_resolution_clock::now();
  std::cout << "scale time: "
    << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
    << "ms\n";

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

  ret = avcodec_send_frame(jpegCodecCtx, copyFrame);
  av_frame_free(&copyFrame);
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
  }

  avcodec_free_context(&jpegCodecCtx);
  return 0;
}

int main(int argc, char* argv[])
{
  char filename[] = "F:/av-learning/bin/win32/thor.mp4";
  AVFormatContext *avFormatCtx = nullptr;
  avFormatCtx = avformat_alloc_context();
  int ret = 0;
  char error_msg_buf[256] = { 0 };
  int videoStreamIdx = -1;

  if ((ret = avformat_open_input(&avFormatCtx, filename, nullptr, nullptr)) != 0) {
    std::cerr << "Error: open failed: " << av_make_error_string(error_msg_buf, sizeof(error_msg_buf), ret);
    exit(EXIT_FAILURE);
  }

  avformat_find_stream_info(avFormatCtx, nullptr);
  if ((videoStreamIdx = av_find_best_stream(avFormatCtx, AVMEDIA_TYPE_VIDEO, -1, 0, nullptr, 0)) < 0)
  {
    std::cerr << "Error: couldn't find video stream" << std::endl;
    exit(EXIT_FAILURE);
  }

  AVCodec* videoCodec = avcodec_find_decoder(avFormatCtx->streams[videoStreamIdx]->codecpar->codec_id);
  AVCodecContext *videoCodecCtx = avcodec_alloc_context3(nullptr);
  avcodec_parameters_to_context(videoCodecCtx, avFormatCtx->streams[videoStreamIdx]->codecpar);
  if ((ret = avcodec_open2(videoCodecCtx, videoCodec, nullptr)))
  {
    std::cerr << "Error: couldn't open decoder" << std::endl;
    exit(EXIT_FAILURE);
  }

  AVPacket* pkt = av_packet_alloc();
  AVFrame* frame = av_frame_alloc();
  int count = 0;
  while (true) {
    ret = av_read_frame(avFormatCtx, pkt);
    if (ret == AVERROR_EOF) break;
    if (ret == AVERROR(EAGAIN)) continue;

    // only process picture 
    if (pkt->stream_index != videoStreamIdx) {
      av_packet_unref(pkt);
      continue;
    }

    ret = avcodec_send_packet(videoCodecCtx, pkt);
    av_packet_unref(pkt);
    if (ret == AVERROR(EAGAIN)) continue;

    while (ret >= 0) {
      ret = avcodec_receive_frame(videoCodecCtx, frame);
      switch (ret)
      {
      case 0: // success
        if (frame->key_frame) {
          static char out_filename[256] = { 0 };
          sprintf(out_filename, "test-%06d.jpg", count++);
          auto start = std::chrono::high_resolution_clock::now();
          //AVFrame *cropFrame = CropFrame(frame, 0, 710, frame->width, 100);
          SaveFrameToJPEG(frame, out_filename);
          auto end = std::chrono::high_resolution_clock::now();
          std::cout << count << ": "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;
          //av_frame_free(&cropFrame);
        }
        break;
      case AVERROR(EAGAIN): // buffer is not available now, try agagin
        continue;
      default: // error
        av_strerror(ret, error_msg_buf, sizeof(error_msg_buf));
        std::cerr << "avcodec receive frame failed: " << error_msg_buf << std::endl;
      }
      av_frame_unref(frame);
    }
  }

  return 0;
}