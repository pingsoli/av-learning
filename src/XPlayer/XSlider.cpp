#include <QMouseEvent>
#include <QWidget>

#include "XSlider.h"

XSlider::XSlider(QWidget *parent)
    : QSlider(parent)
{
}

XSlider::~XSlider()
{
}

void XSlider::mousePressEvent(QMouseEvent *e)
{
    double ratio = (double) e->pos().x() / (double) width();
    setValue(ratio * maximum());
    
    //QSlider::mousePressEvent(e);
    QSlider::sliderReleased();
}