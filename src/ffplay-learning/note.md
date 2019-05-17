
### ffplay.c source code learning
#### Callstack of Important Functions
```
=== main thread ===
main()
  |-> event_loop()                               - Handle an event sent by the GUI
        |-> refresh_loop_wait_event()
              |-> av_usleep()                    - Control video playing speed
              |-> video_refresh()                - Called to display each frame
                    |-> compute_target_delay()
                    |-> update_video_pts()

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

#### Audio Playing Part
audio_callback_time

max sleep time: 10 ms

AV_SYNC_THRESHOLD_MAX 0.1
AV_SYNC_THRESHOLD_MIN 0.04

compute_target_delay based on last frame duration

big delay? duplicate or deleting a frame

```
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
2. audio clock and video clock, how to synchronize them ?  
3. Video playing speed ?  
`video_refersh()` function has following code:
```
return c->pts_drift + time - (time - c->last_updated) * (1.0 - c->speed);
```

sdl_audio_callback -> audio_clock ?

audio pts in sdl_audio_callback ?  
from audio_decode_frame function.


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

nb_samples / sample_rate = 1024 / 48000 = 0.0213333

audio clock and pts ?? what the relationship between them ?  
audio clock stands for when playing audio.  
audio pts stands   
in normal mode(without pause, speed up or slow down audio)  
the difference between audio clock and audio pts is equal.  

audio clock->pts_drift
audio clock->last_updated


video_refresh() 

frame_timer -> What the function ?
how to check video frame rate in refresh_loop_wait_event() ?


```
=== video_refresh() ====
/* display picture */
if (!display_disable && is->force_refresh && is->show_mode == SHOW_MODE_VIDEO && is->pictq.rindex_shown)
  video_display(is);
```

video test file: 30 fps
`forch_refresh` and `rindex_shown` control the picture rendering speed.