#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <mutex>
#include <vector>

#include "Queue.h"
#include "utils.h"

#define SDL_MAIN_HANDLED
#include "SDL.h"
#pragma comment(lib, "SDL2.lib")

extern "C" {
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libavutil/time.h"
#include "libavutil/opt.h"
}
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swresample.lib")

std::size_t audio_callback_count = 0;
using frame_type = AVFrame *;

void audio_callback(void *opaque, uint8_t* stream, int len)
{
  auto queue = static_cast<Queue<frame_type>*>(opaque);
  frame_type frame;

  int len1 = 0;
  while (len > 0)
  {
    frame = queue->Pop();
    len1 = frame->linesize[0];

    if (len1 > len) {
      len1 = len;
      //std::cout << "last len: " << len1 << std::endl;
    }

    memcpy(stream, frame->data[0], len1);

    len -= len1;
    stream += len1;

    if (frame) {
      av_freep(&frame->data[0]);
      av_frame_free(&frame);
    }
  }

  //std::cout << CurrentTimeStr() << " | " << ++audio_callback_count << std::endl;
}

SDL_AudioDeviceID deviceId = 0;

bool OpenAudioAndPlay(int sampleRate, int channels, int format, int samples,
               void (*callback)(void*, uint8_t*, int), void* opaque)
{
  if (SDL_Init(SDL_INIT_AUDIO))
  {
    std::cout << "SDL Init failed: " << SDL_GetError() << std::endl;
    return false;
  }

  if (deviceId >= 2) {
    SDL_CloseAudioDevice(deviceId);
  }

  SDL_AudioSpec wanted_spec;
  SDL_AudioSpec obtained_spec;

  wanted_spec.freq = sampleRate;
  wanted_spec.format = format;
  wanted_spec.channels = channels;
  wanted_spec.silence = 0;
  wanted_spec.samples = samples;
  wanted_spec.callback = callback;
  wanted_spec.userdata = opaque;

  deviceId = SDL_OpenAudioDevice(nullptr, 0, &wanted_spec, &obtained_spec, 1);
  if (deviceId < 2) {
    std::cerr << "Oen Audio Device Failed: " << SDL_GetError() << std::endl;
    return false;
  }

  SDL_PauseAudioDevice(deviceId, 0); // Start to play
  return true;
}

int main(int argc, char* argv[])
{
  int ret_code = 0;
  const char filename[] = "test.wav";
  AVFormatContext *avFormatCtx = nullptr;
  AVCodec *codec = nullptr;
  int audioStreamIdx = -1;
  Queue<frame_type> queue;
  double speed = std::atof(argv[1]);
  std::cout << "speed: " << speed << std::endl;

  if ((ret_code = avformat_open_input(&avFormatCtx, filename, nullptr, nullptr)) != 0) {
    char error_msg_buf[256] = {0};
    av_strerror(ret_code, error_msg_buf, sizeof(error_msg_buf));
    std::cerr << "Error: avformat_open_input failed: " << error_msg_buf << std::endl;
    return false;
  }

  avformat_find_stream_info(avFormatCtx, nullptr);

  audioStreamIdx = av_find_best_stream(avFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
  if (audioStreamIdx < 0) exit(EXIT_FAILURE);

  AVCodecParameters* audioCodecPar = avFormatCtx->streams[audioStreamIdx]->codecpar;
  int sampleRate = audioCodecPar->sample_rate;
  int channels = audioCodecPar->channels;
  int format = audioCodecPar->format;
  int64_t layout = audioCodecPar->channel_layout;

  AVCodecContext *avCodecCtx = avcodec_alloc_context3(codec);
  avcodec_parameters_to_context(avCodecCtx, audioCodecPar);

  ret_code = avcodec_open2(avCodecCtx, codec, nullptr);
  if (ret_code < 0) exit(EXIT_FAILURE);

  //av_dump_format(avFormatCtx, 0, filename, 0);

  SwrContext *swr = swr_alloc_set_opts(nullptr,
                            AV_CH_LAYOUT_STEREO, (AVSampleFormat)AV_SAMPLE_FMT_S16, int(sampleRate / speed),
                            layout, (AVSampleFormat)format, sampleRate,
                            0, nullptr);
  av_opt_set_int(swr, "ich", channels, 0); // input channels count
  av_opt_set_int(swr, "och", channels, 0); // output channels count
  swr_init(swr);

  AVPacket pkt;
  av_init_packet(&pkt);
  pkt.data = nullptr;
  pkt.size = 0;
  int64_t total_bytes = 0;
  int nb_samples_per_frame = 0;

  while (true) {
    ret_code = av_read_frame(avFormatCtx, &pkt);
    if (ret_code != 0) break;

    ret_code = avcodec_send_packet(avCodecCtx, &pkt);
    av_packet_unref(&pkt);

    while (true) {
      AVFrame *frame = av_frame_alloc();
      ret_code = avcodec_receive_frame(avCodecCtx, frame);
      if (ret_code == 0) {

        if (deviceId < 2) {
          nb_samples_per_frame = int(frame->nb_samples / speed);
          OpenAudioAndPlay(sampleRate, channels, AUDIO_S16SYS, nb_samples_per_frame, audio_callback, &queue);
        }

        AVFrame *outFrame = av_frame_alloc();
        outFrame->channel_layout = layout;
        outFrame->format = format;
        outFrame->nb_samples = int(frame->nb_samples / speed);
        av_samples_alloc(outFrame->data, outFrame->linesize, channels, nb_samples_per_frame, (AVSampleFormat)format, 0);
        swr_convert(swr, outFrame->data, nb_samples_per_frame,
          (const uint8_t **)frame->data, frame->nb_samples);

        queue.Push(outFrame);
        total_bytes += frame->linesize[0];
        av_frame_free(&frame);

      } else {
        break;
      }
    }
  }

  std::cout << "total bytes: " << total_bytes << std::endl;

  swr_free(&swr);
  SDL_CloseAudio();
  SDL_Quit();

  return 0;
}