#pragma once

#include <QSlider>

class QWidget;
class QMouseEvent;

class XSlider : public QSlider
{
    Q_OBJECT

public:
    XSlider(QWidget *parent = nullptr);
    ~XSlider();

    // override mousePressEvent()
    // mouse click specific place and progress bar will go there
    void mousePressEvent(QMouseEvent *e) override;
};