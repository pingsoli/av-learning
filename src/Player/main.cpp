#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <fstream>
#include <ctime>

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
#include "libavutil/opt.h"
#include "libswresample/swresample.h"
#include "libavutil/imgutils.h"
#include "libavutil/error.h"
#include "libavutil/time.h"
#include "libswscale/swscale.h"
}

#include "MediaInfo.h"
#include "Decoder.h"
#include "Queue.h"
#include "SDLAudioPlayer.h"
#include "SDLVideoPlayer.h"
#include "Resampler.h"

#include "SDL.h"

struct PlayState {
  Queue<AVFrame*> *samplesQueue;
  Queue<AVFrame*> *picturesQueue;

  double audioClock;
  double videoClock;

  int bytes_per_sec; // audio only
};

void audio_callback(void *opaque, uint8_t* stream, int len) {
  PlayState *state = (PlayState*)opaque;
  Queue<AVFrame*> *queue = state->samplesQueue;
  std::size_t data_size = 0;
  uint8_t* audio_buf = nullptr;
  int len1 = 0;

  AVFrame *frame = nullptr;
  while (len > 0) {
    frame = queue->Pop();

    data_size = av_samples_get_buffer_size(nullptr,
      frame->channels, frame->nb_samples, (AVSampleFormat)frame->format, 1);
    audio_buf = frame->data[0];
    len1 = data_size;
    if (len1 > len)
      len1 = len;

    memcpy(stream, audio_buf, len1);

    len -= len1;
    stream += len1;
  }

  state->audioClock = frame->pts * av_q2d({1, frame->sample_rate})
    + (double) frame->nb_samples / frame->sample_rate;

  // set audio clock
  // std::cout << "audio clock: " << state->audioClock << std::endl;

  av_frame_free(&frame);
};

int main(int argc, char *argv[])
{
  MediaInfo mediaInfo;
  Decoder decoder;
  SDLAudioPlayer audioPlayer;
  Resampler resampler;
  SDLVideoPlayer videoPlayer;

  Queue<AVPacket*> videoPktQueue;
  Queue<AVPacket*> audioPktQueue;

  Queue<AVFrame*> audioFrameQueue(30);
  Queue<AVFrame*> videoFrameQueue;

  PlayState state;
  state.samplesQueue = &audioFrameQueue;
  state.picturesQueue = &videoFrameQueue;
  state.audioClock = 0.0;
  state.videoClock = 0.0;

  std::atomic_bool isPlaying = true;
  std::atomic_bool isSilent = false;

  mediaInfo.Open("src.mp4");
  decoder.Open(mediaInfo);

  int channels = mediaInfo.GetAudioCodecParameters()->channels;
  int sample_rate = mediaInfo.GetAudioCodecParameters()->sample_rate;
  int sampleFormat = mediaInfo.GetAudioCodecParameters()->format;

  int video_timebase = mediaInfo.GetVideoTimebase();
  int audio_timebase = mediaInfo.GetAudioTimebase();

  resampler.Init(AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, sample_rate,
                 AV_CH_LAYOUT_STEREO, (AVSampleFormat)sampleFormat, sample_rate);
  audioPlayer.Init(sample_rate, "s16", channels, audio_callback, &state);
  videoPlayer.Init(mediaInfo.Filename(), mediaInfo.Width(), mediaInfo.Height());

  std::thread demuxThread(
    [&mediaInfo, &isPlaying, &audioPktQueue, &videoPktQueue] {
      AVPacket* tmpPkt;
      while (isPlaying) {

        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.data = nullptr;
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

  std::thread audioDecodeThread(
    [&audioPktQueue, &isPlaying, &isSilent, &decoder, &audioPlayer, &resampler, &mediaInfo, &audioFrameQueue] {
    int count = 0;

    audioPlayer.Play();
    AVFrame *convFrame = nullptr;
    AVFrame* frame = av_frame_alloc();
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
      
      while ((ret = avcodec_receive_frame(decoder.GetAudioCodecContext(), frame)) == 0) {
        convFrame = av_frame_clone(frame);
        resampler.Convert(convFrame, frame);
        audioFrameQueue.Push(convFrame);
      }
    }
  });

  std::thread videoDecodeThread(
    [&isPlaying, &decoder, &videoPktQueue, &videoPlayer, &videoFrameQueue] {

    AVFrame *frame = av_frame_alloc();
    AVFrame *copyFrame = nullptr;
      while (isPlaying) {
        AVPacket* pkt = videoPktQueue.Pop();
        int ret = avcodec_send_packet(decoder.GetVideoCodecContext(), pkt);
        av_packet_free(&pkt);
        if (ret < 0) break;

        while ((ret = avcodec_receive_frame(decoder.GetVideoCodecContext(), frame)) == 0) {
          copyFrame = av_frame_clone(frame);
          videoFrameQueue.Push(copyFrame);
        }
      }
    }
  );

  std::thread displayThread(
    [&videoFrameQueue, &isPlaying, &videoPlayer, &video_timebase, &state] () {
      int64_t last_pts = 0;
      double pts = 0.0;
      double duration = 0.0;
      double delay = 0.0;
      double diff = 0.0;
      double time = 0;
      double frame_timer = 0;
      double remaining_time = 0;
      int force_refresh = 0;
      AVFrame *frame = nullptr;
      
      while (true) {

        if (!isPlaying) {
          std::this_thread::sleep_for(std::chrono::microseconds(10));
          continue;
        }
        remaining_time = 0.01;
        frame = videoFrameQueue.Pop();

        pts = frame->pts * av_q2d({1, video_timebase});
        duration = pts - last_pts * av_q2d({1, video_timebase});

wait_to_play:
        // delay policy
        diff = pts - state.audioClock;
        delay = duration;
        if (diff >= 0.1 && duration > 0.1) {
          delay += diff;
        } else if (diff >= 0.1) {
          delay *= 2;
        }

        // std::cout <<
        //   "diff: " << diff << " | "
        //   "delay time: " << delay
        //   << std::endl;

        time = av_gettime_relative() / 1000000.0;
        if (time < frame_timer + delay) {
          remaining_time = std::min(frame_timer - time + delay, remaining_time);
          std::cout << "remaining time: " << remaining_time << std::endl;
          goto display;
        }

        frame_timer += delay;
        if (delay > 0 && time - frame_timer > 0.1) {
          frame_timer = time;
        }

        force_refresh = 1;
display:
        if (force_refresh) {
          videoPlayer.Render(frame);
          last_pts = frame->pts;
          av_frame_free(&frame);
          force_refresh = 0;
          std::this_thread::sleep_for(std::chrono::microseconds((int64_t)(remaining_time * 1000000.0)));
          // std::cout << "sleep time: " << remaining_time << std::endl;
          continue;
        } else {
          std::this_thread::sleep_for(std::chrono::microseconds((int64_t)(remaining_time * 1000000.0)));
          // std::cout << "sleep time: " << remaining_time << std::endl;
          remaining_time = 0.01;
        }
        goto wait_to_play;
      }
    }
  );

  SDL_Event event;
  while (true) {
    while (SDL_PollEvent(&event)) {
      switch(event.type) {
        case SDL_KEYDOWN:
          //std::cout << "Key down" << std::endl;
          break;
        case SDL_MOUSEMOTION:
          //std::cout << "Mouse motion" << std::endl;
          break;
        default:
          break;
      }
    }
  }

  demuxThread.join();
  audioDecodeThread.join();
  videoDecodeThread.join();
  displayThread.join();

  return 0;
}