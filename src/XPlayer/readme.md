### **Project introduction**

**Total threads**
1) main thread - deal with all kinds of events.
2) XDemuxThread - initialize, run XVideoThread and XAudioThread, then do demuxing.
3) XVideoThread - deocde video frame and render it.
4) XAudioThread - decode audio frame, resample, and then play it.

**Threads relationship**
```
threads call stack:
open file -> run XDemuxThread -> run XAudioThread and XVideoThread

              |-> XVideothread
XDemuxThrad ->|
              |-> XAudioThread
```
**Top level for playing video and audio**  
QIODevice and QAudioFormat are for playing sound.  
QOpenGLWidget is for rendering picture.  

---

### **Questions**
**Synchronize video and audio stream ?**  

**How to keep fixed audio fps when playing local video file ?**  
for example: 48 frame per second (48 fps).

**Wrong progress bar display of QSlider ?**  
the basic policy: get duration and get current pts.
calculate the ratio, and multiply the QSlider max value.

```
pos = (pts / duration) * max
```

***Q1***: use audio pts or video pts ? which one is better ?  
***A1***: It depends on synchronization policy. if choose synchronize audio with
video pts, you should use video pts.

***Q2***: the last pts may not be equal to duration, your QSlider will not arrive end?   
***A2***: 

**How to get the last video frame's pts ?**  


**Closing Qt window causing error when playing rtsp stream, but works on regular video file ?**  
Have no idea.

**How to design buffer to make playing fluent ?**  
the buffer for audio and video decoded AVPacket.

**Qt UI render has a big lag ?**  
call QWidget::show() just draw the main window briefly, and then enter main event loop.

```
int main(int argc, char **argv[]) {
    QApplication a(argc, argv);
    XPlayer w;
    w.show(); // just render window briefly, exclude UI subcomponents.
    // do some blocking jobs, may spend too much time.
    // ...
    return a.exec();
}
```

**Conclusion**: Don't put blocking task into main event thread.

**FFmpeg avcodec_receive_from() return EAGAIN which means `Resource temporarily unavailable` ?**  
why would this happened ? how to avoid this ?  
Have no idea.

### Errors
list all errors here.

> Exception thrown at 0x00007FF8611B695E (Qt5Guid.dll) in XPlayer.exe: 0xC0000005: Access violation reading location 0x0000000000000008.  
>
> A LIST_ENTRY has been corrupted (i.e. double remove)  
>
> XPlayer.exe has triggered a breakpoint.  
> Unhandled exception at 0x00007FF89A611CD0 (ntdll.dll) in XPlayer.exe: 0xC0000374: ¶ÑÒÑËð»µ¡£ (parameters: 0x00007FF89A64ED40).
>
> Exception thrown at 0x00007FF89A55108A (ntdll.dll) in XPlayer.exe: 0xC0000005: Access violation reading location 0x0000000000000000.

---

### **Notes**
**A audio packet may has many frames, but a video packet has only one frame**  

why frame format is not equal to final format ?

**`std::lock_guard` cannot lock the `std::mutex` twice, it causes undefined behavior**  

example code:
```
void close() {
    std::lock_guard<std::mutex> locker(mux);
    // do some cleaning up.
}

void open() {
    std::lock_guard<std::mutex> locker(mux);
    close(); // error, lock the same mutex twice.
}
```
right solution:
```
void open() {
    close(); // ok
    std::lock_guard<std::mutex> locker(mux);
}
```

**Concusion**: Don't lock the same `std::mutex` twice in a function.

**Avoid CPU high loading**  
make thread sleep specific time.
```
while (!isExit) {
    if (isPause) {
        std::this_thread::sleep_for(50ms); // make it slower
        continue;
    }

    // do other thing
    // ...
}
```

---
### **Tips**
**Disable specific warnings in visual studio 2015**  
Configuration Properties -> C/C++ -> Advanced -> Disable Specific Warnings.
enter the integer number without prefix "C", example: "4819", stands for
disable "C4819" warning.

you can do it too with the following code.
```
#if (_WIN32 || _WIN64)
// prompt saving file as unicode format, only in windows platform.
#pragma warning(disable : 4819)
#endif
```

***Needs to know***  
Configuration on visual studio can supress the warning completely, but if you use code,
you must put the above code everywhere that warning occuring.

**Visual Studio 2015 Debugging shortcuts**  
```
f5                 start debug or continue to execute  
shift + f5         quit debugging
ctrl + shift + f5  restart debugging.
f10                step over  
f11                step into  
shift + f11        step out  
```

show callstack  
mutltithread debugging, dead lock  
check memory leak  

---
### **Pendings**
1. reopen video file multiple times. (done)  
2. only play audio or video. (undo)
3. synchronize audio and video stream. (undo) 
4. progress bar do not arrive end at last. (undo)