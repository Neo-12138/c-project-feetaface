#ifndef FACERECOGNIZE_H
#define FACERECOGNIZE_H

#include <QMainWindow>
#include <opencv.hpp>
#include <QTcpSocket>
#include <QTimer>
#include <QDebug>




using namespace cv;
using namespace std;
QT_BEGIN_NAMESPACE
namespace Ui { class FaceRecognize; }
QT_END_NAMESPACE

class FaceRecognize : public QMainWindow
{
    Q_OBJECT

public:
    FaceRecognize(QWidget *parent = nullptr);
    ~FaceRecognize();

    void timerEvent(QTimerEvent *event);

private slots:
    //定时连接服务器
    void timer_connect();
    void stop_connect();
    void start_connect();

    void read_data();


private:
    Ui::FaceRecognize *ui;

    VideoCapture cap;//摄像头

    // haar --级联分类器
    cv::CascadeClassifier cascade;

    QTcpSocket mysocket;
    QTimer mytimer;

    //标志是否同一个人脸进入识别区域
    int flag;

    //保存人脸的数据
    cv::Mat faceMat;
};
#endif // FACERECOGNIZE_H
