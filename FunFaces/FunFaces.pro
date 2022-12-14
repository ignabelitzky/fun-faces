QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    capture_thread.cpp \
    main.cpp \
    mainwindow.cpp \
    utilities.cpp

HEADERS += \
    capture_thread.h \
    mainwindow.h \
    utilities.h

unix: !mac {
    INCLUDEPATH += /usr/include/opencv4
    LIBS += -L/usr/include/opencv/lib -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_video -lopencv_videoio -lopencv_objdetect -lopencv_face
}

DEFINES += OPENCV_DATA_DIR=\\\"/usr/share/opencv4/\\\"

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    images.qrc
