#ifndef CAPTURETHREAD_H
#define CAPTURETHREAD_H

#include <QThread>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

/**
 * Worker thread for continuous camera frame capture
 * Reads frames from OpenCV VideoCapture as fast as they become available
 */
class CaptureThread : public QThread
{
    Q_OBJECT

public:
    explicit CaptureThread(cv::VideoCapture* capture, QObject* parent = nullptr);
    void stop();

signals:
    void frameCaptured(const cv::Mat& frame);

protected:
    void run() override;

private:
    cv::VideoCapture* _capture;
    volatile bool _running;
};

#endif // CAPTURETHREAD_H
