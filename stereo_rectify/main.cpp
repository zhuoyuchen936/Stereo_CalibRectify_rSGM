#include "stereo_rectify.h"
#include <QtWidgets/QApplication>
#include <stdio.h>
#include <iostream>
using namespace cv;
int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    utils::logging::setLogLevel(utils::logging::LOG_LEVEL_ERROR);//Set the log level of OpenCV to error
    QApplication a(argc, argv);
    std::cout << "start." << std::endl;
    stereo_rectify w;
    w.show();
    return a.exec();
}
