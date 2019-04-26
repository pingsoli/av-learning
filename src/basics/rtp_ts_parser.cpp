// rtp transport stream parser
// MPEG TS is 188-byte format, seven units form a packet
// | RTP Header | TS Unit | TS Unit | ... | TS Unit |
// 12 + 188 * 7 = 1328 bytes

#include <iostream>
#include <array>
#include <iomanip>
#include <set>
#include <unordered_map>
#include <algorithm>
#include <thread>
#include <mutex>

#if defined(_WIN32) || defined(_WIN64)
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib") // WinSock library
#elif __linux__
#endif

// NOTE: UdpSocket works on loolback network only.
class UdpSocket {
public:
  UdpSocket(uint16_t local_port, bool reuse_address = true) 
    : local_port_(local_port),
      dest_len_(sizeof(dest_))
  {
    if ((socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
      std::cerr << "Error: socket failed" << std::endl;
      exit(EXIT_FAILURE);
    }

    memset(&local_, 0, sizeof(local_));
    local_.sin_family = AF_INET;
    local_.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    local_.sin_port = htons(local_port_);

    if (reuse_address) {
      int enable;
      if (setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, (const char*) &enable, sizeof(int)) < 0) {
        std::cerr << "Error: reuse address failed" << std::endl;
      }
    }

    if (bind(socket_, (const struct sockaddr*)&local_, sizeof(local_)) < 0) {
      std::cerr << "Error: bind failed" << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  SOCKET Socket() const {
    return socket_;
  }

  bool SetNonblocking() {
    return false;
  }

  int Receive(char* buf, std::size_t size) {
    int recv_size = recvfrom(socket_, buf, size,
      0, (struct sockaddr *)&dest_, &dest_len_);

    if (recv_size > max_recv_size_) {
      std::cout << "Warning: recv too much data("
                << recv_size << ") over buffer size, drop some data" << std::endl;
      return max_recv_size_;
    }

    return recv_size;
  }

private:
  void PrintDestinationInfo() {
    std::array<char, INET_ADDRSTRLEN> ip;
    inet_ntop(AF_INET, &(dest_.sin_addr), &ip[0], INET_ADDRSTRLEN);
    std::cout << "[" << ip.data() << ":" << ntohs(dest_.sin_port) << "] ";
  }

  uint16_t local_port_;
  const int max_recv_size_ = 8192;
  sockaddr_in local_;
  sockaddr_in dest_;
  int dest_len_;
  SOCKET socket_;
};

#pragma pack(push, 1)
// NOTE: we suppose no Contribution Source List in RTP header
// RTP fixed header size: 12 bytes
struct RTPHeader {
  uint8_t cc : 4; // contribution count
  uint8_t x  : 1; // extention part
  uint8_t p  : 1; // padding
  uint8_t v  : 2; // version
  uint8_t pt : 7; // payload type
  uint8_t m  : 1; // marker
  uint16_t seq;   // sequence number
  uint32_t timestamp;
  uint32_t ssrc;
};

struct MPEGTSHeader {
  uint32_t sync_byte                    : 8; // 0x47
  uint32_t transport_error_indicator    : 1; 
  uint32_t payload_unit_start_indicator : 1;
  uint32_t transport_priority           : 1;
  uint32_t pid                          : 13; // packet identifier(PID)
  uint32_t scrambling_control   : 2;
  uint32_t adaption_field_exist : 2; // 00 reserved, 01 no adaption, 10 adaptaion only, no payload, 11 adaption followed by payload
  uint32_t continunity_counter  : 4;

  // NOTE: value is big endian format from network
  void Read(uint32_t value) {
    sync_byte = (value & 0xff000000) >> 24;
    transport_error_indicator = (value & 0x00800000) >> 23;
    payload_unit_start_indicator = (value & 0x00400000) >> 22;
    transport_priority = (value & 0x00200000) >> 21;
    pid = (value & 0x001fff00) >> 8;
    scrambling_control = (value & 0xc0) >> 6;
    adaption_field_exist = (value & 0x30) >> 4;
    continunity_counter = value & 0x0f;
  }

  void Read(const char* data) {
    uint32_t value = 0;
    memcpy(&value, data, sizeof(value));
    value = htonl(value);
    Read(value);
  }

  void PrintInfo() {
    std::cout
      << "Sync byte: " << std::hex << sync_byte << '\n'
      << "Transport error indicator: " << transport_error_indicator << '\n'
      << "Payload uint start indicator: " << payload_unit_start_indicator << '\n'
      << "Transport priority: " << transport_priority << '\n'
      << "PID: " << pid << '\n'
      << "Scrambling control: " << scrambling_control << '\n'
      << "Adaption field exist: " << adaption_field_exist << '\n'
      << "Continuity counter: " << continunity_counter
      << std::endl;
  }
};
#pragma pack(pop)

void PrintRTPHeaderInfo(const RTPHeader&);
RTPHeader ReadRTPHeader(const char* data)
{
  RTPHeader header;
  memcpy(&header, data, 12);

  header.seq = ntohs(header.seq);
  header.timestamp = ntohl(header.timestamp);
  header.ssrc = ntohl(header.ssrc);

  return header;
}

void PrintRTPHeaderInfo(const RTPHeader& rtp) {
  std::cout
    << "Version        : " << (int) rtp.v   << '\n'
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

template <typename T>
T GetLEFromBE(const char* data)
{
  T value;
  memcpy(&value, data, sizeof(T));
  std::reverse(reinterpret_cast<char*>(&value),
    reinterpret_cast<char*>(&value)+sizeof(T));
  return value;
}

// Program Table
void ParsePAT(const MPEGTSHeader& ts, const char* data) {
  if (ts.pid == 0) {
    switch (ts.adaption_field_exist)
    {
      case 0: // reserved
      break;

      case 1: // payload only
      break;

      case 2: // adaption field only
      break;

      case 3: // adaption field followed by payload
      {
        uint8_t adaption_field_len = data[0];
        
        std::unordered_map<uint16_t, uint16_t> PAT_map;
        uint16_t program_number = GetLEFromBE<uint16_t>(&data[1 + adaption_field_len + 8]);
        uint16_t program_map_pid = GetLEFromBE<uint16_t>(&data[1 + adaption_field_len + 8 + 2]) & 0x1ffff;
        // std::cout << program_number << " : " << program_map_pid << std::endl;
      }
      break;
    }
  }
}

int main(int argc, char* argv[])
{
  const uint16_t udp_data_port = 10000;
  const uint16_t udp_rtcp_port = udp_data_port + 1;
  std::mutex cout_mutex;

  WSADATA data;
  WSAStartup(MAKEWORD(2, 2), &data);

  UdpSocket rtp_data_socket(udp_data_port);
  UdpSocket rtcp_socket(udp_rtcp_port);

  std::thread rtpThread(
    [&rtp_data_socket, &cout_mutex] {
      std::array<char, 8192> data_buffer = { 0 };
      std::unordered_map<uint32_t, uint64_t> pid_count_map;
      int recv_data_bytes = 0;
      std::size_t count = 0;

      while (true) {
        recv_data_bytes = rtp_data_socket.Receive(&data_buffer[0], data_buffer.max_size());
        if (recv_data_bytes == -1) continue;

        RTPHeader rtpHeader;
        rtpHeader = ReadRTPHeader(&data_buffer[0]);

        {
          std::lock_guard<std::mutex> lock(cout_mutex);
          std::cout
            << std::setw(4) << ++count << " | "
            << std::setw(4) << (rtpHeader.pt == 33 ? "MP2T" : "Unknown") << " | "
            << std::setw(6) << rtpHeader.seq << " | "
            << std::setw(10) << rtpHeader.ssrc << " | "
            << std::setw(10) << rtpHeader.timestamp << " | "
            << "\n";
        }

        int pos = 12;
        for (int i = 0; i < 7 && pos < recv_data_bytes; ++i) {
          MPEGTSHeader tsHeader;
          tsHeader.Read(&data_buffer[pos]);

          // std::cout
          //   << std::setw(8) << tsHeader.pid << " | "
          //   << tsHeader.adaption_field_exist << " | "
          //   << tsHeader.payload_unit_start_indicator << " | "
          //   << '\n';
          ParsePAT(tsHeader, &data_buffer[pos+4]);

          pid_count_map[tsHeader.pid]++;
          pos += 188; // next ts unit
        }
      }
  });

  std::thread rtcpThread([&rtcp_socket, &cout_mutex] {
    std::array<char, 8192> rtcp_buffer = { 0 };
    int recv_data_bytes = 0;
    std::size_t count = 0;

    while (true) {
      recv_data_bytes = rtcp_socket.Receive(&rtcp_buffer[0], rtcp_buffer.max_size());
      if (recv_data_bytes < 0) continue;
      
      {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "RTCP " << ++count << std::endl;
      }
    }
  });

  rtpThread.join();
  rtcpThread.join();

  return 0;
}