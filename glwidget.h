#pragma once
#ifndef GLWIDGET_H
#define GLWIDGET_H
#include <QOpenGLWidget>
#include <QOpenGLFunctions>

class glwidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
signals:
    void mouseclick(QString);
public:
    explicit glwidget(QWidget* parent = nullptr);
    QImage image;
protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
public slots:
    void loadImage(QImage& image);
    void paintEvent(QPaintEvent*) Q_DECL_OVERRIDE;

};

#endif // GLWIDGET_H
