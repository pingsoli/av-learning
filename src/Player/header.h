#ifndef _HEADER_H_
#define _HEADER_H_

#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <fstream>
#include <ctime>

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
#include "libavutil/opt.h"
#include "libswresample/swresample.h"
#include "libavutil/imgutils.h"
#include "libavutil/error.h"
#include "libavutil/time.h"
#include "libswscale/swscale.h"
}

#include "MediaInfo.h"
#include "Decoder.h"
#include "Queue.h"
#include "SDLAudioPlayer.h"
#include "SDLVideoPlayer.h"
#include "Resampler.h"

#include "SDL.h"

#endif