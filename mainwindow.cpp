#include <QtWidgets/QMainWindow>
#include "MainWindow.h";
#include <QtCore>
#include <QTimer>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <vector>
#include <QThread>
#include <QtConcurrent/QtConcurrent>
#include <QThreadPool>
#include <glwidget.h>
#include <mainwindow.h>
#include <iostream>
#include <ui_mainwindow.h>
#define camid 0
RNG rng(12345);


MainWindow::MainWindow(QWidget* parent) :
    //Setup for the GUI window
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //sets up the video source(0 for local camera, video files must be connected through pathway.)
    capwebcam.open("/Users/mathomas/Desktop/eyetf.mp4"); //Change this to your own video
    if (capwebcam.isOpened() == false) {
        ui->output->appendPlainText("error: was not able to access webcam");
        return;
    }

    //connects processframeupdate to new thread, cannot include gui objects(must be in main thread)
    QThread::currentThread()->setObjectName("mainwindow");
    QThreadPool pool;
    QFuture<void> future = QtConcurrent::run(this, &MainWindow::processframeupdate);
    //glwidget *testw = new glwidget;
    connect(ui->testw, SIGNAL(mouseclick(QString)), this, SLOT(settext(const QString&)));
    connect(ui->testw2, SIGNAL(mouseclick(QString)), this, SLOT(settext(const QString&)));



    //sets up a timer that repeats the process update function
    tTimer = new QTimer(this);
    connect(tTimer, SIGNAL(timeout()), this, SLOT(processframeupdate()));
    tTimer->start(20);

    //links future thread to main thread
    QObject::connect(this, SIGNAL(signalGUI(const QString&)), this, SLOT(settext(const QString&)));

    //sets up the starting variables for the threshold values
    thresh = 15;
    sthresh = 2;
    bthresh = 7;

    ui->threshslider->setRange(0, 256);
    ui->threshslider->setValue(thresh);

    ui->saccadethresh->setRange(0, 20);
    ui->blinkthresh->setRange(0, 20);

    ui->saccadethresh->setValue(2);
    ui->blinkthresh->setValue(7);


}

//deletes the gui window
MainWindow::~MainWindow()
{
    delete ui;
}

//the method that updates the image and contours on the image
void MainWindow::processframeupdate() {

    //checks the source to see if it is the camera or video file
    capwebcam.read(matoriginal);
    if (source != camid) {
        total = capwebcam.get(CAP_PROP_FRAME_COUNT);
        current = capwebcam.get(CAP_PROP_POS_FRAMES);
    }


    if (matoriginal.empty() == true) return;

    //resizes window, converts to greyscale and blurs the image.
    cv::resize(matoriginal, matoriginal, cv::Size(), 0.3, 0.3);
    cv::cvtColor(matoriginal, matprocess, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(matprocess, matprocess, cv::Size(9, 9), 1.5);
    //creates the thresholded image (inversed) based on variable thresh,
    cv::threshold(matprocess, matprocess, thresh, 255, THRESH_BINARY_INV);
    Canny(matprocess, matprocess, 120, 255, 3);

    //creates contours for the image and smoothes them out
    findContours(matprocess, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));
    vector<vector<Point> >hull(contours.size());
    for (size_t i = 0; i < contours.size(); i++)
    {
        convexHull(contours[i], hull[i]);
    }

    //sorts contours by size from largest to smallest
    if (contours.size() >= 1) {
        sort(contours.begin(), contours.end(), [](const vector<Point>& c1, const vector<Point>& c2) {
            return contourArea(c1, false) > contourArea(c2, false);
            });

        //finds the center of the contours
        vector<Moments> mu(contours.size());
        for (int i = 0; i < contours.size(); i++) {
            mu[i] = moments(contours[i], false);
        }
        vector<Point2f> mc(contours.size());
        for (int i = 0; i < contours.size(); i++)
        {
            mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
            //ui->output->appendPlainText(QString("distance dif: ") + QString::number(mu[i].m10/mu[i].m00));
            //ui->output->appendPlainText(QString("distance dif: ") + QString::number(mu[i].m01/mu[i].m00));

        }



        //creates ellipses for the contours
        vector<RotatedRect> minEllipse(contours.size());
        for (int i = 0; i < contours.size(); i++) {
            if (contours[i].size() > 5) {
                minEllipse[i] = fitEllipse(Mat(contours[i]));
            }
        }

        //shows contours, center and the ellipse around the pupil
        for (int i = 0; i < contours.size(); i++) {
            Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));

            //draws contours on the original image, used for more clarity
            //drawContours( matoriginal, contours, i, Scalar(0,0,255), 1, 8, vector<Vec4i>(), 0, Point() );

        }
        ellipse(matoriginal, minEllipse[0], Scalar(0, 0, 255), 2, 8);
        circle(matoriginal, mc[0], 4, Scalar(0, 0, 255), -1, 8, 0);

        //calculates difference in position and height of pupil between frames to detect saccades and blinking
        double H = minEllipse[0].size.height;
        double W = minEllipse[0].size.width;
        double Hdiff;
        if (H == 0 || W == 0) {
            Hdiff = bthresh + 1;
        }
        else {
            Hdiff = pow(H - oldH, 2.0) + pow(W - oldW, 2.0);
        }
        //signalGUI(QString::number(H) + ", "+ QString::number(W));
        //signalGUI(QString::number(Hdiff));
        if (Hdiff >= bthresh) {
            blinklist.push_back(current);
            signalGUI("blink" + QString::number(H));
        }
        oldH = H;
        oldW = W;

        double xval = mu[0].m10 / mu[0].m00;
        double yval = mu[0].m01 / mu[0].m00;
        double distdif = pow((xval - oldx), 2.0) + pow((yval - oldy), 2.0);
        distdif = pow(distdif, 0.5);

        //signalGUI(QString("saccade ")+QString::number(distdif));
        if (distdif >= sthresh && Hdiff < bthresh) {
            saccadelist.push_back(current);
            signalGUI(QString("saccade ") + QString::number(distdif));
        }
        oldx = xval;
        oldy = yval;


        //converts the image from one color space to another.
        cv::cvtColor(matoriginal, matoriginal, cv::COLOR_BGR2RGB);

        //processes the frame into an image
        QImage qimqOriginal((uchar*)matoriginal.data, matoriginal.cols, matoriginal.rows, matoriginal.step, QImage::Format_RGB888);
        QImage qimqProcessed((uchar*)matprocess.data, matprocess.cols, matprocess.rows, matprocess.step, QImage::Format_Indexed8);

        //presents the image
        if (current <= total - 10 || source == camid) {
            ui->testw->loadImage(qimqOriginal);
            ui->testw2->loadImage(qimqProcessed);


        }
        else {
            capwebcam.release();
            tTimer->stop();
        }
    }



}

//start/stop button on the ui
void MainWindow::on_Start_clicked()
{
    if (tTimer->isActive() == true) {
        tTimer->stop();
        ui->Start->setText("Start");
    }
    else {
        if (capwebcam.isOpened() == false) {
            capwebcam.open(0);
        }
        tTimer->start(20);
        ui->Start->setText("Stop");
    }
}

void MainWindow::settext(const QString& string) {
    ui->output->appendPlainText(string);
}

//changes the threshold value for greyscale, saccades and blinking
void MainWindow::on_threshslider_valueChanged(int value)
{
    thresh = value;
}


void MainWindow::on_saccadethresh_valueChanged(int value)
{
    sthresh = value;
}

void MainWindow::on_blinkthresh_valueChanged(int value)
{
    bthresh = value;
}
