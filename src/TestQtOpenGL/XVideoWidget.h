#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QGLShaderProgram>

class XVideoWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    XVideoWidget(QWidget *parent);
    ~XVideoWidget();

protected:
    void paintGL() override;
    void initializeGL() override;
    void resizeGL(int width, int height) override;

private:
    QGLShaderProgram program; // shader program.
    
    // shader yuv address
    GLuint unis[3] = { 0 };

    // opengl texture address
    GLuint texs[3] = { 0 };

    // texture space
    uint8_t *datas[3] = { 0 };

    // pixel resolution
    int width = 420;
    int height = 128;
};
