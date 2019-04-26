// RTSP over tcp, and rtp payload for h264 stream format
// Capture the rtsp live stream and store the data as file

#include <iostream>
#include <fstream>
#include <sstream>

#if defined(_WIN32) || defined(_WIN64)
#include <winsock.h>
#elif __linux__
#include <arpa/inet.h>
#endif

#pragma pack(push, 1)
typedef struct {
  uint8_t doller; // '$'
  uint8_t type;   // stream type(video, video rtcp, audio and audio rtcp)
  uint16_t len;   // data len
} RTSPInterleavedFrame;

// NOTE: we suppose no Contribution Source List in RTP header
typedef struct {
  uint8_t cc : 4; // contribution count
  uint8_t x  : 1; // extention part
  uint8_t p  : 1; // padding
  uint8_t v  : 2; // version
  uint8_t pt : 7; // payload type
  uint8_t m  : 1; // marker
  uint16_t seq;   // sequence number
  uint32_t timestamp;
  uint32_t ssrc;
} RTPHeader;
#pragma pack(pop)

void PrintRTSPInterleavedFrameInfo(const RTSPInterleavedFrame& frame)
{
  std::cout << "Doller: " << frame.doller << '\n'
            << "Type  : " << (int) frame.type << '\n'
            << "Length: " << frame.len
            << std::endl;
}

void PrintRTPHeaderInfo(const RTPHeader& rtp) {
  std::cout << "Version        : " << (int) rtp.v   << '\n'
            << "Padding        : " << (int) rtp.p   << '\n'
            << "Extention      : " << (int) rtp.x   << '\n'
            << "CC             : " << (int) rtp.cc  << '\n'
            << "Payload Type   : " << (int) rtp.pt  << '\n' 
            << "Marker         : " << (int) rtp.m   << '\n'
            << "Sequence Number: " << rtp.seq       << '\n'
            << "Timestamp      : " << rtp.timestamp << '\n'
            << "SSRC           : " << rtp.ssrc
            << std::endl;
}

int main(int argc, char* argv[])
{
  std::ifstream infile("rtsp_over_tcp_h264.dat.2", std::ios::binary);

  // Read entire file and store to std::string
  std::stringstream sstr;
  sstr << infile.rdbuf();
  RTSPInterleavedFrame interleavedFrame;
  RTPHeader rtpHeader;

  // Suppose stream type 0 is video, 2 is audio
  while (sstr.read(reinterpret_cast<char*>(&interleavedFrame), sizeof(RTSPInterleavedFrame))) {
    sstr.read(reinterpret_cast<char*>(&rtpHeader), sizeof(RTPHeader));

    interleavedFrame.len = ntohs(interleavedFrame.len);
    rtpHeader.seq = ntohs(rtpHeader.seq);
    rtpHeader.timestamp = ntohl(rtpHeader.timestamp);
    rtpHeader.ssrc = ntohl(rtpHeader.ssrc);

    PrintRTSPInterleavedFrameInfo(interleavedFrame);
    PrintRTPHeaderInfo(rtpHeader);

    // Skip to ignore the h.264 data
    uint64_t pos = sstr.tellg();
    sstr.seekg(pos + interleavedFrame.len - sizeof(RTPHeader));
    // std::cout << pos << " : " << sstr.tellg() << std::endl;
    std::cout << "============================" << std::endl;

    if (sstr.fail() || sstr.bad()|| sstr.eof()) break;
  }

  return 0;
}