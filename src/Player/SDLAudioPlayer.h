#ifndef _SDL_AUDIO_PLAYER_
#define _SDL_AUDIO_PLAYER_

#include <cstdint>
#define SDL_MAIN_HANDLED
#include "SDL.h"

class SDLAudioPlayer
{
public:
  SDLAudioPlayer();
  ~SDLAudioPlayer();
  bool Init(int sample_rate, const char* format, int channels, void callback(void *, uint8_t*, int), void *opaque);
  void Play();
  void Stop();

private:
  int channels_;
  int sampleRate_;
  uint16_t format_;
  int deviceId_;
  SDL_AudioSpec spec_;
};

#endif