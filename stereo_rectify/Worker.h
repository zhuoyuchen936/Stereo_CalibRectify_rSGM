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

class StereoCalibrationWorker : public QObject {
    Q_OBJECT

public:
    // 添加构造函数、析构函数和所需的方法
    StereoCalibrationWorker() {}

signals:
    void updateText(const QString& text);
    void param_fresh();
    void workFinished();
    void updateProgress(float progress);
    void updateImage(cv::Mat& images1, cv::Mat& images2);

public slots:
    void doWork();
private:
    void processChessboardImages(std::vector<std::string>& images1_path,
        std::vector<std::string>& images2_path,
        const cv::Size& patternSize,
        float square_size,
        std::vector<std::vector<cv::Point3f>>& objectPoints,
        std::vector<std::vector<cv::Point2f>>& imagePoints1,
        std::vector<std::vector<cv::Point2f>>& imagePoints2);

    void calibrateCameras(
        const std::string& projpath,
        std::vector<std::vector<cv::Point3f>>& objectPoints,
        std::vector<std::vector<cv::Point2f>>& imagePoints1,
        std::vector<std::vector<cv::Point2f>>& imagePoints2,
        cv::Size imageSize,
        cv::Mat& cameraMatrix1,
        cv::Mat& distCoeffs1,
        std::vector<cv::Mat>& rvecs1,
        std::vector<cv::Mat>& tvecs1,
        cv::Mat& cameraMatrix2,
        cv::Mat& distCoeffs2,
        std::vector<cv::Mat>& rvecs2,
        std::vector<cv::Mat>& tvecs2);

    void stereoCalibration(
        const std::vector<std::vector<cv::Point3f>>& objectPoints,
        const std::vector<std::vector<cv::Point2f>>& imagePoints1,
        const std::vector<std::vector<cv::Point2f>>& imagePoints2,
        cv::Mat& cameraMatrix1, cv::Mat& distCoeffs1,
        cv::Mat& cameraMatrix2, cv::Mat& distCoeffs2,
        const cv::Size& imageSize, cv::Mat& R, cv::Mat& T, cv::Mat& E, cv::Mat& F);
};
