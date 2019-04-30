// pcm player with qt
// NOTE: you have to copy all qt .dll to executable's directory.

#include <fstream>
#include <array>
#include <thread>

#include <QApplication>
#include <QFile>
#include <QAudioFormat>
#include <QAudioOutput>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  QFile inputFile;
  inputFile.setFileName("test.pcm");
  inputFile.open(QIODevice::ReadOnly);

  QAudioFormat audioFormat;
  audioFormat.setSampleRate(48000);
  audioFormat.setChannelCount(2);
  audioFormat.setSampleSize(16);
  audioFormat.setCodec("audio/pcm");
  audioFormat.setByteOrder(QAudioFormat::LittleEndian);
  audioFormat.setSampleType(QAudioFormat::SignedInt);

  QAudioOutput *audio = new QAudioOutput(audioFormat, nullptr);
  // Read from data
  audio->start(&inputFile);

  // Read fixed data every time
  // auto streamout = audio->start();
  // std::ifstream infile("test.pcm", std::ios::binary);
  // std::array<char, 3840> buffer;
  // std::size_t read_size = 0;
  // using namespace std::chrono_literals;

  // while (infile.read(&buffer[0], buffer.max_size())) {
  //   read_size = infile.gcount();
  //   streamout->write(&buffer[0], read_size);

  //   std::this_thread::sleep_for(19ms);
  // }

  return a.exec();
}