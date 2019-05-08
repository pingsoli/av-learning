#ifndef _MEDIA_INFO_H_
#define _MEDIA_INFO_H_

#include <string>

struct AVFormatContext;
struct AVCodecParameters;

class MediaInfo {
public:
  MediaInfo();
  ~MediaInfo();
  bool Open(const std::string& filename);
  AVFormatContext* GetFormatContext() { return avFormatCtx_; }
  AVCodecParameters *GetVideoCodecParameters() const;
  AVCodecParameters *GetAudioCodecParameters() const;
  int VideoStreamIndex() { return videoStreamIdx_; }
  int AudioStreamIndex() { return audioStreamIdx_; }
  int Width() const { return width_; }
  int Height() const { return height_; }
  std::string Filename() const { return filename_; }

private:
  std::string filename_;
  AVFormatContext* avFormatCtx_;

  int audioStreamIdx_;
  int videoStreamIdx_;

  int width_;
  int height_;
  uint64_t duration_; // milliseconds
};

#endif