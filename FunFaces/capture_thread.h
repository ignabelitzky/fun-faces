#ifndef CAPTURE_THREAD_H
#define CAPTURE_THREAD_H

#include <QString>
#include <QThread>
#include <QMutex>
#include <QElapsedTimer>
#include <vector>
#include "opencv2/opencv.hpp"
#include "utilities.h"

class CaptureThread : public QThread
{
    Q_OBJECT
public:
    CaptureThread(int camera, QMutex *lock);
    ~CaptureThread();
    void setRunning(bool run) { running = run; }
    void takePhoto() { taking_photo = true; }

protected:
    void run() override;

signals:
    void frameCaptured(cv::Mat *data);
    void photoTaken(QString name);

private:
    void takePhoto(cv::Mat &frame);
    void detectFaces(cv::Mat &frame);

private:
    bool running;
    int cameraID;
    QString videoPath;
    QMutex *data_lock;
    cv::Mat frame;
    int frame_width, frame_height;
    bool taking_photo;
    cv::CascadeClassifier *classifier;


};

#endif // CAPTURE_THREAD_H
