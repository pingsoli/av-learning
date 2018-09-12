// NOTE:if you see the picutre, but it's messed up.
// check the video width and height again, make sure they are same.
// At last, we add a seekbar before the OpenGLWidget, just want to see
// the seekbar will not influenced by opengl repaint window.

// TIPS: make visual studio subsystem is console, check the log output on console.
#include <QDebug>
#include <QTimer>
#include <iostream>

#include "XVideoWidget.h"

extern "C" {
#include "libavutil/frame.h"
#include <string.h>
}

// add double quote automatically, make 'x' is string.
#define GET_STR(x)  #x
#define A_VER 3
#define T_VER 4

// Vertex shader
const char *vString = GET_STR(
    attribute vec4 vertexIn;
    attribute vec2 textureIn;
    varying vec2 textureOut;

    void main(void)
    {
        gl_Position = vertexIn;
        textureOut = textureIn;
    }
);

// Fragment shader
const char *fString = GET_STR(
    varying vec2 textureOut;
    uniform sampler2D tex_y;
    uniform sampler2D tex_u;
    uniform sampler2D tex_v;

    void main(void)
    {
        vec3 yuv;
        vec3 rgb;
        yuv.x = texture2D(tex_y, textureOut).r;
        yuv.y = texture2D(tex_u, textureOut).r - 0.5;
        yuv.z = texture2D(tex_v, textureOut).r - 0.5;

        rgb = mat3(1.0, 1.0, 1.0,
            0.0, -0.39465, 2.03211,
            1.13983, -0.58060, 0.0) * yuv;

        gl_FragColor = vec4(rgb, 1.0);
    }
);

// prepare yuv data for testing, put the output file to current project directory.
// ffmpeg -i test.mp4 -t 10 -s 240x128 -pix_fmt yuv420p out420x128.yuv
XVideoWidget::XVideoWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
}

XVideoWidget::~XVideoWidget()
{
}

void XVideoWidget::repaint(AVFrame* frame)
{
    if (!frame) return;

    {
        // line alignment problem.
        std::lock_guard<std::mutex> locker{mux};

        // make sure data, video height and width are not empty.
        if (!datas[0]
            || (width * height == 0)
            || frame->width != width
            || frame->height != height) return;

        if (width == frame->linesize[0]) // line is already aligned.
        {
            memcpy(datas[0], frame->data[0], width * height);
            memcpy(datas[1], frame->data[1], width * height / 4);
            memcpy(datas[2], frame->data[2], width * height / 4);
        } else {
            // needs line alignment operations.
            for (int i = 0; i < height; ++i)
                memcpy(datas[0] + width * i, frame->data[0] + frame->linesize[0] * i, width);
            for (int i = 0; i < height / 2; ++i)
                memcpy(datas[1] + (width / 2) * i, frame->data[1] + frame->linesize[1] * i, width);
            for (int i = 0; i < height / 2; ++i)
                memcpy(datas[2] + (width / 2) * i, frame->data[2] + frame->linesize[2] * i, width);
        }
    }

    // IMPORTANT.
    av_frame_free(&frame);

    // refresh screen, render.
    update();
}

void XVideoWidget::init(int width_, int height_)
{
    std::lock_guard<std::mutex> locker{mux};
    width = width_;
    height = height_;

    delete datas[0];
    delete datas[1];
    delete datas[2];

    // allocate texture memory space
    datas[0] = new uint8_t[width * height];      // Y
    datas[1] = new uint8_t[width * height / 4];  // U
    datas[2] = new uint8_t[width * height / 4];  // V

    // release the texture.
    if (texs[0]) {
        glDeleteTextures(3, texs);
    }

    // create texture
    glGenTextures(3, texs);

    // a 4 Ys has 2 UVs.
    // Y vector
    glBindTexture(GL_TEXTURE_2D, texs[0]);
    // mag filter and linear insertion operation. GL_LINEAR has low efficiency
    // GL_NEAREST has high efficiency, but low quality of piture.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // create texture space for GPU
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    // U vector
    glBindTexture(GL_TEXTURE_2D, texs[1]);
    // mag filter and linear insertion operation.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // create texture space for GPU
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width / 2, height / 2, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    // V vector
    glBindTexture(GL_TEXTURE_2D, texs[2]);
    // mag filter and linear insertion operation.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // create texture space for GPU
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width / 2, height / 2, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
}

// initialization.
void XVideoWidget::initializeGL()
{
    qDebug() << "initializeGL";
    std::lock_guard<std::mutex> locker{mux};

    // initialize opengl functions inherited from QOpenGLFunctions
    initializeOpenGLFunctions();

    // load shader scripts from qt program. 
    qDebug() << "add fragment from source: " << program.addShaderFromSourceCode(QGLShader::Fragment, fString);
    qDebug() << "add vertex from source:   " << program.addShaderFromSourceCode(QGLShader::Vertex, vString);

    // set vertex coordinate
    program.bindAttributeLocation("vertexIn", A_VER);

    // set texture coordinate
    program.bindAttributeLocation("textureIn", T_VER);

    // compiler shader
    qDebug() << "compile shader: " << program.link();

    // bind shader
    qDebug() << "bind shader:    " << program.bind();

    // pass vertex and fragment coordinate
    // vertex coordinate
    static const GLfloat ver[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f
    };

    // texture coordiante
    static const GLfloat tex[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f
    };

    // vertex
    glVertexAttribPointer(A_VER, 2, GL_FLOAT, 0, 0, ver);
    glEnableVertexAttribArray(A_VER);

    // texture
    glVertexAttribPointer(T_VER, 2, GL_FLOAT, 0, 0, tex);
    glEnableVertexAttribArray(T_VER);

    // get texture from shader
    unis[0] = program.uniformLocation("tex_y");
    unis[1] = program.uniformLocation("tex_u");
    unis[2] = program.uniformLocation("tex_v");
}

// paint window every time.
void XVideoWidget::paintGL()
{
    std::lock_guard<std::mutex> locker{mux};
    glActiveTexture(GL_TEXTURE0); // 0 layer
    glBindTexture(GL_TEXTURE_2D, texs[0]);  // 0 layer binds to texture Y
    // modify texture (copy datas)
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, datas[0]);
    // associate with shader unis variable
    glUniform1i(unis[0], 0);    // 0 layer


    glActiveTexture(GL_TEXTURE1); // 1 layer
    glBindTexture(GL_TEXTURE_2D, texs[1]);  // 1 layer binds to texture U
    // modify texture (copy datas)
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_RED, GL_UNSIGNED_BYTE, datas[1]);
    // associate with shader unis variable
    glUniform1i(unis[1], 1);    // 1 layer


    glActiveTexture(GL_TEXTURE2); // 2 layer
    glBindTexture(GL_TEXTURE_2D, texs[2]);  // 2 layer binds to texture V
    // modify texture (copy datas)
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_RED, GL_UNSIGNED_BYTE, datas[2]);
    // associate with shader unis variable
    glUniform1i(unis[2], 2);    // 2 layer


    // draw a picture.
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void XVideoWidget::resizeGL(int width, int height)
{
    std::lock_guard<std::mutex> locker{mux};
    qDebug() << "resizeGL:" << width << "x" << height;
}