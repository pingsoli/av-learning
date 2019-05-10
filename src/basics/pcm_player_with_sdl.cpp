// References
// https://github.com/leixiaohua1020/simplest_media_play/blob/master/simplest_audio_play_sdl2/simplest_audio_play_sdl2.cpp
// NOTE: the last pcm data will not be played

#include <fstream>
#include <iostream>
#include <array>
#include <chrono>
#include <mutex>
#include <sstream>
#include <string>

#define SDL_MAIN_HANDLED
#include "SDL.h"

#pragma comment(lib, "SDL2.lib")

int main(int argc, char* argv[])
{
  if (SDL_Init(SDL_INIT_AUDIO)) {
    std::cout << "SDL Init failed: " << SDL_GetError() << std::endl;
    exit(EXIT_FAILURE);
  }

  SDL_AudioSpec wanted_spec;
  SDL_AudioSpec obtained_spec;

  wanted_spec.freq = 48000;
  wanted_spec.format = AUDIO_S16LSB; // use native byte order if you have no diea.
  wanted_spec.channels = 2;
  wanted_spec.callback = nullptr;
  wanted_spec.userdata = nullptr;

  const char filename[] = "G:/av-learning/bin/win32/test.pcm";
  std::ifstream infile(filename, std::ios::binary);
  // std::stringstream sstr;
  // sstr << infile.rdbuf();
  // std::string data = sstr.str();

  SDL_AudioDeviceID deviceId = SDL_OpenAudioDevice(nullptr, 0, &wanted_spec, &obtained_spec, 1);
  if (deviceId < 2) {
    std::cerr << "Oen Audio Device Failed: " << SDL_GetError() << std::endl;
    exit(EXIT_FAILURE);
  }

  SDL_PauseAudioDevice(deviceId, 0);
  auto start = std::chrono::system_clock::now();

  std::array<char, 8192> buffer;
  uint64_t count = 0;
  // interval 
  uint32_t interval = (buffer.max_size() * 1000) / (wanted_spec.freq * wanted_spec.channels * 2);
  std::cout << "Interval : " << interval << "ms" << std::endl;
  while (infile.read(&buffer[0], buffer.max_size())) {
    std::size_t read_size = (std::size_t) infile.gcount();
    if (SDL_QueueAudio(deviceId, &buffer[0], read_size) != 0) {
      std::cerr << "Queue Audio Failed: " << SDL_GetError() << std::endl;
    }

    // count += read_size;
    // std::cout << "pos " << count << std::endl;

    // std::this_thread::sleep_for(19ms);
    SDL_Delay(interval); // 19ms delay for audio playing and goto next frame
  }

  // std::size_t last_unread_size = infile.gcount();
  // SDL_QueueAudio(deviceId, &buffer[0], last_unread_size);
  // SDL_Delay(interval);
  // std::cout << "last unread size: " << last_unread_size << std::endl;

  auto end = std::chrono::system_clock::now();
  std::cout
    << "duration: "
    << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms"
    << std::endl;

  SDL_CloseAudio();
  SDL_Quit();

  return 0;
}