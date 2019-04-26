// MP3 file parser
// NOTE: mp3 file encapsulated by ID3v2.4 tag(fixed 10 bytes header at the beginning)

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <array>
#include <chrono>
#include <iomanip>
#include <vector>

struct IDv3Tag {
  char tag[4];
  uint16_t version;
  uint8_t flags;
  uint32_t size; // exclude the tag header(fixed 10 bytes)
};

// The ID3v2 tag size is encoded with four bytes where the most significant bit (bit 7)
// is set to zero in every byte, making a total of 28 bits. The zeroed bits are ignored,
// so a 257 bytes long tag is represented as $00 00 02 01.
uint32_t CalcIDv3DataSize(uint32_t value)
{
  uint32_t v = 0;
  char c[4] = { 0 };
  memcpy(&c, &value, 4);
  v |= (c[0] & 0x7f) << 21;
  v |= (c[1] & 0x7f) << 14;
  v |= (c[2] & 0x7f) << 7;
  v |= (c[3] & 0x7f);
  return v;
}

#pragma pack(push, 1)
struct MP3Header {
  uint16_t error_protection : 1;
  uint16_t layer     : 2;
  uint16_t version   : 1;
  uint16_t sync_word : 12;

  uint8_t priv_bit  : 1;
  uint8_t pad_bit   : 1;
  uint8_t frequency : 2; // 00 == 44100 Hz
  uint8_t bit_rate  : 4;

  uint8_t emphasis : 2;
  uint8_t original : 1;
  uint8_t copy     : 1;
  uint8_t mode_ex  : 2; // mode extension (Used with Joint Stereo)
  uint8_t mode     : 2; // 01 == Joint Stereo
};
#pragma pack(pop)

template <typename F>
void Execution(F func)
{
  using namespace std::chrono;
  auto start = high_resolution_clock::now();
  func();
  auto end = high_resolution_clock::now();
  std::cout << "Execution Elapsed: "
    << duration_cast<milliseconds>(end - start).count()
    << " ms\n";
}

int main(int argc, char* argv[])
{
  std::ifstream infile("G:\\av-learning\\bin\\win32\\out.mp3", std::ios::binary);
  if (!infile.is_open()) {
    std::cerr << "file is not opened" << std::endl;
    exit(EXIT_FAILURE);
  }

  std::stringstream sstr;
  std::string data;

  Execution([&infile, &sstr, &data] {
     sstr << infile.rdbuf();
     data = sstr.str();
  });

  std::size_t pos = 0;
  IDv3Tag tag;

  std::array<char, 10> tag_header;
  memcpy(&tag_header, &data[0], 10);

  memcpy(&tag.size, &tag_header[6], 4);
  // Execution([&tag] { tag.size = CalcIDv3DataSize(tag.size); });
  tag.size = CalcIDv3DataSize(tag.size);
  pos = tag.size + 10;

  std::size_t count = 0;
  std::vector<std::size_t> frame_poss;
  frame_poss.reserve(100000);
  
  std::vector<std::size_t> fffb_poss;
  fffb_poss.reserve(100000);
  // char syncword[] = { 0xff, 0xfb, 0x00 };
  char syncword[] = { 0xff, 0xfa, 0x00 };
  std::string needle(syncword);

  while ((pos = data.find(needle, pos)) != std::string::npos)
  {
    fffb_poss.push_back(pos);
    ++count;
    pos += 2;
  }
  std::cout << "Count: " << count << std::endl;
  return 0;

  auto start = std::chrono::high_resolution_clock::now();
  while ((pos = data.find((char) 0xff, pos)) != std::string::npos)
  {
    if ((data[pos+1] & 0xf0) == 0xf0) {
      frame_poss.push_back(pos);
      pos += 2;
      ++count;
    } else {
      ++pos;
    }
  }
  auto end = std::chrono::high_resolution_clock::now();
  std::cout << "Total frame count: " << count << '\n'
    << "Scanning elapsed time: "
    << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n";

  return 0;
}