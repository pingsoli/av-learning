## Qt Environment Setting Up

### Installing Windows SDK for Debuging
Windows SDK: https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk  

Function: you can debug program on qt creator(qt creator detects Visual Studio debugger automatically, or specify by yourself)  

### Installing Qt plugin on Visual Studio IDE
VS Plugin Downloading link: https://download.qt.io/official_releases/vsaddin/  

Double click to install plugin directly, and "Qt VS tools" option can be seen on VS menu bar when install successfully.   

### Qt Resources Downloading
link: https://download.qt.io/  

the directorie contains QT development, Qt creator, VS addins and etc.  
Qt creator contains Qt designer and other stuff, using them to develop faster.

---
## libyuv library compilation
link: https://chromium.googlesource.com/libyuv/libyuv/+/refs/heads/master

**Build under MSVC**  
```
cmake -DCMAKE_BUILD_TYPE="Release" ..
cmake --build . --config Release
```

---
## SoundTouch library compilation
link: https://gitlab.com/soundtouch/soundtouch

```
git clone https://gitlab.com/soundtouch/soundtouch
cd soundtouch
make-win.bat
```

**16 bit integer and 32 bit float sample format on windows, which is your need ?**  
SoundTouch default is float sample format, if you want 16 bit integer smaple format.
uncomment `#define SOUNDTOUCH_INTEGER_SAMPLES     1` and comment `#define SOUNDTOUCH_FLOAT_SAMPLES       1` in `STTypes.h`.

NOTE: make sure msvc **toolset v140** installed on Visual Studio 2017. another way,
upgrade the toolset you have installed on your PC, you can uncomment the code in `make-win.bat`.
`devenv source\SoundTouchDll\SoundTouchDll.sln /upgrade`.

the differences of `MD`, `MDd`, `MT` and `MTd` ?
