#include <mutex>
#include <iostream>

#include <QAudioOutput>

#include "XAudioPlayer.h"

// do the real jobs, and hide the real implementation in behind.
class CXAudioPlayer : public XAudioPlayer
{
public:
    virtual bool open()
    {
        QAudioFormat fmt;

        fmt.setSampleRate(sample_rate);
        fmt.setSampleSize(sample_size);
        fmt.setChannelCount(channels);
        fmt.setCodec("audio/pcm");
        fmt.setByteOrder(QAudioFormat::LittleEndian);
        fmt.setSampleType(QAudioFormat::UnSignedInt);

        std::lock_guard<std::mutex> locker{mux};
        output = new QAudioOutput(fmt);
        io = output->start(); // play audio data
        return (io ? true : false);
    }

    virtual void close() {
        std::lock_guard<std::mutex> locker{ mux };
        if (io) {
            io->close();
            // NOTE: Don't delete QIODevice.
            // or the QAudioOutput will occur access violation.
            // from the code, QIODevice is got from QAudioOutput.
            // do not `delete io;`, just set it to nullptr.
            io = nullptr;
        }

        if (output) {
            output->stop();
            delete output;
            output = nullptr;
        }
    }

    virtual void clear() {
        std::lock_guard<std::mutex> locker{mux};
        if (io) io->reset();
    }

    virtual bool write(const uint8_t *data, int size)
    {
        if (!data || size <= 0) return false;

        {
            std::lock_guard<std::mutex> locker{mux};
            if (!output || !io) return false;
            int wrote_size = io->write((char*)data, size);
            if (size != wrote_size) return false;
        }

        return true;
    }

    virtual void setSampleRate(int sample_rate_) {
        std::lock_guard<std::mutex> locker{mux};
        sample_rate = sample_rate_;
    }

    virtual void setChannels(int channels_) {
        std::lock_guard<std::mutex> locker{mux};
        channels = channels_;
    }

    virtual void setPause(bool isPause_) {
        std::lock_guard<std::mutex> locker{mux};
        if (!output) return;

        if (isPause_) {
            output->suspend();
        } else {
            output->resume();
        }
    }

    virtual int get_free() {
        std::lock_guard<std::mutex> locker{mux};
        return (output ? output->bytesFree() : 0);
    }

    // for audio and video synchronization.
    virtual int64_t getNoPlayMs() {
        int64_t pts = 0;
        std::lock_guard<std::mutex> locker{mux};
        if (!output) return 0;
        // unplay bytes.
        double size = output->bufferSize() - output->bytesFree();

        // one second audio size.
        double secSize = sample_rate * (sample_size / 8) * channels;
        if (secSize <= 0) pts = 0;
        else pts = (size / secSize) * 1000;

        return pts;
    }

private:
    std::mutex mux; // protect all members.
    QAudioOutput *output = nullptr;
    QIODevice *io = nullptr;
};

// singleton pattern.
XAudioPlayer* XAudioPlayer::get_instance() {
    static CXAudioPlayer audioPlayer;
    return &audioPlayer;
}

XAudioPlayer::XAudioPlayer()
{
}

XAudioPlayer::~XAudioPlayer()
{
}
