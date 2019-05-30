#include <QtCore/QCoreApplication>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QThread>

#include <fstream>
#include <iostream>

// prepare source data
// ffmpeg -i test.mp4 -f s16le output.pcm

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QAudioFormat fmt;

    fmt.setSampleRate(44100);
    fmt.setSampleSize(16);
    fmt.setChannelCount(2);
    fmt.setCodec("audio/pcm");
    fmt.setByteOrder(QAudioFormat::LittleEndian);
    fmt.setSampleType(QAudioFormat::UnSignedInt);

    QAudioOutput *out = new QAudioOutput(fmt);

    QIODevice *io = out->start(); // play audio data

    int size = out->periodSize();
    char *buf = new char[size];

    FILE *fp = fopen("output.pcm", "rb");
    if (fp == nullptr) {
        std::cout << "cannot open file" << std::endl;
        return -1;
    }

    while (!feof(fp)) {
        if (out->bytesFree() < size) {
            QThread::msleep(1);
            continue;
        }

        int len = fread(buf, 1, size, fp);
        if (len <= 0) break;
        io->write(buf, len);
    }

    // It's bad.
    delete buf;
    buf = nullptr;

    fclose(fp);

    return a.exec();
}
