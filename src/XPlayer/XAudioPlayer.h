#pragma once

// Singleton pattern, provided for user.
class XAudioPlayer
{
public:
    static XAudioPlayer* get_instance();

    // set audio parameters, and prepare to play audio.
    virtual bool open() = 0;

    // close QIODevice, stop playing audio, delete QAudioOutput handle.
    virtual void close() = 0;
    // clear buffer for seeking operations.
    virtual void clear() = 0;

    // write data to audio device, then we will listen it.
    virtual bool write(const uint8_t* data, int size) = 0;

    // setters.
    virtual void setChannels(int channels_) = 0;
    virtual void setSampleRate(int sample_rate_) = 0;
    virtual void setPause(bool isPause_) = 0;

    // get free space size for writing(to voice card).
    virtual int get_free() = 0;

    // return the unplay buffer time (ms precision).
    // NOTE: we use Qt to play audio, there has unplayed buffer,
    // we must subtract the unplay buffer time to calculate pts.
    virtual int64_t getNoPlayMs() = 0;

protected:
    // default parameters for audio player.
    int sample_rate = 44100;
    int sample_size = 16; // 16 bits.
    int channels = 2;

    XAudioPlayer();
    virtual ~XAudioPlayer();

    XAudioPlayer(const XAudioPlayer&) = delete;
    XAudioPlayer& operator=(const XAudioPlayer&) = delete;
};

