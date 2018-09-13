#include <iostream>

#include "XDecoder.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}

XDecoder::XDecoder()
{
}

XDecoder::~XDecoder()
{
}

bool XDecoder::open(AVCodecParameters* params)
{
    if (!params) return false;

    AVCodec *c = avcodec_find_decoder(params->codec_id);
    if (!c) {
        std::cout << "cannot find the decoder" << std::endl;
        return false;
    }

    {
        std::lock_guard<std::mutex> locker{mux};
        codec = avcodec_alloc_context3(c);
        // copy context from parameters.
        avcodec_parameters_to_context(codec, params);
        codec->thread_count = 2; // default thread num for decoding.

        int ret = avcodec_open2(codec, nullptr, nullptr);
        if (ret != 0) {
            char buf[256] = { 0 };
            av_strerror(ret, buf, sizeof(buf) - 1);
            std::cout << "avcodec_open2 failed, error: " << buf << std::endl;
            avcodec_free_context(&codec);
            return false;
        }
    }

    //std::cout << "avcodec_open2 success!" << std::endl;
    return true;
}

bool XDecoder::send(AVPacket* pkt)
{
    int ret = -1;
    {
        std::lock_guard<std::mutex> locker{mux};
        if (!codec) return false;
        ret = avcodec_send_packet(codec, pkt);
        if (ret != 0) {
            char buf[256] = { 0 };
            av_strerror(ret, buf, sizeof(buf) - 1);
            std::cout << "avcodec_send_packet() failed, ret code: "
                << ret << ", error: " << buf << std::endl;
        }
    }

    if (pkt) {
        av_packet_unref(pkt);
        av_packet_free(&pkt);
    }

    return (ret == 0 ? true : false);
}

AVFrame* XDecoder::recv()
{
    // IMPROVEMENT:  must allocate new memory space every receipt.
    // waste time, need to be polished.
    AVFrame *frame = av_frame_alloc();

    int ret = -1;
    {
        std::lock_guard<std::mutex> locker{mux};

        if (!codec) {
            av_frame_free(&frame);
            return nullptr;
        }

        // Question: avcodec_receive_frame() return EAGAIN at most time ?
        // how to avoid this ?
        ret = avcodec_receive_frame(codec, frame); 
        if (ret != 0) {
            
            //char buf[256] = { 0 };
            //av_strerror(ret, buf, sizeof(buf) - 1);
            //std::cout << "avcodec_receive_from() failed: " << buf << std::endl;

            av_frame_free(&frame);
            return nullptr;
        }
        pts = frame->pts; // record the pts for progress bar.
    }

    return frame;
}

int64_t XDecoder::getPts() {
    std::lock_guard<std::mutex> locker{mux};
    return pts;
}
//
//void XDecoder::setPts(int64_t pts_)
//{
//    std::lock_guard<std::mutex> locker{mux};
//    pts = pts_;
//}

void XDecoder::close()
{
    std::lock_guard<std::mutex> locker{mux};
    if (codec) {
        // closing rtsp stream will corrupt.
        // I have no idea.
        avcodec_close(codec);
        avcodec_free_context(&codec);
    }
    pts = 0;
}

void XDecoder::clear()
{
    std::lock_guard<std::mutex> locker{mux};
    if (codec) avcodec_flush_buffers(codec);
}