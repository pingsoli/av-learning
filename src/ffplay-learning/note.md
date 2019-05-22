
### ffplay.c source code learning
#### Callstack of Important Functions
```
=== main thread ===
main()
  |-> event_loop()                               - Handle an event sent by the GUI
        |-> refresh_loop_wait_event()            - Ready to render picture when invoking
              |-> av_usleep()                    - Calculate the remaining sleep time
              |-> video_refresh()                - Called to display each frame
                    |-> compute_target_delay()   - Based on last frame duration and get delay value
                    |-> update_video_pts()
                    |-> video_display()          - Render picture when needed, or ignore the rending

the calculation of remaining_time:
If the media file is 30 fps, 1000(ms) / 30(fps) = 33.333 ms, rendering a picture every 33.333ms.
and 33.333ms can be divided into 10ms, 10ms, 10ms, 3ms(ideal), so we sleep 10ms, then 10ms, 10ms,
at last, 3ms is left, it's remaining_time, so we should sleep 3ms.
that is the key of synchronization of video and audio.

max sleep time: 10 ms
AV_SYNC_THRESHOLD_MAX 0.1
AV_SYNC_THRESHOLD_MIN 0.04

what if a big delay? duplicate or deleting a frame.

=== video decode thread ===
video_thread()               - NOTE: exclude video filter part
  |-> get_video_frame() <------------------------+
  |-> calculate the frame duration and pts(ms)   |
  |-> queue_picture() -------------------------->+

=== audio decode thread ===
audio_thread()                - NOTE: exclude audio filter part
  |-> decoder_decode_frame() <-+
  |-> frame_queue_push() ------^

=== audio playing thread (controlled by SDL inner audio playing thread) ===
sdl_audio_callback()
  |-> audio_decode_frame()
  |-> copy frame data to SDL inner buffer for playing(not playing right now, until callback returns)
  |-> set audio clock and pts
```

#### Video Playing Part
```
=== compute_target_delay ===
diff = video clock - master clock
sync_threshold = max(AV_SYNC_THRESHOLD_MIN, min(AV_SYNC_THRESHOLD_MAX, last_frame_duration))

if (diff <= -sync_threshold)
    delay = FFMAX(0, delay + diff);
else if (diff >= sync_threshold && delay > AV_SYNC_FRAMEDUP_THRESHOLD)
    delay = delay + diff;
else if (diff >= sync_threshold)
    delay = 2 * delay;
``` 

#### Questions
1. Control Audio playing speed, 0.5, 1.0, 2.0 ...?  
`video_refersh()` function has following code:
```
return c->pts_drift + time - (time - c->last_updated) * (1.0 - c->speed);
```

2. audio clock and video clock, how to synchronize them ?  
ffplay use audio as master, video as slave deault(can specify by yourself). Slicing big sleep time into small pieces.

---
#### Testing File and Recoding
origin audio sample  
format: fltp(32 / 8 = 4 bytes)  
a audio frame size = 2 (ch) * (32/8) * 1024 = 8192  

after `swr_convert`  
format: s16  
a audio frame size = 2 (ch) * (16/8) * 1024 = 4096  

channels * (bit depth / 8) * sample rate = 2 * (16/8) * 48000 = 192000 (bytes per sec)  
a audio frame size = channels * (bit depth / 8) * sample num = 2 * (16/8) * 1024 = 4096  
192000 / 8192 = 46.875 fps  