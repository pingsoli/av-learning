#include <iostream>

#include "XDemuxer.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
}

// not used any more. it's AVFormatContext's callback.
static int interrupt_callback(void *ctx) {
    AVFormatContext *fctx = reinterpret_cast<AVFormatContext*>(ctx);
    if (!fctx) return 1;
    return 0;
}

XDemuxer::XDemuxer()
{
    // register all muxters, demuxters and protocols.
    av_register_all();

    // initialize network library (rtsp, rtmp, http).
    avformat_network_init();

    // preallocated for custom IO.
    //fctx = avformat_alloc_context();
}

XDemuxer::~XDemuxer()
{
}

bool XDemuxer::open(const std::string& url)
{
    AVDictionary *opts = nullptr;  // parameter options, such as rtsp max_delay.
    int ret_code = -1;

    // for network stream
    av_dict_set(&opts, "rtsp_transport", "tcp", 0); // using tcp protocol to open rtsp stream.
    av_dict_set(&opts, "max_delay",      "500", 0); // network max delay, may be useful in WAN.
    av_dict_set(&opts, "stimeout",   "2000000", 0); // 2s timeout.
    //av_dict_set(&opts, "bufsize",    "655360",  0); // 64k buffer.

    // clear and close the AVFormatContext.
    clear();
    close();

    std::lock_guard<std::mutex> locker{mux};

    // timeout (NOTE: make sure you preallocated AVFormatContext)
    //fctx->interrupt_callback.callback = interrupt_callback;
    //fctx->interrupt_callback.opaque = fctx;

    // avformat_opent_input is blocking and waits to return.
    // you can set the timeout value.
    ret_code = avformat_open_input(&fctx, url.c_str(), nullptr, &opts);
    if (ret_code != 0) {
        char buf[256] = { 0 };
        av_strerror(ret_code, buf, sizeof(buf) - 1);
        std::cout << "avformat_open_input " << url << " failed! error: " << buf << std::endl;
        return false;
    } else {
        std::cout << "avformat_open_input " << url << " successfully!" << std::endl;
    }

    // get stream info, necessary. if it's rtsp stream, it will preread the first key frame.
    // and get the complete stream information.
    ret_code = avformat_find_stream_info(fctx, nullptr);
    if (ret_code == 0) {
        // get video total ms.
        totalMs = fctx->duration / AV_TIME_BASE * 1000;
    }

    // print streams info on console.
    av_dump_format(fctx, 0, url.c_str(), 0);

    videoStream = av_find_best_stream(fctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    audioStream = av_find_best_stream(fctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);

    // it's all in AVCodecParameters object.
    // get video width and height
    width = fctx->streams[videoStream]->codecpar->width;
    height = fctx->streams[videoStream]->codecpar->height;

    // get audio sample rate and channels.
    sample_rate = fctx->streams[audioStream]->codecpar->sample_rate;
    channels = fctx->streams[audioStream]->codecpar->channels;

    return true;
}

AVPacket* XDemuxer::read()
{
    std::lock_guard<std::mutex> locker{mux};

    if (!fctx) {
        std::cout << "AVFormatContext has been not opened." << std::endl;
        return nullptr;
    }

    AVPacket* pkt = av_packet_alloc();

    // read packet and allocate memory.
    int ret = av_read_frame(fctx, pkt);
    if (ret != 0) {
        av_packet_free(&pkt);
        return nullptr;
    }

    // let pakcet's pts be milliseconds precision.
    pkt->pts = pkt->pts * av_q2d(fctx->streams[pkt->stream_index]->time_base) * 1000;
    pkt->dts = pkt->dts * av_q2d(fctx->streams[pkt->stream_index]->time_base) * 1000;

    //if (pkt->stream_index == videoStream) {
    //    std::cout << pkt->pts << " ";
    //} else if (pkt->stream_index == audioStream) {
    //    std::cout << pkt->pts << " ";
    //} else {
    //    std::cout << "unknown error";
    //}

    return pkt;
}

AVCodecParameters* XDemuxer::copyParams(int stream_index)
{
    std::lock_guard<std::mutex> locker{mux};
    if (!fctx) return nullptr;

    AVCodecParameters* copy_param = avcodec_parameters_alloc();
    avcodec_parameters_copy(copy_param, fctx->streams[stream_index]->codecpar);

    return copy_param;
}

AVCodecParameters* XDemuxer::copyVideoParams()
{
    return copyParams(videoStream);
}

AVCodecParameters* XDemuxer::copyAudioParams()
{
    return copyParams(audioStream);
}

bool XDemuxer::seek(double pos)
{
    int ret = -1;
    {
        std::lock_guard<std::mutex> locker{mux};
        if (!fctx) return false;

        // flush the network data. it's necessarry.
        avformat_flush(fctx);

        int64_t seekPos = fctx->streams[videoStream]->duration * pos;
        ret = av_seek_frame(fctx, videoStream, seekPos, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD);
    }

    return (ret >= 0 ? true : false);
}

void XDemuxer::clear()
{
    std::lock_guard<std::mutex> locker(mux);
    // flush read buffer
    if (fctx) avformat_flush(fctx);
}

void XDemuxer::close()
{
    std::lock_guard<std::mutex> locker(mux);
    if (fctx) avformat_close_input(&fctx);
}

bool XDemuxer::isAudio(AVPacket* pkt)
{
    if (!pkt) return false;

    std::lock_guard<std::mutex> locker{mux};
    return (pkt->stream_index == videoStream ? false : true);
}
