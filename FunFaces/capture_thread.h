#ifndef CAPTURE_THREAD_H
#define CAPTURE_THREAD_H

#include <QString>
#include <QThread>
#include <QMutex>
#include <QElapsedTimer>
#include <QApplication>
#include <vector>
#include "opencv2/opencv.hpp"
#include "opencv2/face/facemark.hpp"
#include "utilities.h"

class CaptureThread : public QThread
{
    Q_OBJECT

public:
    enum MASK_TYPE {
        RECTANGLE = 0,
        LANDMARKS,
        GLASSES,
        MUSTACHE,
        MOUSE_NOSE,
        MASK_COUNT
    };

public:
    CaptureThread(int camera, QMutex *lock);
    ~CaptureThread();
    void setRunning(bool run) { running = run; }
    void takePhoto() { taking_photo = true; }
    void updateMasksFlag(MASK_TYPE type, bool on_or_off);

protected:
    void run() override;

signals:
    void frameCaptured(cv::Mat *data);
    void photoTaken(QString name);

private:
    void takePhoto(cv::Mat &frame);
    void detectFaces(cv::Mat &frame);
    void loadOrnaments();
    void drawGlasses(cv::Mat &frame, std::vector<cv::Point2f> &marks);
    void drawMustache(cv::Mat &frame, std::vector<cv::Point2f> &marks);
    void drawMouseNose(cv::Mat &frame, std::vector<cv::Point2f> &marks);
    bool isMaskOn(MASK_TYPE type);

private:
    bool running;
    int cameraID;
    QString videoPath;
    QMutex *data_lock;
    cv::Mat frame;
    int frame_width, frame_height;
    bool taking_photo;
    cv::CascadeClassifier *classifier;
    cv::Ptr<cv::face::Facemark> mark_detector;
    cv::Mat glasses;
    cv::Mat mustache;
    cv::Mat mouse_nose;
    uint8_t masks_flag;


};

#endif // CAPTURE_THREAD_H
