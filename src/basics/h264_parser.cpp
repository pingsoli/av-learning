// H264 annexb format parser not for rtp payload format

// Annex B H264 stream format
// +------------+-----+
// | start code | NAL |
// +------------+-----+
// start code is 0x00 00 01 or 0x00 00 00 01
//
// NAL format
// +---+-----+------+
// | F | NRI | TYPE |
// +---+-----+------+

#include <iostream>
#include <unordered_map>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>

enum class NaluPriority {
  disposble = 0,
  low       = 1,
  high      = 2,
  highest   = 3
};

struct NALU {
  uint8_t forbidden_bit;
  uint8_t nal_reference_idc; // NALU priority
  uint8_t nal_unit_type;
  char* buf;
  uint32_t len;
  uint32_t max_size;
};

static const std::unordered_map<int, std::string> NALU_type_map = {
  {  1, "SLICE" },    // Coded slice
  {  2, "DPA" },      // Coded slice data partition A
  {  3, "DPB" },      // Coded slice data partition B
  {  4, "DPC" },      // Coded slice data partition C
  {  5, "IDR" },      // Coded slice of an IDR picture
  {  6, "SEI" },      // Suplemental enhancement information
  {  7, "SPS" },      // Sequence parameter set
  {  8, "PPS" },      // Picture parameter set
  {  9, "AUD" },      // Picture dilimiter
  { 10, "EOSEQ" },    // End of sequence
  { 11, "EOSTREAM" }, // End of stream
  { 12, "FILL" }      // Filler data
};

static const std::unordered_map<int, std::string> NALU_priroity_map = {
  { 0, "Disposble" },
  { 1, "Low"},
  { 2, "High" },
  { 3, "Highest" }
};

int get_start_code_length(const std::string& data)
{
  std::size_t pos = -1;
  if ((pos = data.find(0x01)) != std::string::npos)
    return ++pos;

  return -1;
}

int main(int argc, char* argv[])
{
  std::ifstream infile("test.h264", std::ios::binary);
  std::stringstream sstr;

  // read the whole file
  sstr << infile.rdbuf();
  std::string data(sstr.str());

  std::vector<std::size_t> poss; // position set for every nalu location
  uint64_t num = 0;

  // start code as needle, 0x00 00 01 or 0x00 00 00 01
  std::size_t start_code_len = get_start_code_length(data);
  std::string needle(start_code_len - 1, 0x00);
  needle += 0x01;

  NALU nalu;
  std::size_t pos = 0;

  std::cout
    << "-----+----------+------------+------------+" << '\n'
    << " NUM | POSITION |  PRIORITY  |    TYPE    |" << '\n'
    << "-----+----------+------------+------------+" << '\n';
  while ((pos = data.find(needle, pos)) != std::string::npos) {
    
    std::size_t data_pos = pos + start_code_len;
    nalu.forbidden_bit     = (data[data_pos] & 0x80) >> 7;
    nalu.nal_reference_idc = (data[data_pos] & 0x60) >> 5;
    nalu.nal_unit_type     = data[data_pos] & 0x1f;

    std::cout
      << std::setw(4) << num << " | "
      << std::setw(8) << pos << " | "
      << std::setw(10) << NALU_priroity_map.at(nalu.nal_reference_idc) << " | "
      << std::setw(10) << NALU_type_map.at(nalu.nal_unit_type) << " |"
      << std::endl;

    poss.push_back(pos);
    pos += start_code_len; // jump to data to continue to find start code
    ++num; // next block
  }

  return 0;
}