// NOTE:if you see the picutre, but it's messed up.
// check the video width and height again, make sure they are same.
// At last, we add a seekbar before the OpenGLWidget, just want to see
// the seekbar will not influenced by opengl repaint window.

// NOTE: make visual studio subsystem is console. you can see the log output.
#include "XVideoWidget.h"
#include <QDebug>
#include <QTimer>

// add double quote automatically.
#define GET_STR(x)  #x
#define A_VER 3
#define T_VER 4

const char *filename = "out420x128.yuv";

FILE *fp = nullptr;

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

// initialization.
void XVideoWidget::initializeGL()
{
    qDebug() << "initializeGL";

    // initialize opengl functions inherited from QOpenGLFunctions
    initializeOpenGLFunctions();

    // load shader scripts from qt program. 
    qDebug() << program.addShaderFromSourceCode(QGLShader::Fragment, fString);
    qDebug() << program.addShaderFromSourceCode(QGLShader::Vertex, vString);

    // set vertex coordinate
    program.bindAttributeLocation("vertexIn", A_VER);

    // set texture coordinate
    program.bindAttributeLocation("textureIn", T_VER);

    // compiler shader
    qDebug() << "compiler shader: " << program.link();

    // bind shader
    qDebug() << "bind shader: " << program.bind();

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


    // allocate texture memory space
    datas[0] = new uint8_t[width * height];      // Y
    datas[1] = new uint8_t[width * height / 4];  // U
    datas[2] = new uint8_t[width * height / 4];  // V

    // read texture(binary format) file (yuv format).
    fp = fopen(filename, "rb");
    if (!fp) {
        qDebug() << "open file" << filename << "failed!\n";
        exit(-1);
    }

    // start a timer
    QTimer *ti = new QTimer(this);
    connect(ti, SIGNAL(timeout()), this, SLOT(update()));
    ti->start(40);
}

// paint window every time.
void XVideoWidget::paintGL()
{
    // paint cyclically.
    if (feof(fp)) {
        fseek(fp, 0, SEEK_SET);
    }

    // read a set yuv data from file.
    fread(datas[0], 1, width * height,     fp); // Y
    fread(datas[1], 1, width * height / 4, fp); // U
    fread(datas[2], 1, width * height / 4, fp); // V


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

    qDebug() << "paintGL";
}

void XVideoWidget::resizeGL(int width, int height)
{
    qDebug() << "resizeGL, " << width << ": " << height;
}