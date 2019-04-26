// AAC raw stream parser

// convert AAC to adts online(if you don't have adts format aac file)
// https://onlineconvertfree.com/convert-format/aac-to-adts/

// Audio Data Transport Stream (ADTS) is a format, used by MPEG TS or Shoutcast to stream audio, usually AAC.
// 
// Structure
// AAAAAAAA AAAABCCD EEFFFFGH HHIJKLMM MMMMMMMM MMMOOOOO OOOOOOPP (QQQQQQQQ QQQQQQQQ)
// Letter Length Description
//   A	    12	 syncword 0xFFF, all bits must be 1
//   B	    1	   MPEG Version: 0 for MPEG-4, 1 for MPEG-2
//   C	    2	   Layer: always 0
//   D	    1	   protection absent, Warning, set to 1 if there is no CRC and 0 if there is CRC
//   E	    2	   profile, the MPEG-4 Audio Object Type minus 1
//   F	    4	   MPEG-4 Sampling Frequency Index (15 is forbidden)
//   G	    1	   private bit, guaranteed never to be used by MPEG, set to 0 when encoding, ignore when decoding
//   H	    3	   MPEG-4 Channel Configuration (in the case of 0, the channel configuration is sent via an inband PCE)
//   I	    1	   originality, set to 0 when encoding, ignore when decoding
//   J	    1	   home, set to 0 when encoding, ignore when decoding
//   K	    1	   copyrighted id bit, the next bit of a centrally registered copyright identifier, set to 0 when encoding, ignore when decoding
//   L	    1	   copyright id start, signals that this frame's copyright id bit is the first bit of the copyright id, set to 0 when encoding, ignore when decoding
//   M	    13	 frame length, this value must include 7 or 9 bytes of header length: FrameLength = (ProtectionAbsent == 1 ? 7 : 9) + size(AACFrame)
//   O	    11	 Buffer fullness
//   P	    2	   Number of AAC frames (RDBs) in ADTS frame minus 1, for maximum compatibility always use 1 AAC frame per ADTS frame
//   Q	    16	 CRC if protection absent is 0
// more detail: https://wiki.multimedia.cx/index.php/ADTS

#include <iostream>
#include <utility>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <iomanip>
#include <chrono> // for calculating the duration of execution

const std::unordered_map<uint8_t, std::string> Audio_object_type_map = {
  // { 0, "NULL" },
  { 0, "Main" },
  { 1, "LC"   },
  { 2, "SSR"  },
  { 3, "LTP"  }
};

const std::unordered_map<uint8_t, std::string> Sampling_frequency_map = {
  { 0, "96000 Hz" },
  { 1, "88200 Hz" },
  { 2, "64000 Hz" },
  { 3, "48000 Hz" },
  { 4, "44100 Hz" },
  { 5, "32000 Hz" },
  { 6, "24000 Hz" },
  { 7, "22050 Hz" },
  { 8, "16000 Hz" },
  { 9, "12000 Hz" },
  { 10, "11025 Hz" },
  { 11, "8000 Hz"  },
  { 12, "7350 Hz"  },
  { 13, "Reserved" },
  { 14, "Reserved" }
};

bool IsAdtsSyncword(const char* data)
{
  if (((uint8_t) data[0] == 0xff) && ((uint8_t) (data[1] & 0xf0) == 0xf0)) {
    return true;
  }
  return false;
}

int main(int argc, char* argv[])
{
  std::ifstream infile("test.adts", std::ios::binary);
  std::stringstream sstr;
  sstr << infile.rdbuf();
  std::string data = sstr.str();

  std::size_t pos = 0;
  uint32_t num = 1;
  auto start = std::chrono::high_resolution_clock::now();
  while (pos < (data.size() - 1) && IsAdtsSyncword(&data[pos])) {
      // AAAAAAAA AAAABCCD EEFFFFGH HHIJKLMM MMMMMMMM MMMOOOOO OOOOOOPP (QQQQQQQQ QQQQQQQQ)
      //   E	    2	   profile, the MPEG-4 Audio Object Type minus 1
      //   F	    4	   MPEG-4 Sampling Frequency Index (15 is forbidden)
      uint8_t profile = ((uint8_t) (data[pos+2]) & 0x60) >> 6;
      uint8_t sampling_frequency_idx = ((uint8_t) (data[pos+2]) & 0x3c) >> 2;
      uint16_t frame_length = 0;
      frame_length |= (data[pos+3] & 0x03) << 11;   // high 2 bits
      frame_length |= ((uint8_t) data[pos+4]) << 3; // middle 8 bits, NOTE: take care of the overflow of signed char
      frame_length |= (data[pos+5] & 0xe0) >> 5;    // low 3 bits

      // std::cout
      //   << std::setw(5) << num << " | "
      //   << std::setw(5) << Audio_object_type_map.at(profile) << " | "
      //   << std::setw(8) << Sampling_frequency_map.at(sampling_frequency_idx) << " | "
      //   << std::setw(5) << frame_length  << " |"
      //   << std::setw(10) << pos << std::endl;
      pos += frame_length;
      ++num;
  }
  auto stop = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
  std::cout << "Total time: " << duration.count() << std::endl;

  return 0;
}