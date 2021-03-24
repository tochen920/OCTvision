#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QThread>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <glwidget.h>
using namespace std;
using namespace cv;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();



public slots:
    void processframeupdate();

    void settext(const QString &string);
private slots:
    void on_Start_clicked();

    void on_threshslider_valueChanged(int value);

    void on_saccadethresh_valueChanged(int value);

    void on_blinkthresh_valueChanged(int value);

signals:
   //this signal is used to tell main thread to change text
   void signalGUI(QString);

private:
    Ui::MainWindow *ui;
    cv::VideoCapture capwebcam;
    cv::Mat matoriginal;
    cv::Mat matprocess;
    QImage imgOriginal;
    QImage imgProcess;
    std::vector<cv::Vec3f> veccircles;
    std::vector<cv::Vec3f>::iterator itcircles;
    QTimer *tTimer;
    std::vector<std::vector<cv::Point> > contours;
    vector<vector<Point>>newcontours;
    vector<Vec4i> hierarchy;
    QString sourcetext;
    int thresh;
    int sthresh;
    int bthresh;
    double oldH;
    double oldW;
    int oldx = 0;
    int oldy = 0;
    int current = 0;
    int total = 0;
    QString source;
    vector<double> saccadelist;
    vector<double> blinklist;
    QThread* thread;
    glwidget *original;
    glwidget *processed;

};

#endif // MAINWINDOW_H
