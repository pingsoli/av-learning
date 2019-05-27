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

#include "SoundTouch.h"
#ifdef _DEBUG
#pragma comment(lib, "SoundTouchD.lib")
#else
#pragma comment(lib, "SoundTouch.lib")
#endif

std::size_t audio_callback_count = 0;
using frame_type = std::vector<char>;

void audio_callback(void *opaque, uint8_t* stream, int len)
{
  auto queue = static_cast<Queue<frame_type>*>(opaque);
  frame_type frame;

  int len1 = 0;
  while (len > 0) {
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
SDL_AudioDeviceID deviceId = 0;

bool OpenAudio(int sampleRate, int channels, SDL_AudioFormat format, int samples,
               void (*callback)(void*, uint8_t*, int), void* opaque)
{
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

  return true;
}

int main(int argc, char* argv[])
{
  if (SDL_Init(SDL_INIT_AUDIO))
  {
    std::cout << "SDL Init failed: " << SDL_GetError() << std::endl;
    exit(EXIT_FAILURE);
  }

  const int max_queue_size = 80;
  int sampleRate = 48000;
  int channels = 2;
  int samples = 1024;
  double speed = 0.5;

  soundtouch::SoundTouch soundTouch;
  soundTouch.setChannels(channels);
  soundTouch.setRate(speed);
  soundTouch.setSampleRate(sampleRate);
  
  Queue<frame_type> queue(max_queue_size);
  const char filename[] = "test_s16le.pcm";
  std::ifstream infile(filename, std::ios::binary);

  if (!OpenAudio(sampleRate, channels, AUDIO_S16SYS, int (samples / speed), audio_callback, &queue)) {
    return -1;
  }
  SDL_PauseAudioDevice(deviceId, 0); // Start to play

  const int buffer_size = samples * channels * (16 / 8);
  frame_type frame;
  frame.reserve(buffer_size);
  std::fill(frame.begin(), frame.end(), 0);
  frame.resize(buffer_size);

  int bytes_per_sec = sampleRate * channels * (16 / 8);
  int nb = 0;

  while (infile.read((char*) &frame[0], buffer_size)) {
    std::size_t read_size = (std::size_t) infile.gcount();
    frame.resize(read_size);

    frame_type copy;
    std::size_t nb_samples = sampleRate * read_size / bytes_per_sec;
    std::size_t size = (std::size_t) (nb_samples * channels * (16 / 8) / speed);
    copy.reserve(size);
    std::fill(copy.begin(), copy.end(), 0);
    copy.resize(size);

    soundTouch.putSamples((const short*) &frame[0], nb_samples);
    nb = soundTouch.receiveSamples((short *)&copy[0], uint (nb_samples / speed));

    queue.Push(copy);
  }

  std::cout << "Reading finished ..." << std::endl;
  std::cin.get();

  SDL_CloseAudio();
  SDL_Quit();

  return 0;
}