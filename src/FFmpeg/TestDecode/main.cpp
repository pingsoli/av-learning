#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/time.h"
}

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "swresample.lib")

int main(int argc, char* argv[])
{
    //std::ofstream out("out.txt");
    //std::streambuf *old_cout_buf = std::cout.rdbuf(out.rdbuf()); // save and redirect.

    AVFormatContext *fctx = nullptr;
    const char filename[] = "cut.mp4";
    AVDictionary *opts = nullptr;  // parameter options, such as rtsp max_delay.
    int ret_code = -1;

    av_dict_set(&opts, "rtsp_transport", "tcp", 0); // using tcp protocol to open rtsp stream.
    av_dict_set(&opts, "max_delay",      "500", 0); // network max delay, may be useful in WAN.

    ////////////////////////////////////////////////////////////////////////////////
    // register all muxters, demuxters and protocols.
    av_register_all();

    // initialize network library (rtsp, rtmp, http).
    avformat_network_init();

    // register decoder.
    avcodec_register_all();
    ///////////////////////////////////////////////////////////////////////////////

    ret_code = avformat_open_input(&fctx, filename, nullptr, &opts);
    if (ret_code != 0) {
        char buf[1024] = { 0 };
        av_strerror(ret_code, buf, sizeof(buf) - 1);
        std::cout << "open " << filename << " failed! error: " << buf << std::endl;
    } else {
        std::cout << "open files successfully!" << std::endl; 
    }
    
    // get stream information.
    ret_code = avformat_find_stream_info(fctx, nullptr);
    if (ret_code != 0) {
        char buf[1024] = { 0 };
        std::cout << "avformat_find_stream_info failed, message: "
                  << av_strerror(ret_code, buf, sizeof(buf) - 1)
                  << std::endl;
    }

    // test av_gettime() function, no use.
    //std::cout << "get time: " << av_gettime() << std::endl;
    //std::cin.get();

    // print duration, seconds and milliseconds precision.
    auto total_seconds = fctx->duration / AV_TIME_BASE; // second precision
    auto total_millis  = fctx->duration * 1000 / AV_TIME_BASE; // millisecond precision

    std::cout << filename << " duration: \n"
              << "  total seconds:     " << total_seconds << " s\n"
              << "  total milliseonds: " << total_millis  << " ms" << std::endl;

    // print video detail information.
    //av_dump_format(fctx, 0, filename, 0);

    // print all streams information.
    int audioStream = -1;
    int videoStream = -1;

    // find video and audio streams according to media type.
    for (size_t i = 0; i < fctx->nb_streams; ++i) {
        AVStream *as = fctx->streams[i];
        
        // audio stream
        if (as->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStream = i;
            std::cout << "audio stream index: " << i << std::endl;
            std::cout << "  sample rate: " << as->codecpar->sample_rate << std::endl;
            std::cout << "  format: " << as->codecpar->format << std::endl;
            std::cout << "  channels: " << as->codecpar->channels << std::endl;
            std::cout << "  codec id: " << as->codecpar->codec_id << std::endl;
            std::cout << "  time_base: " << as->time_base.num << " / " << as->time_base.den << std::endl;

            // copy from AVRational
            //AVRational tmp = as->time_base;
            //std::cout << "(" << tmp.num << " / " << tmp.den << ")" << std::endl;
            //std::cin.get();

            std::cout << "  duration: " << as->duration << std::endl;
            std::cout << "  duration(s): " << as->duration / as->time_base.den << std::endl;
            std::cout << "  duration(ms): " << as->duration * 1000.0 / as->time_base.den << std::endl;

            // a frame in audio ? What is it ?
            // It's specific quantity samples on a single channel.
            std::cout << "  frame size: " << as->codecpar->frame_size << std::endl;
            // 1024 * 2(double channels) * 2(16 bits) = 4096 bytes per audio frame.
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
            std::cout << "  (" << as->avg_frame_rate.num 
                      << " / " << as->avg_frame_rate.den << ")" << std::endl;
            std::cout << "  video frame rate: " << av_q2d(as->avg_frame_rate) << std::endl;
            std::cout << "  time_base: " << as->time_base.num << " / " << as->time_base.den << std::endl;
            std::cout << "  duration: " << as->duration << std::endl;
            std::cout << "  duration(s): " << as->duration / as->time_base.den << std::endl;
            std::cout << "  duration(ms): " << as->duration * 1000.0 / as->time_base.den << std::endl;
        }
        
        // subtitle stream
        if (as->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE) {
            std::cout << "subtitle stream index: " << std::endl;
        }

        std::cout << std::endl;
    } // loops all streams
    //std::cin.get();

    // get duration from video and audio stream.
    // video and audio have different duration at most time,
    // for example, video stream recorded 24 frames per second.
    // audio data sampled 44100 data per seconds.
    // supposing a media file has 10 seconds. 
    // NOTE: duration's type is int64_t.
    // video frame rate: { 24000, 1001 }.
    // audio sample rate: { 44100, 1 }.
    // video duration: 24000 * 10 (seconds) = 240000.
    // audio duration: 44100 * 10 (seconds) = 441000.
    std::cout << "video duration: " << fctx->streams[videoStream]->duration << '\n'
        << "audio duration: " << fctx->streams[audioStream]->duration << std::endl;
    //std::cin.get();


    // more consice approach to get the video and audio stream index.
    videoStream = av_find_best_stream(fctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    audioStream = av_find_best_stream(fctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    std::cout << "audio index: " << audioStream << std::endl;
    std::cout << "video index: " << videoStream << std::endl;

    ///////////////////////////////////////////////////////////////////////////////
    // find video decoder
    AVCodec *vcodec = avcodec_find_decoder(fctx->streams[videoStream]->codecpar->codec_id);
    if (!vcodec) {
        std::cout << "cannot find the decoder ID: "
                  << fctx->streams[videoStream]->codecpar->codec_id << std::endl;
        return -1;
    }

    AVCodecID vCodecID = fctx->streams[videoStream]->codecpar->codec_id;
    std::cout << "video decoder id: " << vCodecID << ", "
              << "name: " << avcodec_get_name(vCodecID) << std::endl;
    //std::cin.get();

    // create decode context.
    AVCodecContext *vcc = avcodec_alloc_context3(vcodec);

    // configure decoder context from decoded codec parameter.
    avcodec_parameters_to_context(vcc, fctx->streams[videoStream]->codecpar);

    // set thread number for decoding.
    vcc->thread_count = 2; // old laptop, 2 cores 4 threads.

    // open codec context.
    ret_code = avcodec_open2(vcc, 0, 0);
    if (ret_code != 0) {
        char buf[1024] = { 0 };
        std::cout << "open codec failed, message: "
                  << av_strerror(ret_code, buf, sizeof(buf) - 1) << std::endl;
        return -1;
    }


    ///////////////////////////////////////////////////////////////////////////////
    // find audio decoder.
    AVCodec *acodec = avcodec_find_decoder(fctx->streams[audioStream]->codecpar->codec_id);
    if (!acodec) {
        std::cout << "cannot find the audio decoder id: "
                  << fctx->streams[audioStream]->codecpar->codec_id << std::endl;
        return -1;
    }

    AVCodecID aCodecID = fctx->streams[audioStream]->codecpar->codec_id;
    std::cout << "audio decoder id: " << aCodecID << ", "
        << "name: " << avcodec_get_name(aCodecID) << std::endl;
    //std::cin.get();

    // create decode context.
    AVCodecContext *acc = avcodec_alloc_context3(acodec);

    // configure audio decoder context parameter.
    avcodec_parameters_to_context(acc, fctx->streams[audioStream]->codecpar);

    acc->thread_count = 2;

    ret_code = avcodec_open2(acc, 0, 0);
    if (ret_code != 0) {
        char buf[1024];
        std::cout << "open audio codec failed, message: "
                  << av_strerror(ret_code, buf, sizeof(buf) - 1) << std::endl;
        return -1;
    }


    // allocate the memory and initialize it.
    AVPacket *pkt = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();


    // pixel format changing and scale.
    SwsContext *vctx = nullptr;
    uint8_t *rgb = nullptr;


    // audio resample
    SwrContext *actx = swr_alloc();
    actx = swr_alloc_set_opts(actx,
            av_get_default_channel_layout(2), // 2 channels output for convenience. specify as you need.
            AV_SAMPLE_FMT_S16, // output sample format (16 bits)
            acc->sample_rate,  // sample rate
            av_get_default_channel_layout(2), // 2 channels input
            acc->sample_fmt,
            acc->sample_rate,
            0, nullptr
        );

    ret_code = swr_init(actx);  // initialize first, otherwise won't compile.
    if (ret_code != 0) {
        char buf[1024];
        std::cout << "swr_init failed, message: " << av_strerror(ret_code, buf, sizeof(buf) - 1) << std::endl;
        return -1;
    }

    // point to pcm raw data.
    uint8_t *pcm = nullptr;


    int64_t total_frames = 0;

    // now we are going to read frame and do some operating.
    for ( ; ; ) {
        int ret = av_read_frame(fctx, pkt); // return next frame of a stream(video, audio ....)
        // For video, the packet contains exactly one frame.
        // For audio, it contains an integer number of frams if each frame has a known fixed size.

        // read until end.
        if (ret != 0)  {
            std::cout << "====================== seek to 3 seconds ====================" << std::endl;
            std::cin.get();

            // play the video from 3 seconds to end cyclically.
            int ms = 3000; // goto 3 seconds place
            double timestamp = (ms / 1000.0) * av_q2d(fctx->streams[videoStream]->time_base);
            std::cout << "seek position: " << timestamp << std::endl;
            std::cout << fctx->streams[videoStream]->time_base.num << " / " << fctx->streams[videoStream]->time_base.den << std::endl;
            av_seek_frame(fctx, videoStream, static_cast<int>(timestamp), AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);

            continue;
        }

        std::cout << "packet size: " << pkt->size << std::endl;
        std::cout << "pts: " << pkt->pts << ", " // presentation timestamp
                  << "dts: " << pkt->dts << std::endl; // decompression timestamp

        // get the time (milliseconds), audio and video asynchronization is more convenience.
        auto time = pkt->pts * av_q2d(fctx->streams[pkt->stream_index]->time_base); // the result is double value
        auto millis = time * 1000;
        std::cout << "time: " << time << "s, milliseconds: " << millis << "ms" << std::endl;

        AVCodecContext *cc = nullptr;
        if (pkt->stream_index == videoStream) {
            std::cout << "dealing with picture of video stream" << std::endl;
            cc = vcc;
        } else if (pkt->stream_index == audioStream) {
            std::cout << "dealing with audio stream" << std::endl;
            cc = acc;
        } else {
            std::cout << "unknown now" << std::endl;
            cc = nullptr;
        }

        // send packet to decoder thread (thread num depends on thread_count)
        // just send a packet to decoding queue. (no CPU calculation)
        // play the last cache frame, send nullptr packet, and call receive multiple times.
        ret_code = avcodec_send_packet(cc, pkt);

        // decrease the ref count, if zero, free it. once sent, decrease the ref counts.
        av_packet_unref(pkt); // important, otherwise causes memory leak.

        if (ret_code != 0) {
            char buf[1024] = { 0 };
            av_strerror(ret_code, buf, sizeof(buf) - 1);
            std::cout << "avcodec_send_packet failed! message: " << buf << std::endl;
            continue;
        }

        // send operation one time, maybe receive multiple times. (No high CPU operations)
        // the frame can contains audio and audio raw data.
        for ( ; ; ) {
            ret_code = avcodec_receive_frame(cc, frame);
            if (ret_code != 0) break;  // receive operations end.

            // video and audio have different format and linesize.
            std::cout << "recv frame: format = " << frame->format 
                      << ", linesize = " << frame->linesize[0]
                      << ", packet size: " << frame->pkt_size << std::endl;

            if (cc == vcc) {
                auto current_pos = frame->pts * av_q2d(fctx->streams[videoStream]->time_base);
                auto total_duration = fctx->duration / AV_TIME_BASE;

                std::cout << "video position: " << current_pos << std::endl; // current position(seconds)
                std::cout << "video ratio: " << current_pos / total_duration * 1000 << std::endl;

                // video pixel format scaling operations.
                // NOTE: these operations cost high CPU calculation. doing the task on GPU is better.
                vctx = sws_getCachedContext(
                    vctx,                          // if nullptr, create a new memory space.
                    frame->width, frame->height,   // width, height of input video.
                    (AVPixelFormat) frame->format, // YUV420P.
                    frame->width, frame->height,   // width, height of output video.
                    AV_PIX_FMT_RGBA,               // input format RGBA.
                    SWS_BILINEAR,                  // scale algorithm.
                    nullptr, nullptr, nullptr      // filters and parameters for picture algorithm.
                    );

                std::cout << "picture formating scale " << (vctx ? "successfully!" : "failed!") << std::endl;

                // deal with picture frame
                if (vctx) {
                    if (!rgb) {
                        rgb = new uint8_t[frame->width * frame->height * 4];
                    }

                    uint8_t *data[2] = { 0 };
                    data[0] = rgb;
                    int lines[2] = { 0 };
                    lines[0] = frame->width * 4; // RGB every unit is 4 bits.

                    ret_code = sws_scale(vctx,
                        frame->data,     // input data
                        frame->linesize, // input linesize
                        0,               // slice place
                        frame->height,   // input height
                        data,            // rgb planer only has single slot
                        lines
                    );

                    // video resolution: 1024 * 576, so sws_scale is 576.
                    std::cout << "sws_scale = " << ret_code << std::endl;
                }

            } else if (cc == acc) {
                std::cout << "audio position: " << frame->pts * av_q2d(fctx->streams[audioStream]->time_base) << std::endl;

                // audio resample procedure.
                if (actx) {
                    uint8_t *data[2] = { 0 };
                    if (!pcm) {
                        pcm = new uint8_t[frame->nb_samples * 4 /* 2(channels) * 2(sample_format 16bits) */];
                    }
                    data[0] = pcm;

                    ret_code = swr_convert(actx,
                        data, frame->nb_samples,       // output data
                        const_cast<const uint8_t**>(frame->data), frame->nb_samples // input data
                    );

                    std::cout << "swr_convert = " << ret_code << std::endl;
                }
            } else {
                std::cout << "other stream" << std::endl;
            }
        }

        std::cout << std::endl;

        // sleep 500ms for printing debug information.
        //std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    av_packet_free(&pkt);
    av_frame_free(&frame);
    swr_free(&actx);

    // recover the cout, not redirect to file stream.
    //std::cout.rdbuf(old_cout_buf);

    // clean up
    if (fctx) {
        avformat_close_input(&fctx);
    }

    return 0;
}