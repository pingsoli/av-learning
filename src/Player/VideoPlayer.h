#pragma once

// interface class for playing video.
// nice way to separate the higher level and lower level.
struct AVFrame;

class VideoPlayer
{
public:
    virtual void init(int width_, int height_) = 0;
    virtual void repaint(AVFrame* frame) = 0;
};