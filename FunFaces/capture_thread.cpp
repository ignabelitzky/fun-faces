#include "capture_thread.h"

CaptureThread::CaptureThread(int camera, QMutex *lock):
    running(false), cameraID(camera), videoPath(""), data_lock(lock)
{
    frame_width = frame_height = 0;
    taking_photo = false;
    loadOrnaments();
    masks_flag = 0;
}

CaptureThread::~CaptureThread()
{

}

void CaptureThread::updateMasksFlag(MASK_TYPE type, bool on_or_off)
{
    uint8_t bit = 1 << type;
    if(on_or_off) {
        masks_flag |= bit;
    } else {
        masks_flag &= ~bit;
    }
}

void CaptureThread::run()
{
    running = true;
    cv::VideoCapture cap(cameraID);
    cv::Mat tmp_frame;
    frame_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    frame_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    classifier = new cv::CascadeClassifier(OPENCV_DATA_DIR "haarcascades/haarcascade_frontalface_default.xml");
    mark_detector = cv::face::createFacemarkLBF();
    QString model_data = "../../FunFaces/data/lbfmodel.yaml";
    qDebug() << model_data;
    mark_detector->loadModel(model_data.toStdString());
    while(running) {
        cap >> tmp_frame;
        if(tmp_frame.empty())
            break;
        if(masks_flag > 0)
            detectFaces(tmp_frame);
        if(taking_photo) {
            takePhoto(tmp_frame);
        }
        cvtColor(tmp_frame, tmp_frame, cv::COLOR_BGR2RGB);
        data_lock->lock();
        frame = tmp_frame;
        data_lock->unlock();
        emit frameCaptured(&frame);
    }
    cap.release();
    delete classifier;
    classifier = nullptr;
    running = false;
}

void CaptureThread::takePhoto(cv::Mat &frame)
{
    QString photo_name = Utilities::newPhotoName();
    QString photo_path = Utilities::getPhotoPath(photo_name, "jpg");
    cv::imwrite(photo_path.toStdString(), frame);
    emit photoTaken(photo_name);
    taking_photo = false;
}

void CaptureThread::detectFaces(cv::Mat &frame)
{
    std::vector<cv::Rect> faces;
    cv::Mat gray_frame;
    cv::cvtColor(frame, gray_frame, cv::COLOR_BGR2GRAY);
    classifier->detectMultiScale(gray_frame, faces, 1.3, 5);
    cv::Scalar color = cv::Scalar(0, 0, 255);   // red
    if(isMaskOn(RECTANGLE)) {
        for(size_t i = 0; i < faces.size(); i++) {
            cv::rectangle(frame, faces[i], color, 1);
        }
    }
    std::vector<std::vector<cv::Point2f>> shapes;
    if(mark_detector->fit(frame, faces, shapes)) {
        // draw facial land marks
        for(unsigned long i = 0u; i < faces.size(); i++) {
            if (isMaskOn(LANDMARKS)) {
                for(unsigned long k = 0u; k < shapes[i].size(); k++) {
                    cv::circle(frame, shapes[i][k], 2, color, cv::FILLED);
                    // QString index = QString("%1").arg(k);
                    //cv::putText(frame, index.toStdString(), shapes[i][k], cv::FONT_HERSHEY_SIMPLEX, 0.4, color, 2);
                }
            }
            if(isMaskOn(GLASSES))
                drawGlasses(frame, shapes[i]);
            if(isMaskOn(MUSTACHE))
                drawMustache(frame, shapes[i]);
            if(isMaskOn(MOUSE_NOSE))
                drawMouseNose(frame, shapes[i]);
        }
    }
}

void CaptureThread::loadOrnaments()
{
    QImage image;
    image.load(":/images/glasses.jpg");
    image = image.convertToFormat(QImage::Format_RGB888);
    glasses = cv::Mat(
                image.height(), image.width(), CV_8UC3,
                image.bits(), image.bytesPerLine()).clone();
    image.load(":/images/mustache.jpg");
    image = image.convertToFormat(QImage::Format_RGB888);
    mustache = cv::Mat(
                image.height(), image.width(), CV_8UC3,
                image.bits(), image.bytesPerLine()).clone();
    image.load(":/images/mouse-nose.jpg");
    image = image.convertToFormat(QImage::Format_RGB888);
    mouse_nose = cv::Mat(
                image.height(), image.width(), CV_8UC3,
                image.bits(), image.bytesPerLine()).clone();
}

void CaptureThread::drawGlasses(cv::Mat &frame, std::vector<cv::Point2f> &marks)
{
    // resize
    cv::Mat ornament;
    double distance = cv::norm(marks[45] - marks[36]) * 1.5;
    cv::resize(glasses, ornament, cv::Size(0, 0), distance / glasses.cols,
               distance / glasses.cols, cv::INTER_NEAREST);

    // rotate
    double angle = -atan((marks[45].y - marks[36].y) / (marks[45].x - marks[36].x));
    cv::Point2f center = cv::Point(ornament.cols/2, ornament.rows/2);
    cv::Mat rotateMatrix = cv::getRotationMatrix2D(center, angle * 180 / 3.14, 1.0);

    cv::Mat rotated;
    cv::warpAffine(ornament, rotated, rotateMatrix, ornament.size(),
                   cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255));

    // paint
    center = cv::Point((marks[45].x + marks[36].x) / 2, (marks[45].y + marks[36].y) / 2);
    cv::Rect rec(center.x - rotated.cols / 2, center.y - rotated.rows / 2, rotated.cols, rotated.rows);
    frame(rec) &= rotated;
}

void CaptureThread::drawMustache(cv::Mat &frame, std::vector<cv::Point2f> &marks)
{
    // resize
    cv::Mat ornament;
    double distance = cv::norm(marks[54] - marks[48]) * 1.5;
    cv::resize(mustache, ornament, cv::Size(0, 0), distance / mustache.cols,
               distance / mustache.cols, cv::INTER_NEAREST);

    // rotate
    double angle = -atan((marks[54].y - marks[48].y) / (marks[54].x - marks[48].x));
    cv::Point2f center = cv::Point(ornament.cols/2, ornament.rows/2);
    cv::Mat rotateMatrix = cv::getRotationMatrix2D(center, angle * 180 / 3.14, 1.0);

    cv::Mat rotated;
    cv::warpAffine(ornament, rotated, rotateMatrix, ornament.size(),
                   cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255));

    // paint
    center = cv::Point((marks[51].x + marks[33].x) / 2, (marks[51].y + marks[33].y) / 2);
    cv::Rect rec(center.x - rotated.cols / 2, center.y - rotated.rows / 2, rotated.cols, rotated.rows);
    frame(rec) &= rotated;
}

void CaptureThread::drawMouseNose(cv::Mat &frame, std::vector<cv::Point2f> &marks)
{
    // resize
    cv::Mat ornament;
    double distance = cv::norm(marks[13] - marks[3]);
    cv::resize(mouse_nose, ornament, cv::Size(0, 0), distance / mouse_nose.cols,
               distance / mouse_nose.cols, cv::INTER_NEAREST);

    // rotate
    double angle = -atan((marks[16].y - marks[0].y) / (marks[16].x - marks[0].x));
    cv::Point2f center = cv::Point(ornament.cols/2, ornament.rows/2);
    cv::Mat rotateMatrix = cv::getRotationMatrix2D(center, angle * 180 / 3.14, 1.0);

    cv::Mat rotated;
    cv::warpAffine(ornament, rotated, rotateMatrix, ornament.size(),
                   cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255));

    // paint
    center = marks[30];
    cv::Rect rec(center.x - rotated.cols / 2, center.y - rotated.rows / 2, rotated.cols, rotated.rows);
    frame(rec) &= rotated;
}

bool CaptureThread::isMaskOn(MASK_TYPE type)
{
    return (masks_flag & (1 << type)) != 0;
}


