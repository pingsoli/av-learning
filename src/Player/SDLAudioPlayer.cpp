#include "SDLAudioPlayer.h"
#include "SDL_Audio.h"

#include <cassert>
#include <iostream>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <cmath>

// all options are native byte order, don't have to spcify little endian or big endian
const std::unordered_map<std::string, uint16_t> sdl_sample_format_map = 
{
  { "u8",  AUDIO_U8 },
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

bool SDLAudioPlayer::Init(int sample_rate, const char *format, int channels,
  void callback(void *, uint8_t*, int), void *opaque)
{
  if (sdl_sample_format_map.find(format) == sdl_sample_format_map.end()) {
    std::cout << "Error: wrong or unsupported sample format: " << format << std::endl;
    return false;
  }

  if (SDL_Init(SDL_INIT_AUDIO)) {
    std::cout << "SDL Init failed: " << SDL_GetError() << std::endl;
    return false;
  }

  SDL_AudioSpec wanted_spec;
  SDL_AudioSpec obtained_spec;

  wanted_spec.freq = sample_rate;
  wanted_spec.format = sdl_sample_format_map.at(format);
  wanted_spec.silence = 0;
  wanted_spec.samples = std::max(sdl_audio_min_buffer_size, 2 << (int) std::log2(wanted_spec.freq / sdl_audio_max_callbacks_per_sec));
  wanted_spec.channels = channels;
  wanted_spec.callback = callback;
  wanted_spec.userdata = opaque;

  deviceId_ = SDL_OpenAudioDevice(nullptr, 0, &wanted_spec, &obtained_spec,
                                  SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE);
  if (deviceId_ < 2) {
    std::cerr << "Error: Oen Audio Device Failed: " << SDL_GetError() << std::endl;
    return false;
  }

  sampleRate_ = sample_rate;
  channels_ = channels;
  format_ = sdl_sample_format_map.at(format);
  spec_ = obtained_spec;

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