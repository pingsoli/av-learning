#ifndef _SDL_AUDIO_PLAYER_
#define _SDL_AUDIO_PLAYER_

#include <cstdint>

class SDLAudioPlayer
{
public:
  SDLAudioPlayer();
  ~SDLAudioPlayer();
  bool Init(int sample_rate, const char* format, int channels);
  void Play();
  void Stop();
  void Queue(uint8_t* data, int len);

private:
  int channels_;
  int sampleRate_;
  uint16_t format_;
  int deviceId_;
};

#endif