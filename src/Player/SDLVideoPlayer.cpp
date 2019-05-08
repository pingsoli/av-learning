#include "SDLVideoPlayer.h"

#define SDL_MAIN_HANDLED
#include "SDL.h"
#include "SDL_Video.h"
#pragma comment(lib, "SDL2.lib")

extern "C" {
 #include "libavformat/avformat.h"
}

SDLVideoPlayer::SDLVideoPlayer()
{
}

SDLVideoPlayer::~SDLVideoPlayer()
{
  SDL_DestroyTexture(texture_);
  SDL_DestroyRenderer(renderer_);
  SDL_DestroyWindow(window_);
}

void SDLVideoPlayer::Init(const std::string& window_name, int width, int height)
{
  windowName_ = window_name;
  width_ = width;
  height_ = height;

  window_ = SDL_CreateWindow(windowName_.c_str(),
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    width_, height_,
    SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
  renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
  texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STATIC, width_, height_);
}

void SDLVideoPlayer::Render(AVFrame *frame)
{
  SDL_UpdateYUVTexture(texture_, nullptr,
                       frame->data[0], frame->linesize[0],
                       frame->data[1], frame->linesize[1],
                       frame->data[2], frame->linesize[2]);
  SDL_RenderClear(renderer_);
  SDL_RenderCopy(renderer_, texture_, nullptr, nullptr);
  SDL_RenderPresent(renderer_);
}