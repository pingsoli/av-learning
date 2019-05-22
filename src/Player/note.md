### Player

**NOTE:**
the synchronization of video and audio is copied from ffplay.c.

---
#### What's the meaning of 'sw' of swresample and swscale libraries ?
it means software. click [here](https://stackoverflow.com/questions/43066572/what-is-the-meaning-of-sw-in-libswscale-of-ffmpeg) for original answer.

---
#### How to contorl SDL audio callback's frequency ?
set smaples field of SDL_AudioSpec.

---
#### Some noise when playing audio at middle ?
at beginning, Audio AVPacket Queue will be full, then starts to decrease, at last, wait a long time
to get a audio AVPacket, because current thread is stucked at `videoPktQueue.Push(tmpPkt)`.

at last, I found the problem
```
if (pkt.stream_index == mediaInfo.VideoStreamIndex()) {
  videoPktQueue.Push(tmpPkt);
} else if (pkt.stream_index == mediaInfo.AudioStreamIndex()) {
  audioPktQueue.Push(tmpPkt);
}

    / -------> Video AVPacket Queue --------> Video AVFrame Queue --------> Screen
   /  
File   demux                         decode                        render
   \
    \ -------> Audio AVPacket Queue --------> Audio AVFrame Queue --------> Speaker
```

rendering process is too slow, and Video AVFrame Queue is full, and Video AVPacket Queue is full too, and could't be pushed
more data, and demux thread must wait until Video AVPacket Queue is not full(decoded by video decoding thread), and Audio AVPacket
Queue is consumed too fast(decoded -> playing through speaker) and has no enough data. so there is reason for noise at the middle of
playing audio.

**Conlusion:**  
videoPktQueue pushing speed will influence audioPktQueue pushing process.

---
#### Thread is dead at some place, not going to play video or audio ?
have no idea, deadlock ?