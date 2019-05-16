## Project Description

---
### NOTE
`Xplayer` is outdated and not maintained now.

---
### Basic Process of Playing Media File
```
      demux             decode            render
File -------> AVPacket --------> AVFrame --------> Renderer(Screen and Speaker)

Demuxing Process
      Video Stream
    /
File
    \
      Audio Stream

swr_convert and sws_convert process:
Convert specified AVFrame supported.

such as:
swr_convert: fltp audio frame ----> s16 audio frame
sws_convert: RGB24 ----> YUV420P
```

---
### Combine Visual Studio IDE and Qt
Mix Visual Studio xxxx and qt creator to compile the whole project.  

1) Installing Qt plugin on Visual Studio xxxx.  
2) Compiling the project on Visual Studio xxxx, and compile on Qt.  
3) Installing Windows SDK for debugging program with CDB.  

---
### Qt Basics for OpenGL
1) OpenGL learning with Qt  
2) Shader basics  
3) GLSL(OpenGL Shading Language)  
