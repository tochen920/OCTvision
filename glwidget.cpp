#include "glwidget.h"
#include <QOpenGLWidget>
#include <QPainter>
#include <QMouseEvent>
#include <mainwindow.h>
#include <iostream>
#include <ui_mainwindow.h>
glwidget::glwidget(QWidget* parent) :
    QOpenGLWidget(parent)
{

}

void glwidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0, 0, 0, 1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);

}

void glwidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}

void glwidget::resizeGL(int w, int h)
{

}

void glwidget::mousePressEvent(QMouseEvent* event)
{
    if (event->buttons() == Qt::LeftButton) {
        QPoint p = this->mapFromGlobal(QCursor::pos());
        int px = p.x();
        int py = p.y();
        mouseclick(QString::number(px) + ", " + QString::number(py));
        std::cout << px << ", " << py << "\n";
    }
}

void glwidget::loadImage(QImage& img)
{
    image = img;
    this->update();
}

void glwidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.drawImage(this->rect(), image);
    //painter.drawLine(3,3,11,11);

}
