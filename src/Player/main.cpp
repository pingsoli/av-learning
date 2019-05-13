#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <fstream>
#include <ctime>

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swresample.lib") // audio resample
#pragma comment(lib, "swscale.lib")

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
#include "libavutil/opt.h"
#include "libswresample/swresample.h"
#include "libavutil/imgutils.h"
#include "libavutil/error.h"
#include "libswscale/swscale.h"
}

#include "MediaInfo.h"
#include "Decoder.h"
#include "Queue.h"
#include "SDLAudioPlayer.h"
#include "SDLVideoPlayer.h"
#include "Resampler.h"

#define SDL_MAIN_HANDLED
#include "SDL.h"
#pragma comment(lib, "SDL2.lib")

int SaveFrameToJPEG(const AVFrame* frame, const char* filename, float ratio)
{
  char error_msg_buf[256] = { 0 };

  AVFrame *outFrame = av_frame_alloc();
  int dstWidth = static_cast<int>(frame->width * ratio);
  int dstHeight = static_cast<int>(frame->height * ratio);

  // NOTE: the outFrame data buffer must be aligned.
  av_image_alloc(outFrame->data, outFrame->linesize, dstWidth, dstHeight, (AVPixelFormat)frame->format, 1);
  outFrame->width = dstWidth;
  outFrame->height = dstHeight;
  outFrame->format = frame->format;

  SwsContext* swsContext = sws_getContext(
    frame->width, frame->height, (AVPixelFormat) frame->format,
    outFrame->width, outFrame->height, (AVPixelFormat) outFrame->format,
    SWS_BICUBIC, nullptr, nullptr, nullptr);

  //auto scale_parallel = [&swsContext, &frame, &copyFrame](int height) {
  //  sws_scale(swsContext, frame->data, frame->linesize, 0, height, copyFrame->data, copyFrame->linesize);
  //};
  //std::thread t1(scale_parallel, frame->height / 2);
  //std::thread t2(scale_parallel, frame->height);s

  int dh = sws_scale(swsContext, frame->data, frame->linesize, 0, frame->height, outFrame->data, outFrame->linesize);

  AVCodec *jpegCodec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
  if (!jpegCodec) return -1;
  AVCodecContext *jpegCodecCtx = avcodec_alloc_context3(jpegCodec);
  if (!jpegCodecCtx) return -2;

  jpegCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P; // NOTE: Don't use AV_PIX_FMT_YUV420P
  jpegCodecCtx->width = dstWidth;
  jpegCodecCtx->height = dstHeight;
  jpegCodecCtx->time_base = { 1, 25 };
  jpegCodecCtx->framerate = { 25, 1 };

  int ret = avcodec_open2(jpegCodecCtx, jpegCodec, nullptr);
  if (ret < 0) {
    std::cerr << "avcodec open failed" << std::endl;
    ret = -3;
    goto cleanup;
  }

  AVPacket pkt;
  av_init_packet(&pkt);

  ret = avcodec_send_frame(jpegCodecCtx, outFrame);
  if (ret < 0) {
    std::cerr << "Error: " <<
      av_make_error_string(error_msg_buf, sizeof(error_msg_buf), ret) << std::endl;
    ret = -4;
    goto cleanup;
  }

  while (ret >= 0) {
    ret = avcodec_receive_packet(jpegCodecCtx, &pkt);
    if (ret == AVERROR(EAGAIN)) continue;
    if (ret == AVERROR_EOF) break;

    std::ofstream outfile(filename, std::ios::binary);
    outfile.write((char*) pkt.data, pkt.size);
  }

cleanup:
  av_frame_free(&outFrame);
  avcodec_free_context(&jpegCodecCtx);
  sws_freeContext(swsContext);
  return ret;
}

void PrintCurrentTime()
{
  auto currentTime = std::chrono::system_clock::now();
  static char buffer[80];

  auto transformed = currentTime.time_since_epoch().count() / 1000000;
  auto millis = transformed % 1000;
  std::time_t tt;
  tt = std::chrono::system_clock::to_time_t(currentTime);
  strftime(buffer, sizeof(buffer), "%F %H:%M:%S", localtime(&tt));
  printf("%s.%03d\n", buffer, (int) millis);
}

int main(int argc, char *argv[])
{
  MediaInfo mediaInfo;
  Decoder decoder;
  SDLAudioPlayer audioPlayer;
  Resampler resampler;
  SDLVideoPlayer videoPlayer;

  Queue<AVPacket*> videoPktQueue;
  Queue<AVPacket*> audioPktQueue;
  //Queue<AVFrame> videoFrameQueue;

  std::atomic_bool isPlaying = true;
  std::atomic_bool isSilent = false;

  mediaInfo.Open("src.mp4");
  decoder.Open(mediaInfo);

  int channels = mediaInfo.GetAudioCodecParameters()->channels;
  int sample_rate = mediaInfo.GetAudioCodecParameters()->sample_rate;
  int sampleFormat = mediaInfo.GetAudioCodecParameters()->format;

  resampler.Init(AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, sample_rate,
                 AV_CH_LAYOUT_STEREO, (AVSampleFormat)sampleFormat, sample_rate);
  audioPlayer.Init(sample_rate, "s16", channels);
  videoPlayer.Init(mediaInfo.Filename(), mediaInfo.Width(), mediaInfo.Height());

  std::thread decodeThread(
    [&mediaInfo, &audioPktQueue, &isPlaying, &videoPktQueue] {

      AVPacket* tmpPkt;
      while (isPlaying) {
        AVPacket pkt;
        pkt.size = 0;

        int r = av_read_frame(mediaInfo.GetFormatContext(), &pkt);
        if (r < 0) break;

        tmpPkt = av_packet_clone(&pkt);
        if (pkt.stream_index == mediaInfo.VideoStreamIndex()) {
          videoPktQueue.Push(tmpPkt);
        } else if (pkt.stream_index == mediaInfo.AudioStreamIndex()) {
          audioPktQueue.Push(tmpPkt);
        } else {
          std::cerr << "Warning: decoding a packet from unknown stream" << std::endl;
        }
        av_packet_unref(&pkt);
      }
    }
  );

  std::thread audioThread(
    [&audioPktQueue, &isPlaying, &isSilent, &decoder, &audioPlayer, &resampler, &mediaInfo] {
    AVFrame* frame = av_frame_alloc();
    int count = 0;

    // std::ofstream outfile("test.dat", std::ofstream::binary | std::ofstream::app);
    uint8_t *pcm = new uint8_t[8192];

    audioPlayer.Play();
    while (isPlaying && !isSilent)
    {
      AVPacket *pkt = audioPktQueue.Pop();
      int ret = avcodec_send_packet(decoder.GetAudioCodecContext(), pkt);
      av_packet_free(&pkt);

      if (ret != 0) {
        char error_msg_buf[256] = { 0 };
        av_strerror(ret, error_msg_buf, sizeof(error_msg_buf));
        std::cerr << "Error: send packet to audio decoder failed: " << error_msg_buf << std::endl;
        continue;
      }

      while ((ret = avcodec_receive_frame(decoder.GetAudioCodecContext(), frame)) == 0)
      {
        int ret = resampler.Convert(pcm, frame);
        int dst_linesize = 0;
        int dst_bufsize = av_samples_get_buffer_size(&dst_linesize,
          2, ret, AV_SAMPLE_FMT_S16, 0);
        // PrintCurrentTime();
        audioPlayer.Queue(pcm, dst_bufsize);
      }
    }
  });

  std::thread videoThread(
    [&isPlaying, &decoder, &videoPktQueue, &videoPlayer] {
      AVFrame* frame = av_frame_alloc();

      int count = 0;
      while (isPlaying) {
        AVPacket* pkt = videoPktQueue.Pop();
        int ret = avcodec_send_packet(decoder.GetVideoCodecContext(), pkt);
        av_packet_free(&pkt);
        if (ret < 0) break;

        while ((ret = avcodec_receive_frame(decoder.GetVideoCodecContext(), frame)) == 0) {
          videoPlayer.Render(frame);
          std::this_thread::sleep_for(std::chrono::milliseconds(33));

          if (frame->key_frame) {
            static char filename[256] = { 0 };
            sprintf(filename, "test-%06d.jpg", count++);
            SaveFrameToJPEG(frame, filename, 1);
            exit(0);
          }
        }
      }
    }
  );

  SDL_Event event;
  while (true) {
    while (SDL_PollEvent(&event)) {
      switch(event.type) {
        case SDL_KEYDOWN:
          std::cout << "Key down" << std::endl;
          break;
        case SDL_MOUSEMOTION:
          std::cout << "Mouse motion" << std::endl;
          break;
        default:
          std::cout << "Unknown event" << std::endl;
      }
    }
  }

  decodeThread.join();
  audioThread.join();
  videoThread.join();

  return 0;
}