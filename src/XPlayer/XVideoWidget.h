#pragma once

#include <mutex>

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QGLShaderProgram>

#include "IVideoCall.h"

struct AVFrame;

// play the video frame.
class XVideoWidget : public QOpenGLWidget,
    protected QOpenGLFunctions, public IVideoCall
{
    Q_OBJECT

public:
    XVideoWidget(QWidget *parent);
    ~XVideoWidget();

    // those funcitons are inherited from IVideoCall.
    // initialize 
    virtual void init(int width_, int height_) override;
    // release the frame no matter success or failed.
    virtual void repaint(AVFrame* frame) override;

protected:
    // these functions are inherited from QOpenGLWidget.
    void paintGL() override;
    void initializeGL() override;
    void resizeGL(int width, int height) override;

private:
    // shader program.
    QGLShaderProgram program;
    
    // shader yuv address
    GLuint unis[3] = { 0 };

    // opengl texture address
    GLuint texs[3] = { 0 };

    // texture space
    uint8_t *datas[3] = { 0 };

    // pixel resolution
    int width = 0;
    int height = 0;

    std::mutex mux; // protect all members.
};
