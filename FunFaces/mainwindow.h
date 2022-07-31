#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QCheckBox>
#include <QPushButton>
#include <QListView>
#include <QStatusBar>
#include <QLabel>
#include <QGridLayout>
#include <QPixmap>
#include <QMediaDevices>
#include <QCameraDevice>
#include <QMessageBox>
#include <QMutex>
#include <QStandardItemModel>
#include "opencv2/opencv.hpp"
#include "capture_thread.h"
#include "utilities.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    QMenu *fileMenu;
    QMenu *helpMenu;

    QAction *cameraInfoAction;
    QAction *openCameraAction;
    QAction *exitAction;
    QAction *aboutAction;

    QGraphicsScene *imageScene;
    QGraphicsView *imageView;

    QPushButton *shutterButton;

    QListView *saved_list;

    QStatusBar *mainStatusBar;
    QLabel *mainStatusLabel;

    cv::Mat currentFrame;

    // for capture thread
    QMutex *data_lock;
    CaptureThread *capturer;

    QStandardItemModel *list_model;

private:
    void initUI();
    void createActions();
    void populateSavedList();
    void appendSavedPhoto(QString name);

private slots:
    void showCameraInfo();
    void openCamera();
    void updateFrame(cv::Mat *mat);
    void showAbout();
};
#endif // MAINWINDOW_H
