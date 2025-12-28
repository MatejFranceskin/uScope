#include "CaptureThread.h"
#include <QDebug>

CaptureThread::CaptureThread(cv::VideoCapture* capture, QObject* parent)
    : QThread(parent)
    , _capture(capture)
    , _running(false)
{
}

void CaptureThread::stop()
{
    _running = false;
}

void CaptureThread::run()
{
    _running = true;
    qDebug() << "CaptureThread::run - starting continuous frame capture";
    
    while (_running && _capture && _capture->isOpened()) {
        cv::Mat frame;
        if (_capture->read(frame) && !frame.empty()) {
            emit frameCaptured(frame);
        } else {
            // Small delay if read failed to avoid busy loop
            msleep(10);
        }
    }
    
    qDebug() << "CaptureThread::run - stopped";
}
