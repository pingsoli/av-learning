#ifndef _SDL_VIDEO_PLAYER_H_
#define _SDL_VIDEO_PLAYER_H_

#include <string>

struct SDL_Window;
struct SDL_Texture;
struct SDL_Renderer;
struct AVFrame;

class SDLVideoPlayer {
public:
  SDLVideoPlayer();
  ~SDLVideoPlayer();

  void Init(const std::string& window_name, int width, int height);
  void Render(AVFrame* frame);

private:
  SDL_Window* window_;
  SDL_Texture* texture_;
  SDL_Renderer* renderer_;
  std::string windowName_;
  int height_;
  int width_;
};

#endif