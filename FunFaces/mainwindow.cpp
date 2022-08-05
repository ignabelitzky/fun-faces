#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    capturer = nullptr;
    data_lock = new QMutex();
    initUI();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initUI()
{
    this->resize(1000, 800);
    // setup menubar
    fileMenu = menuBar()->addMenu("&File");
    helpMenu = menuBar()->addMenu("&Help");

    QGridLayout *main_layout = new QGridLayout();
    imageScene = new QGraphicsScene(this);
    imageView = new QGraphicsView(imageScene);
    main_layout->addWidget(imageView, 0, 0, 12, 1);

    QGridLayout *tools_layout = new QGridLayout();
    main_layout->addLayout(tools_layout, 12, 0, 1, 1);

    shutterButton = new QPushButton(this);
    shutterButton->setText("Take a Photo");
    tools_layout->addWidget(shutterButton, 0, 0, Qt::AlignHCenter);

    connect(shutterButton, SIGNAL(clicked(bool)),this, SLOT(takePhoto()));

    // masks
    QGridLayout *masks_layout = new QGridLayout();
    main_layout->addLayout(masks_layout, 13, 0, 1, 1);
    masks_layout->addWidget(new QLabel("Select Masks:", this));
    for(int i = 0; i < CaptureThread::MASK_COUNT; i++) {
        mask_checkboxes[i] = new QCheckBox(this);
        masks_layout->addWidget(mask_checkboxes[i], 0, i + 1);
        connect(mask_checkboxes[i], SIGNAL(stateChanged(int)), this, SLOT(updateMasks(int)));
    }
    mask_checkboxes[0]->setText("Rectangle");
    mask_checkboxes[1]->setText("Landmarks");
    mask_checkboxes[2]->setText("Glasses");
    mask_checkboxes[3]->setText("Mustache");
    mask_checkboxes[4]->setText("Mouse Nose");

    for(int i = 0; i < CaptureThread::MASK_COUNT; i++) {
        mask_checkboxes[i]->setCheckState(Qt::Unchecked);
    }

    // list of saved videos
    saved_list = new QListView(this);
    main_layout->addWidget(saved_list, 14, 0, 4, 1);

    QWidget *widget = new QWidget();
    widget->setLayout(main_layout);
    setCentralWidget(widget);

    // setup status bar
    mainStatusBar = statusBar();
    mainStatusLabel = new QLabel(mainStatusBar);
    mainStatusBar->addPermanentWidget(mainStatusLabel);
    mainStatusLabel->setText("Fun Faces is ready");


    // list of saved videos
    saved_list = new QListView(this);
    saved_list->setViewMode(QListView::IconMode);
    saved_list->setResizeMode(QListView::Adjust);
    saved_list->setSpacing(5);
    saved_list->setWrapping(false);
    list_model = new QStandardItemModel(this);
    saved_list->setModel(list_model);
    main_layout->addWidget(saved_list, 14, 0, 4, 1);

    createActions();
    populateSavedList();
}

void MainWindow::createActions()
{
    cameraInfoAction = new QAction("Camera &Info", this);
    openCameraAction = new QAction("Open Camera", this);
    exitAction = new QAction("E&xit");
    aboutAction = new QAction("&About", this);

    fileMenu->addAction(cameraInfoAction);
    fileMenu->addAction(openCameraAction);
    fileMenu->addAction(exitAction);

    helpMenu->addAction(aboutAction);

    connect(exitAction, SIGNAL(triggered(bool)), QApplication::instance(), SLOT(quit()));
    connect(cameraInfoAction, SIGNAL(triggered(bool)), this, SLOT(showCameraInfo()));
    connect(openCameraAction, SIGNAL(triggered(bool)), this, SLOT(openCamera()));
    connect(aboutAction, SIGNAL(triggered(bool)), this, SLOT(showAbout()));
}

void MainWindow::populateSavedList()
{
    QDir dir(Utilities::getDataPath());
    QStringList nameFilters;
    nameFilters << "*.jpg";
    QFileInfoList files = dir.entryInfoList(nameFilters,
                                            QDir::NoDotAndDotDot | QDir::Files,
                                            QDir::Name);
    foreach(QFileInfo cover, files) {
        QString name = cover.baseName();
        QStandardItem *item = new QStandardItem();
        list_model->appendRow(item);
        QModelIndex index = list_model->indexFromItem(item);
        list_model->setData(index, QPixmap(cover.absoluteFilePath()).scaledToHeight(145),
                            Qt::DecorationRole);
        list_model->setData(index, name, Qt::DisplayRole);
    }
}

void MainWindow::appendSavedPhoto(QString name)
{
    QString photo_path = Utilities::getPhotoPath(name, "jpg");
    QStandardItem *item = new QStandardItem();
    list_model->appendRow(item);
    QModelIndex index = list_model->indexFromItem(item);
    list_model->setData(index, QPixmap(photo_path).scaledToHeight(145),
                        Qt::DecorationRole);
    list_model->setData(index, name, Qt::DisplayRole);
    saved_list->scrollTo(index);
}

void MainWindow::showCameraInfo()
{
    QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    QString info = QString("Available Cameras: \n");

    foreach(const QCameraDevice &cameraInfo, cameras) {
        info += " - " + cameraInfo.id() + ": ";
        info += cameraInfo.description() + "\n";
    }
    QMessageBox::information(this, "Cameras", info);
}

void MainWindow::openCamera()
{
    if(capturer != nullptr) {
        // if a thread is already runngin, stop it
        capturer->setRunning(false);
        disconnect(capturer, &CaptureThread::frameCaptured, this, &MainWindow::updateFrame);
        disconnect(capturer, &CaptureThread::photoTaken, this, &MainWindow::appendSavedPhoto);
        connect(capturer, &CaptureThread::finished, capturer, &CaptureThread::deleteLater);
    }
    int camID = 0;
    capturer = new CaptureThread(camID, data_lock);
    connect(capturer, &CaptureThread::frameCaptured, this, &MainWindow::updateFrame);
    connect(capturer, &CaptureThread::photoTaken, this, &MainWindow::appendSavedPhoto);
    capturer->start();
    mainStatusLabel->setText(QString("Capturing camera %1").arg(camID));
}

void MainWindow::updateFrame(cv::Mat *mat)
{
    data_lock->lock();
    currentFrame = *mat;
    data_lock->unlock();

    QImage frame(currentFrame.data, currentFrame.cols, currentFrame.rows, currentFrame.step, QImage::Format_RGB888);
    QPixmap image = QPixmap::fromImage(frame);

    imageScene->clear();
    imageView->resetTransform();
    imageScene->addPixmap(image);
    imageScene->update();
    imageView->setSceneRect(image.rect());
}

void MainWindow::showAbout()
{
    QString text = "Desktop application developed by\n";
    text += "Ignacio Belitzky\n\n";
    text += "License: GNU General Public License v3";
    QMessageBox::information(this, "About", text);
}

void MainWindow::takePhoto()
{
if(capturer != nullptr) {
    capturer->takePhoto();
}
}

void MainWindow::updateMasks(int status)
{
    if(capturer == nullptr) {
        return;
    }
    QCheckBox *box = qobject_cast<QCheckBox*>(sender());
    for(int i = 0; i < CaptureThread::MASK_COUNT; i++) {
        if(mask_checkboxes[i] == box) {
            capturer->updateMasksFlag(static_cast<CaptureThread::MASK_TYPE> (i), status != 0);
        }
    }
}
