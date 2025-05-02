#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_stereo_rectify.h"
#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <filesystem>
// #include <Windows.h>
#include <QImage>
#include <QPixmap>
#include <QThread>
#include <QPainter>
#include <QLine>

#include "Worker.h"
#include "stereo_sgm.h"

class stereo_rectify : public QMainWindow
{
    Q_OBJECT

public:
    stereo_rectify(QWidget *parent = nullptr);

    ~stereo_rectify();

private:
    Ui::stereo_rectifyClass ui;
    StereoCalibrationWorker* worker;
    QThread *workerThread;

private slots:
    void on_LeftImage_clicked();
    void on_Calib_clicked();
    void on_Import_clicked();
    void on_TestImage_clicked();
    void on_Rectify_clicked();
    void on_showRectify_cliked();
    void on_Crop_cliked();
    void on_computeRange_cliked();
    void on_disLine_cliked();
    void on_ConfirmChess_cliked();
    
    void param_fresh();
    void param2_fresh();
    void updateProgress(float progress);
    void updateTextBrowser(const QString& text);
    void updateImage(cv::Mat images1,cv::Mat images2);
    void on_stereo_cliked();
    void on_next_img_cliked();
    void on_test1_cliked();
    cv::Mat reserveROI(cv::Mat disp_sgm, int center_width, int center_height);
    

    //std::vector<cv::Mat> stereo_rectify::loadImagesFromDirectory(const std::string& directoryPath);
};


