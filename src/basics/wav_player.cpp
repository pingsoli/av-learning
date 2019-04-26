// use SDL2 to play .wav file

// How to calculate the duration of wav file?
// time = file size / ( sample rate * channels * bits per sample / 8)

#include <SDL.h>
#include <stdio.h>

#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib") // necessary

long GetFilesize(char* filename)
{
  FILE *fp = fopen(filename, "rb");
  fseek(fp, 0, SEEK_END);
  return ftell(fp);
}

int main(int argc, char* argv[])
{
  char filename[] = "G:\\av-learning\\bin\\win32\\test.wav";
  SDL_AudioSpec wavSpec;
  SDL_AudioSpec obtained;

  uint32_t wavLength;
  uint8_t *wavBuffer;
  uint32_t duration;
  long filesize = GetFilesize(filename);

  // Get the information from ffmpeg and fill the spec
  wavSpec.freq = 48000;
  wavSpec.channels = 2;
  wavSpec.format = AUDIO_S16LSB;

  duration = filesize / (wavSpec.freq * wavSpec.channels * 2);

  SDL_Init(SDL_INIT_AUDIO);
  if (SDL_LoadWAV(filename, &wavSpec, &wavBuffer, &wavLength)) {
    SDL_AudioDeviceID deviceId = SDL_OpenAudioDevice(nullptr, 0, &wavSpec, &obtained, 1);
    if (deviceId) {
      int success = SDL_QueueAudio(deviceId, wavBuffer, wavLength);
      if (success < 0) {
        SDL_ShowSimpleMessageBox(0, "Error", "Failed to queue audio", nullptr);
      }
      SDL_PauseAudioDevice(deviceId, 0);
      SDL_Delay(duration * 1000); // Delay current thread to let SDL_Audio thread run
    } else {
      SDL_ShowSimpleMessageBox(0, "Error", "Open Audio Device Failed", nullptr);
    }
  } else {
    SDL_ShowSimpleMessageBox(0, "Error", "Load Wav failed", nullptr);
  }

  SDL_FreeWAV(wavBuffer);
  SDL_Quit();

  return 0;
}