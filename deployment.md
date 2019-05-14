## Qt Environment Setting Up

### Installing Windows SDK for Debuging
Windows SDK: https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk  

Function: you can debug program on qt creator(qt creator detects Visual Studio debugger automatically, or specify by yourself)  

### Installing Visual Stuio IDE plugin (for creating qt project on Visual Studio IDE)
VS Plugin Downloading link: https://download.qt.io/official_releases/vsaddin/  

Double click to install plugin directly, and "Qt VS tools" option can be seen on VS menu bar when install successfully.   

### Qt Resources Downloading
link: https://download.qt.io/  

the directorie contains QT development, Qt creator, VS addins and etc.  
Qt creator contains Qt designer and other stuff, using them to develop faster.

### Visual Studio Tips
1. Enabling console output when developing UI program.  
VS specific configuration：  
Configuration Property -> Linker -> System -> Subsystem, choose '/SUBSYSTEM:CONSOLE'.  


### libyuv installation
link: https://chromium.googlesource.com/libyuv/libyuv/+/refs/heads/master

#### Build under MSVC
```
cmake -DCMAKE_BUILD_TYPE="Release" ..
cmake --build . --config Release
```