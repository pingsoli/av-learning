#ifndef _DECODER_H_
#define _DECODER_H_

struct AVCodec;
struct AVCodecContext;

class MediaInfo;

class Decoder {
public:
  Decoder();
  ~Decoder();
  int Open(const MediaInfo&);
  AVCodecContext* GetVideoCodecContext() { return videoCodecCtx_; }
  AVCodecContext* GetAudioCodecContext() { return audioCodecCtx_; }

private:
  AVCodecContext* videoCodecCtx_;
  AVCodecContext* audioCodecCtx_;
  AVCodec* videoCodec_;
  AVCodec* audioCodec_;
};

#endif