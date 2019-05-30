// use SDL2 to play .wav file

// How to calculate the duration of wav file?
// time = (file size - 46) / ( sample rate * channels * bits per sample / 8)

#include <iostream>
#include <fstream>
#include <string>

// define SDL_MAIN_HANDLED or pragma comment(lib, "SDL2main.lib")
#define SDL_MAIN_HANDLED

#include "SDL.h"
#pragma comment(lib, "SDL2.lib")

uint64_t GetFileSize(const std::string& filename)
{
  std::ifstream infile(filename, std::ifstream::ate);
  return infile.tellg();
}

int main(int argc, char* argv[])
{
  char filename[] = "test.wav";
  SDL_AudioSpec wavSpec;

  uint32_t wavLength;
  uint8_t *wavBuffer;
  uint64_t duration;
  uint64_t filesize = GetFileSize(filename);

  SDL_Init(SDL_INIT_AUDIO);
  if (SDL_LoadWAV(filename, &wavSpec, &wavBuffer, &wavLength)) {
    // NOTE: duration is not precise because of wav header information size included.
    duration  = filesize / (wavSpec.freq * wavSpec.channels * (wavSpec.format & SDL_AUDIO_MASK_BITSIZE) / 8);
    SDL_AudioDeviceID deviceId = SDL_OpenAudioDevice(nullptr, 0, &wavSpec, nullptr, 1);
    if (deviceId) {
      int success = SDL_QueueAudio(deviceId, wavBuffer, wavLength);
      if (success < 0) {
        SDL_ShowSimpleMessageBox(0, "Error", "Failed to queue audio", nullptr);
      }
      SDL_PauseAudioDevice(deviceId, 0); // Play
      SDL_Delay((uint32_t) duration * 1000); // Delay current thread to let SDL_Audio thread run
      SDL_CloseAudioDevice(deviceId);
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