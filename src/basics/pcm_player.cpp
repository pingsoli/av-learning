// References
// https://github.com/leixiaohua1020/simplest_media_play/blob/master/simplest_audio_play_sdl2/simplest_audio_play_sdl2.cpp

#include <fstream>
#include <iostream>
#include <array>
#include <chrono>
#include <mutex>
#include <sstream>
#include <string>

#include "SDL.h"

#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")

int main(int argc, char* argv[])
{
  if (SDL_Init(SDL_INIT_AUDIO)) {
    std::cout << "SDL Init failed: " << SDL_GetError() << std::endl;
    exit(EXIT_FAILURE);
  }

  SDL_AudioSpec wanted_spc;
  SDL_AudioSpec obtained_spec;

  wanted_spc.freq = 48000;
  wanted_spc.format = AUDIO_S16SYS; // use native byte order if you have no diea.
  wanted_spc.channels = 2;
  wanted_spc.silence = 0;
  wanted_spc.samples = 4096;
  wanted_spc.padding = 0;
  wanted_spc.size = 16384;
  wanted_spc.callback = nullptr;
  wanted_spc.userdata = nullptr;

  const char filename[] = "G:\\av-learning\\bin\\win32\\test.wav";
  std::ifstream infile(filename, std::ios::binary);
  //std::stringstream sstr;
  //sstr << infile.rdbuf();
  //std::string data = sstr.str();

  if (SDL_OpenAudio(&wanted_spc, &obtained_spec)) {
    std::cerr << "Cannot open audio: " << SDL_GetError() << std::endl;
    exit(EXIT_FAILURE);
  }

  SDL_AudioDeviceID deviceId = SDL_OpenAudioDevice(nullptr, 0, &wanted_spc, nullptr, 1);
  if (deviceId < 2) {
    std::cerr << "Oen Audio Device Failed: " << SDL_GetError() << std::endl;
    exit(EXIT_FAILURE);
  }

  // Play the audio when SDL_Delay()
  SDL_PauseAudio(0);

  std::array<char, 8192> buffer;
  while (infile.read(&buffer[0], buffer.max_size())) {
    std::size_t read_size = infile.gcount();
    if (SDL_QueueAudio(deviceId, &buffer[0], read_size) != 0) {
      std::cerr << "Queue Audio Failed: " << SDL_GetError() << std::endl;
    }
    SDL_Delay(10);
  }

  SDL_CloseAudio();
  SDL_Quit();

  return 0;
}