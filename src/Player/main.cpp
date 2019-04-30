#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swresample.lib") // audio resample

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
}

#include "MediaInfo.h"
#include "Decoder.h"
#include "Queue.h"

int main(int argc, char *argv[])
{
  MediaInfo mediaInfo;
  Queue<AVPacket*> videoPktQueue;
  Queue<AVPacket*> audioPktQueue;
  //Queue<AVFrame> videoFrameQueue;

  std::atomic_bool isPlaying = true;
  std::atomic_bool isSilent = false;

  mediaInfo.Open("src.mp4");
  Decoder decoder(mediaInfo);

  std::thread decodeThread(
    [&mediaInfo, &audioPktQueue, &isPlaying, &videoPktQueue] {

      AVPacket* tmpPkt;
      while (isPlaying) {
        AVPacket pkt;
        pkt.size = 0;

        int r = av_read_frame(mediaInfo.GetFormatContext(), &pkt);
        if (r < 0) break;

        tmpPkt = av_packet_clone(&pkt);
        if (pkt.stream_index == mediaInfo.VideoStreamIndex()) {
          videoPktQueue.Push(tmpPkt);
        } else if (pkt.stream_index == mediaInfo.AudioStreamIndex()) {
          audioPktQueue.Push(tmpPkt);
        } else {
          std::cerr << "Warning: decoding a packet from unknown stream" << std::endl;
        }

        av_packet_unref(&pkt);
      }
    }
  );

  std::thread audioThread(
    [&audioPktQueue, &isPlaying, &isSilent, &decoder] {
      AVFrame* frame = av_frame_alloc();
      int count  = 0;

      while (isPlaying && !isSilent) {
        AVPacket *pkt = audioPktQueue.Pop();
        int ret = avcodec_send_packet(decoder.GetAudioCodecContext(), pkt);
        av_packet_free(&pkt);

        if (ret != 0) {
          char error_msg_buf[256] = { 0 };
          av_strerror(ret, error_msg_buf, sizeof(error_msg_buf));
          std::cerr << "Error: send packet to audio decoder failed: " << error_msg_buf << std::endl;
          continue;
        }

        while ((ret = avcodec_receive_frame(decoder.GetAudioCodecContext(), frame)) == 0) {
          std::cout
            << ++count << " | " << frame->pts << std::endl;
        }
      }
    }
  );

  std::thread videoThread(
    [&isPlaying, &decoder, &videoPktQueue] {
      AVFrame* frame = av_frame_alloc();

      int count = 0;
      while (isPlaying) {
        AVPacket* pkt = videoPktQueue.Pop();
        int ret = avcodec_send_packet(decoder.GetVideoCodecContext(), pkt);
        av_packet_free(&pkt);
        if (ret < 0) break;

        while ((ret = avcodec_receive_frame(decoder.GetVideoCodecContext(), frame)) == 0) {
          std::cout
            << ++count << " | " << frame->pts << std::endl;
          //if (count == 100) exit(0);
        }
      }
    }
  );

  decodeThread.join();
  audioThread.join();
  videoThread.join();

  return 0;
}