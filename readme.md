### Project Introduction

```
the project directory hierarchy
|-- bin            output executable files and dll files(dynamic libraries) location.
|   |-- win32
|   |-- win64
|-- include
|   |-- ffmpeg       Encode, decode, filter library for audio and video
|   |-- sdl2         Audio playing and picture rendering library
|   |-- glfw         GLFW and GLAD for OpenGL library
|   |-- glm          OpenGL Mathematics library(header-only)
|   |-- stab         Graphics library(header-only)
|   |-- libyuv       YUV scaling and conversion
|   |-- soundtouch   Audio library for audio pitch, tempo and rete.
|-- lib
|   |-- ffmpeg
|   |   |-- win32
|   |   |-- win64
|   |-- sdl2
|   |   |-- ...
|   |-- ...
|-- src            all codes(projects) are here.
|   |-- basics              common library basic usages and test codes
|   |-- ffplay-learning     ffplay source code learning project
|   |-- Player              Meida file player with av sync
|   |-- ...
```

NOTE: Only commit the `src` directory, others are ignored.  
On linux platform, don't have to move all dynamic libraries to current project directory,
just specify the path on configuration file(such as *.pro file).  
On windows platform, for convenience, copy all .dlls to `bin/win32` or `bin/win64` directory, and you can
test and debug the program directly.

---
#### Project Abstract
**FFmpeg**  
get media file information  
important video and audio stream parameters  
decode frame(AVPacket and AVFrame)  
swresample and swscale satisfy our hardware requirements  
AVFilters  

**SDL2**  
playing audio and rendering picture  

**libyuv**  
pixel format conversion, such as YUV to RGB, RGB to YUV or YUV411 to YUV420  

**SoundTouch**  
variable speed playback for audio(0.5x, 1.0x, 1.5x, 2.0x and etc.)  

**Qt5**  
GUI design and multi-meida components  