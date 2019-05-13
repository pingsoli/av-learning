#include <fstream>
#include <iostream>
#include <chrono>
#include <sstream>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <ctime>

#include "Queue.h"

#define SDL_MAIN_HANDLED
#include "SDL.h"
#pragma comment(lib, "SDL2.lib")

extern "C" {
#include "libavutil/time.h"
}
#pragma comment(lib, "avutil.lib")

using frame_type = std::vector<uint8_t>;

char* CurrentTimeStr()
{
  auto now = std::chrono::system_clock::now();
  auto mills = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
  std::time_t t = std::time(nullptr);

  static char buf[24] = { 0 };
  std::size_t len = std::strftime(buf, sizeof(buf), "%F %T", std::localtime(&t));
  std::sprintf(buf + len, ".%03lld", mills % 1000);
  return buf;
}

std::size_t audio_callback_count = 0;
int64_t audio_callback_time = 0;

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
  Queue<frame_type> queue(80);

  if (SDL_Init(SDL_INIT_AUDIO))
  {
    std::cout << "SDL Init failed: " << SDL_GetError() << std::endl;
    exit(EXIT_FAILURE);
  }

  SDL_AudioSpec wanted_spec;
  SDL_AudioSpec obtained_spec;

  wanted_spec.freq = 48000;
  wanted_spec.format = AUDIO_S16SYS;
  wanted_spec.channels = 2;
  wanted_spec.callback = audio_callback;
  wanted_spec.userdata = &queue;

  const char filename[] = "F:/av-learning/bin/win32/test.pcm";
  std::ifstream infile(filename, std::ios::binary);

  SDL_AudioDeviceID deviceId = SDL_OpenAudioDevice(nullptr, 0, &wanted_spec, &obtained_spec, 1);
  if (deviceId < 2) {
    std::cerr << "Oen Audio Device Failed: " << SDL_GetError() << std::endl;
    exit(EXIT_FAILURE);
  }

  SDL_PauseAudioDevice(deviceId, 0); // Start to play

  const int buffer_size = 13107;
  frame_type frame;
  frame.reserve(buffer_size);
  std::fill(frame.begin(), frame.end(), 0);
  frame.resize(buffer_size);

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