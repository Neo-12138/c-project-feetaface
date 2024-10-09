#include "FaceRecognize.h"
#include "ui_FaceRecognize.h"


#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>



FaceRecognize::FaceRecognize(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FaceRecognize)
{
    ui->setupUi(this);
    //打开摄像头
    cap.open(0);
    //启动定时器时间
    startTimer(100);

    //导入级联分类器文件
    cascade.load("C:/opencv-4.5.2/opencv-SeetaFace-qt5.14.0minGW/opencv452/etc/haarcascades/haarcascade_frontalface_alt2.xml");

    connect(&mysocket,&QTcpSocket::disconnected,this,&FaceRecognize::start_connect);
    connect(&mysocket,&QTcpSocket::connected,this,&FaceRecognize::stop_connect);


    //定时连接服务器
    connect(&mytimer,&QTimer::timeout,this,&FaceRecognize::timer_connect);
    mytimer.start(5000);

    //绑定readyread信号
    connect(&mysocket,&QTcpSocket::readyRead,this,&FaceRecognize::read_data);

    flag = 0;

    ui->widget_LB->hide();
}

FaceRecognize::~FaceRecognize()
{
    delete ui;
}

void FaceRecognize::timerEvent(QTimerEvent *)
{
    //采集数据
    Mat srcImage;
    if (cap.grab())
    {
        cap.read(srcImage);//读取一帧数据
    }

    //转灰度图，检测更快
    Mat grayimage;
    cvtColor(srcImage,grayimage,COLOR_BGR2GRAY);
    //检测人脸数据
    std::vector<Rect> faceRects;
    cascade.detectMultiScale(grayimage,faceRects);//检测
    if(faceRects.size() > 0 && flag >= 0)
    {

        Rect rect = faceRects.at(0);//第一个人脸的矩形框
        rectangle(srcImage,rect,Scalar(0,0,255));//把框画出来

        //此处可以设置一个图片框代替矩形框跟踪人脸（没素材）

        if (flag > 2)
        {
            //把Mat数据转化为qbytearry 发送 imencode编码  imdecode 解码
            std::vector<uchar> buf;

            cv::imencode(".jpg",srcImage,buf);

            QByteArray byte((const char*)buf.data(),buf.size());
            //准备发送
            quint64 backsize = byte.size();

            QByteArray senddata;
            QDataStream stream(&senddata,QIODevice::WriteOnly);
            stream.setVersion(QDataStream::Qt_5_14);//设置流的版本

            stream<<backsize<<byte;
            mysocket.write(senddata);
            flag = -2;

            //发送完人脸数据 保存图片
            faceMat = srcImage(rect);//矩形框中的数据保存起来
            imwrite("./face.jpg",faceMat);
        }
        flag++;
    }
    if (faceRects.size() == 0)
    {
        //没检测到人脸的时候把人脸框传送到中心位置
        flag = 0;
        ui->widget_LB->hide();
    }

    if (srcImage.data == nullptr)
    {
        return;
    }
    //把opencv里的mat BGR数据转QT里的qimage RGB
    cvtColor(srcImage,srcImage, COLOR_BGR2RGB);
    QImage image(srcImage.data,srcImage.cols,srcImage.rows,srcImage.step1(),QImage::Format_RGB888);
    QPixmap mmp = QPixmap::fromImage(image);
    ui->label->setPixmap(mmp);
}

void FaceRecognize::timer_connect()
{
    //连接服务器
    mysocket.connectToHost("192.168.11.220",50000);//ip 端口
    qDebug()<<"正在连接服务器...";
    // 检查连接状态
    if (mysocket.waitForConnected(3000))
    {
        qDebug() << "连接服务器成功!";
    }
    else
    {
        qDebug() << "连接服务器失败: " << mysocket.errorString();
    }
}

void FaceRecognize::stop_connect()
{
    mytimer.stop();
    qDebug()<<"成功连接服务器";
}

void FaceRecognize::start_connect()
{
    mytimer.start(5000);
    qDebug()<<"服务器断开连接";
}

void FaceRecognize::read_data()
{
    QByteArray recv_data =  mysocket.readAll();
    qDebug()<<recv_data<<endl;

    QJsonParseError err;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(recv_data,&err); // 解析 JSON 数据
    if (err.error != QJsonParseError::NoError)
    {
        qDebug() << "json数据错误";
        return;
    }

    QJsonObject jsonObj = jsonDoc.object(); // 获取 JSON 对象

    qDebug() << "ID:" << jsonObj["id"].toString();
    qDebug() << "Name:" << jsonObj["name"].toString();
    qDebug() << "Work:" << jsonObj["work"].toString();
    qDebug() << "Time:" << jsonObj["time"].toString();

    ui->lineEdit_workerid->setText(jsonObj["id"].toString());
    ui->lineEdit_name->setText(jsonObj["name"].toString());
    ui->lineEdit_work->setText(jsonObj["work"].toString());
    ui->lineEdit_time->setText(jsonObj["time"].toString());

    if(jsonObj["id"].toString() != "-1")
    {
        ui->widget_LB->show();
    }


//    cvtColor(faceMat,faceMat, COLOR_BGR2RGB);
//    QImage image(faceMat.data,faceMat.cols,faceMat.rows,faceMat.step1(),QImage::Format_RGB888);
//    QPixmap mmp = QPixmap::fromImage(image);
//    ui->headLB_2->setPixmap(mmp);
    //通过样式来显示图片
    QString style = "border-radius:65px;border-image: url(./face.jpg);";
    ui->headLB_2->setStyleSheet(style);
}

