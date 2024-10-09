#include "FaceRecognize.h"

#include <QApplication>




int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FaceRecognize w;
    w.show();
    return a.exec();
}
