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
#include "libavutil/time.h"
}
#pragma comment(lib, "avutil.lib")

std::size_t audio_callback_count = 0;
int64_t audio_callback_time = 0;
using frame_type = std::vector<char>;

void audio_callback(void *opaque, uint8_t* stream, int len)
{
  auto queue = static_cast<Queue<frame_type>*>(opaque);
  frame_type frame;
  audio_callback_time = av_gettime_relative();

  int len1 = 0;
  while (len > 0)
  {
    frame = queue->Pop();
    len1 = frame.size();

    if (len1 > len) {
      len1 = len;
      std::cout << "last len: " << len1 << std::endl;
    }

    memcpy(stream, &frame[0], len1);

    len -= len1;
    stream += len1;
  }
  std::cout << CurrentTimeStr() << " | " << ++audio_callback_count << std::endl;
}

int main(int argc, char* argv[])
{
  if (SDL_Init(SDL_INIT_AUDIO))
  {
    std::cout << "SDL Init failed: " << SDL_GetError() << std::endl;
    exit(EXIT_FAILURE);
  }

  SDL_AudioSpec wanted_spec;
  SDL_AudioSpec obtained_spec;
  Queue<frame_type> queue(80);

  int sampleRate = 48000;
  int channels = 2;
  int samples = 1024;

  wanted_spec.freq = sampleRate;
  wanted_spec.format = AUDIO_S16SYS;
  wanted_spec.channels = channels;
  wanted_spec.silence = 0;
  wanted_spec.samples = samples;
  wanted_spec.callback = audio_callback;
  wanted_spec.userdata = &queue;

  const char filename[] = "test_s16le.pcm";
  std::ifstream infile(filename, std::ios::binary);

  SDL_AudioDeviceID deviceId = SDL_OpenAudioDevice(nullptr, 0, &wanted_spec, &obtained_spec, 1);
  if (deviceId < 2) {
    std::cerr << "Oen Audio Device Failed: " << SDL_GetError() << std::endl;
    exit(EXIT_FAILURE);
  }

  SDL_PauseAudioDevice(deviceId, 0); // Start to play

  const int buffer_size = samples * channels * (16 / 8);
  frame_type frame;
  frame.reserve(buffer_size);
  frame.resize(buffer_size);
  std::fill(frame.begin(), frame.end(), 0);

  while (infile.read((char*) &frame[0], buffer_size)) {
    std::size_t read_size = (std::size_t) infile.gcount();
    frame.resize(read_size);

    frame_type copy;
    copy.reserve(read_size);
    copy.resize(read_size);
    std::copy(frame.begin(), frame.end(), copy.begin());

    queue.Push(copy);
  }

  std::cout << "Reading finished ..." << std::endl;
  std::cin.get();

  SDL_CloseAudio();
  SDL_Quit();

  return 0;
}