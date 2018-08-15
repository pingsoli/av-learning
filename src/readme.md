## Project Description

#### First try
**helloworld** is first qt program. familiarize yourself with basic knowdage about qt.  
---
### Mix VS with Qt
using Visual Studio 2015 and qt creator compiling the whole project.  

1) install Qt plugin on VS.  
2) compile code on VS, and compile with Qt. Check the compatible.  
3) install Windows SDK, you can debug program with CDB.  
---
### Load FFmpeg in program
load ffmpeg dynamic library, and call the ffmpeg function.

1) configure ffmpeg library and header files path, write test example. (downlaod build libraries on windows platform)  
2) using qt creator compile agagin. (writing my own *.pro file and compiling on windows)  
3) transplanting all code from windows to linux platform.(installing ffmpeg on linux platform, and compiling the project)  
---
### Demux video files
```
av_register_all()
avformat_network_init()
avformat_open_input()
avformat_find_stream_info()
```

1) demuxing video file and print information.  
2) get all streams(audio and video streams).  
3) audio frame rate and video frame rate.  
4) AVPacket object allocation and deallocation, familiarize with some functions.  
```
av_packet_alloc() and av_packet_free()
av_packet_clone()
av_packet_ref() and av_packet_unref()
```
5) seek operation preview.  
av_seek_frame()  
---
### Decode video files

**two means to find decoder**  
AVCodec* avcodec_find_decoder(enum AVCodecID id)  
AVCodec* avcodec_find_decoder_by_name(const char* name);  

av_register_all() function register all decoders, so we can use these functions to find
the decoder.  
In android, we can specify specific decoder name in hardware decoding, like this:  
avcodec_find_decoder_by_name("h264_mediacodec")  

avcodec_alloc_context3(const AVCodec* codec) and avcodec_free_context(AVCodecContext** avctx);  
avcodec_open2(AVCodecContext* avctx, const AVCodec* codec, AVDictionary** options);  

1) decode packet to frame.
avcodec_send_packet  
avcodec_receive_frame  
NOTE: play the last frame, you must set the input packet is nullptr, and read until nothing.  
2) video pixel format scaling.  
3) audio resample.  