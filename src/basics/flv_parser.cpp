// FLV parser

// FLV Header
//    3 bytes    1 byte   1 byte  4 bytes
// | Signature | Version | Flags | Size |
// 
// Flv tags
// | type | size | tiemstamp | timestamp_ex | stream id | data |

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#elif __linux__
#include <arpa/inet.h>
#include <string.h> // memcpy()
#endif

struct FLVHeader {
  char signature[4]; // "FLV\0"
  uint8_t version;
  uint8_t flags;     // first 5 bits are reserved, is 0, 6th bit stands for exsiting audio, 7th bit is 0
  uint32_t size;     // header size

  std::size_t Read(const char* data) {
    memcpy(signature, data, 3);
    signature[3] = '\0';
    version = data[3];
    flags = data[4];
    memcpy(&size, &data[5], sizeof(size));
    size = ntohl(size);

    return 9; // return header size
  }
} ;

struct FLVTag {
  uint8_t type; // audio, video or script data
  uint32_t size;  // tag data size, exclude Tag fixed header part(such as type, size and etc.)
  uint32_t timestamp;
  uint32_t reserved; // timestamp extension and stream id
  uint8_t* data;
};

struct MetaData {
  double duration;
  double width;
  double height;
  double video_data_rate;
  double frame_rate;
  double video_codec_id;
  bool stereo;
  double audio_sample_rate;
  double audio_sample_size;
  double audio_codec_id;
  double filesize; // total bytes of the file
};

using ui8s_map_type = std::unordered_map<uint8_t, std::string>;
const ui8s_map_type Tag_type_map = {
  { 0x08, "AUDIO" },
  { 0x09, "VIDEO" },
  { 0x12, "SCRIPT" }
};

const ui8s_map_type Audio_codec_map = {
  { 0, "Linear PCM (platform endian)" },
  { 1, "ADPCM" },
  { 2, "MP3" },
  { 3, "Linear PCM (little endian)" },
  { 4, "Nellymoser 16 kHz mono" },
  { 5, "Nellymoser 8 kHz mono" },
  { 6, "Nellymoser" },
  { 7, "G.711 A-law logarithmic PCM" },
  { 8, "G.711 mu-law logarithmic PCM" },
  { 9, "Reserved" },
  { 10, "AAC" },
  { 14, "MP3 8 kHz" },
  { 15, "Device Specific Sound" }
};

const ui8s_map_type Audio_sample_rate_map = {
  { 0, "5.5 kHz" },
  { 1, "11 kHz" },
  { 2, "22 kHz" },
  { 3, "44 khz" }
};

const ui8s_map_type Video_frame_type_map = {
  { 1, "Key Frame" }, // for AVC, a seekable frame
  { 2, "Inter Frame" }, // for AVC, a nonseekable frame
  { 3, "Disposble Inter Frame" }, // H.263 only
  { 4, "Generated Key Frame" }, // reserved for server use
  { 5, "Video Info/Command Frame" }
};

const ui8s_map_type Video_codec_type_map = {
  { 1, "JPEG" },
  { 2, "Sorenson H.263" },
  { 3, "Screen Video" },
  { 4, "On2 VP6" },
  { 5, "On2 Vp6 with alpha channel" },
  { 6, "Screen Video Version 2" },
  { 7, "AVC" }
};

void Read3BytesToUI32(void* dst, char *src, std::size_t pos)
{
  uint32_t* dest = static_cast<uint32_t*>(dst);
  memset(dest, 0, 4);
  memcpy(dest, src+pos, 3);
  *dest <<= 8;
  *dest = ntohl(*dest);
}

double be64tole(uint64_t v) {
  char *c = reinterpret_cast<char*>(&v);
  std::reverse(c, c+8);
  return (*reinterpret_cast<double*>(&v));
}

void GetMetaProperty(const std::string& meta, const std::string& property, std::size_t size, void *value)
{
  std::size_t pos = 0;
  double* v = static_cast<double*>(value);
  if ((pos = meta.find(property)) != std::string::npos) {
    memcpy(v, &meta[pos+property.length()+1], size);
    switch (size)
    {
      case 1: break;
      case 8:
        *v = be64tole(*((uint64_t *) value));
        break;
      default: break;
    }
  } else {
    memset(v, 0, size);
  }
}

void ReadMetaData(char *data, std::size_t size)
{
  std::string metaData(data, data+size);
  MetaData meta;

  std::unordered_map<std::string, double*> properties = {
    { "duration", &meta.duration },
    { "width", &meta.width },
    { "height", &meta.height },
    { "videodatarate", &meta.video_data_rate },
    { "framerate", &meta.frame_rate },
    { "videocodecid", &meta.video_codec_id },
    { "audiosamplerate", &meta.audio_sample_rate },
    { "audiosamplesize", &meta.audio_sample_size },
    { "audiocodecid", &meta.audio_codec_id },
    { "filesize", &meta.filesize },
  };

  for (auto& p : properties) {
    GetMetaProperty(metaData, p.first, sizeof(double), p.second);
    std::cout << p.first << " : " << std::setprecision(10) << *p.second << std::endl;
  }
  GetMetaProperty(metaData, "stereo", sizeof(bool), &meta.stereo);
}

int main(int argc, char* argv[])
{
  std::ifstream infile("test.flv", std::ios::binary);
  std::stringstream sstr;
  sstr << infile.rdbuf();
  std::string data = sstr.str();

  FLVHeader header;
  std::size_t pos = 0;

  pos = header.Read(data.c_str());

  FLVTag tag;
  pos += 4; // Ignore first previous tag size field
  while (pos < data.size()) {

    tag.type = data[pos];
    Read3BytesToUI32(&tag.size, &data[pos], 1);
    Read3BytesToUI32(&tag.timestamp, &data[pos], 4);
    tag.data = (uint8_t*) &data[pos+11];

    std::cout 
      << std::setw(6) << Tag_type_map.at(tag.type) << " | "
      << std::setw(6) << +tag.size << " | "
      << std::setw(10) << +tag.timestamp << " | "
      << std::setw(10) << pos << " | ";

    switch (tag.type)
    {
      case 0x08: // Audio tag
        {
          uint8_t audio_codec_type = (tag.data[0] & 0xf0) >> 4;
          uint8_t sample_rate = (tag.data[0] & 0x0c) >> 2;
          uint8_t precision = (tag.data[0] & 0x02) >> 1;
          uint8_t audio_type = tag.data[0] & 0x01;

          std::cout << Audio_codec_map.at(audio_codec_type) << " | "
            << Audio_sample_rate_map.at(sample_rate) << " | "
            << (precision ? "16 bits" : "8 bits") << " | "
            << (audio_type ? "sndStereo" : "sndMono") << " |\n";
        }
        break;

      case 0x09: // Video tag
        {
          uint8_t video_frame_type = (tag.data[0] & 0xf0) >> 4;
          uint8_t video_codec_type = (tag.data[0] & 0x0f);
          std::cout << Video_frame_type_map.at(video_frame_type) << " | "
            << Video_codec_type_map.at(video_codec_type) << " |\n";
        }
        break;

      case 0x12: // Script tag
        std::cout << std::endl;
        ReadMetaData((char*)tag.data, tag.size);
        break;

      default:
        std::cerr << "Unkown tag type" << std::endl;
        exit(-1);
    }

    pos += tag.size + 15; // next tag, previous tag size field in the stream
  }

  return 0;
}