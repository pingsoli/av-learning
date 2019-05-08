#include "SDLAudioPlayer.h"

#define SDL_MAIN_HANDLED
#include "SDL.h"
#include "SDL_Audio.h"
#pragma comment(lib, "SDL2.lib")

#include <cassert>
#include <iostream>
#include <unordered_map>
#include <string>

// all options are native byte order, don't have to spcify little endian or big endian
const std::unordered_map<std::string, uint16_t> sdl_sample_format_map = 
{
  { "u8", AUDIO_U8 },
  { "u16", AUDIO_U16SYS },
  { "s16", AUDIO_S16SYS },
  { "s32", AUDIO_S32SYS },
  { "f32", AUDIO_F32SYS }
};

SDLAudioPlayer::SDLAudioPlayer()
{
}

SDLAudioPlayer::~SDLAudioPlayer()
{
  SDL_CloseAudioDevice(deviceId_);
}

bool SDLAudioPlayer::Init(int sample_rate, const char *format, int channels)
{
  if (sdl_sample_format_map.find(format) == sdl_sample_format_map.end())
  {
    std::cout << "Error: wrong or unsupported sample format: " << format << std::endl;
    return false;
  }

  if (SDL_Init(SDL_INIT_AUDIO))
  {
    std::cout << "SDL Init failed: " << SDL_GetError() << std::endl;
    return false;
  }

  SDL_AudioSpec wanted_spec;
  SDL_AudioSpec obtained_spec;

  wanted_spec.freq = sample_rate;
  wanted_spec.format = sdl_sample_format_map.at(format);
  wanted_spec.channels = channels;
  wanted_spec.callback = nullptr;
  wanted_spec.userdata = nullptr;

  deviceId_ = SDL_OpenAudioDevice(nullptr, 0, &wanted_spec, &obtained_spec, 1);
  if (deviceId_ < 2) {
    std::cerr << "Error: Oen Audio Device Failed: " << SDL_GetError() << std::endl;
    return false;
  }

  sampleRate_ = sample_rate;
  channels_ = channels;
  format_ = sdl_sample_format_map.at(format);

  return true;
}

void SDLAudioPlayer::Play()
{
  SDL_PauseAudioDevice(deviceId_, 0);
}

void SDLAudioPlayer::Stop()
{
  SDL_PauseAudioDevice(deviceId_, 1);
}

void SDLAudioPlayer::Queue(uint8_t *data, int len)
{
  int sample_size_per_sec = (sampleRate_ * channels_ * (format_ & SDL_AUDIO_MASK_BITSIZE) / 8);
  int sleep_time = len * 1000 / sample_size_per_sec;
  int ret = SDL_QueueAudio(deviceId_, data, len);
  if (ret != 0) {
    std::cerr << "Error: SDL_QueueAudio failed: " << SDL_GetError() << std::endl;
    assert(ret == 0);
  }
  // SDL_Delay(sleep_time - 5);
  std::cout << "Queued size: " << SDL_GetQueuedAudioSize(deviceId_) << std::endl;
}