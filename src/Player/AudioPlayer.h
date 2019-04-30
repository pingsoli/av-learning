#ifndef _AUDIO_PLAYER_H_
#define _AUDIO_PLAYER_H_

struct AVFrame;

class AudioPlayer
{
public:
  void Init();
  void Queue(AVFrame* frame);

private:
};

#endif