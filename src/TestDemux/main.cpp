#include <iostream>
#include <thread>
#include <chrono>

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avcodec.lib")

int main(int argc, char* argv[])
{
    AVFormatContext *fctx = nullptr;
    const char filename[] = "cut.mp4";
    AVDictionary *opts = nullptr;  // parameter options, such as rtsp max_delay.
    int ret_code = -1;

    av_dict_set(&opts, "rtsp_transport", "tcp", 0); // using tcp protocol to open rtsp stream.
    av_dict_set(&opts, "max_delay",      "500", 0); // network max delay, may be useful in WAN.

    // register all muxters, demuxters and protocols.
    av_register_all();

    // initialize network library (rtsp, rtmp, http).
    avformat_network_init();

    ret_code = avformat_open_input(&fctx, filename, nullptr, &opts);
    if (ret_code != 0) {
        char buf[1024] = { 0 };
        av_strerror(ret_code, buf, sizeof(buf) - 1);
        std::cout << "open " << filename << " failed! error: " << buf << std::endl;
    } else {
        std::cout << "open files successfully!" << std::endl; 
    }
    
    // get stream info
    ret_code = avformat_find_stream_info(fctx, 0);

    // print duration, seconds
    auto total_seconds = fctx->duration / AV_TIME_BASE; // seconds
    auto total_millis  = fctx->duration / (AV_TIME_BASE / 1000); // milliseconds

    std::cout << filename << " duration: \n"
              << "  total seconds:      " << total_seconds << " s\n"
              << "  ttotal milliseonds: " << total_millis  << " ms" << std::endl;

    // print video stream detail info
    av_dump_format(fctx, 0, filename, 0);


    // print all streams information
    int audioStream = -1;
    int videoStream = -1;

    for (int i = 0; i < fctx->nb_streams; ++i) {
        AVStream *as = fctx->streams[i];
        
        // audio stream
        if (as->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStream = i;
            std::cout << "audio stream index: " << i << std::endl;
            std::cout << "  sample rate: " << as->codecpar->sample_rate << std::endl;
            std::cout << "  format: " << as->codecpar->format << std::endl;
            std::cout << "  channels: " << as->codecpar->channels << std::endl;
            std::cout << "  codec id: " << as->codecpar->codec_id << std::endl;

            // a frame in audio ? What is it ?
            // It's specific quantity samples on single channel.
            std::cout << "  frame size: " << as->codecpar->frame_size << std::endl;
            // 1024 * 2(double channels) * 2(16 bits) = 4096 bytes one audio frame.
            // audio frame rate = sample rate / frame size
            // example: 44100 / 1024 = 43(fps)
            std::cout << "  audio frame rate: " << as->codecpar->sample_rate / as->codecpar->frame_size << std::endl;
        }
        
        // video stream
        if (as->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            std::cout << "video stream index: " << i << std::endl;

            // NOTE: width and height may not be set. It's not accurate.
            std::cout << "  width: " << as->codecpar->width << std::endl;
            std::cout << "  height: " << as->codecpar->height << std::endl;

            // frame rate (fps), NOTE: denominator may be zero, you must check its value before divsing it.
            std::cout << "  (" << as->avg_frame_rate.num << "/" << as->avg_frame_rate.den << ")" << std::endl;
            std::cout << "frame rate: " << av_q2d(as->avg_frame_rate) << std::endl;
        }
        
        // subtitle stream
        if (as->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE) {
            std::cout << "subtitle stream index: " << std::endl;
        }

        std::cout << std::endl;
    } // all streams loop


    // the more concise to get the index of video and audio stream.
    videoStream = av_find_best_stream(fctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    audioStream = av_find_best_stream(fctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    std::cout << "audio index: " << audioStream << std::endl;
    std::cout << "video index: " << videoStream << std::endl;


    // allocate the memory and initialize it.
    AVPacket *pkt = av_packet_alloc();
    for ( ; ; ) {
        int ret = av_read_frame(fctx, pkt);

        if (ret != 0)  {
            std::cout << "====================== seek to 3 seconds ====================" << std::endl;
            getchar();

            // play the video cyclically.
            int ms = 3000; // goto 3 seconds place
            int64_t timestamp = (ms / 1000.0) / av_q2d(fctx->streams[videoStream]->time_base);
            std::cout << "seek position: " << timestamp << std::endl;
            std::cout << fctx->streams[videoStream]->time_base.num << " / " << fctx->streams[videoStream]->time_base.den << std::endl;
            av_seek_frame(fctx, videoStream, timestamp, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);

            continue;
        }

        std::cout << "packet size: " << pkt->size << std::endl;
        std::cout << "pts: " << pkt->pts << std::endl; // presentation timestamp
        std::cout << "dts: " << pkt->dts << std::endl; // decompression timestamp

        // get the time (milliseconds), audio and video asynchronization is more convenience.
        auto time = pkt->pts * av_q2d(fctx->streams[pkt->stream_index]->time_base); // the result is double value
        auto millis = time * 1000;
        std::cout << "time: " << time << "s, milliseconds: " << millis << std::endl;
         
        if (pkt->stream_index == videoStream) {
            std::cout << "picture" << std::endl;
        } else if (pkt->stream_index == audioStream) {
            std::cout << "audio" << std::endl;
        } else {
            std::cout << "unknown now" << std::endl;
        }

        std::cout << std::endl;
        // decrease the ref count, if zero, free it.
        av_packet_unref(pkt); // important, otherwise memory leak.

        // sleep 500ms for print information.
        //std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    av_packet_free(&pkt);

    // clean up
    if (fctx) {
        avformat_close_input(&fctx);
    }

    return 0;
}