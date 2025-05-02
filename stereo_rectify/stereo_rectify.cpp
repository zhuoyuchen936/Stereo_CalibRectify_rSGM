// #include <pcl/io/pcd_io.h>
// #include <pcl/point_types.h>
// #include <pcl/kdtree/kdtree_flann.h>
// #include <pcl/surface/mls.h>
// #include <pcl/visualization/cloud_viewer.h>
// #include <pcl/visualization/pcl_visualizer.h>
// #include <pcl/search/kdtree.h>
// #include <Eigen/Dense>

#include "stereo_rectify.h"
#include "stereo_sgm.h"
#include <stdio.h>
#include <cmath>
#include <bitset>
#include <iostream>
#include <QFileDialog>


//#include <pcl/io/pcd_io.h>
//#include <pcl/point_types.h>
//#include <pcl/kdtree/kdtree_flann.h>
//#include <pcl/surface/mls.h>
////#include <pcl/visualization/cloud_viewer.h>
//#include <pcl/search/kdtree.h>


using namespace std;
using namespace cv;
namespace fs = std::filesystem;
bool is_rec;
bool is_remap;
bool shouldDrawLine;
bool is_workfinish;
int is_Crop;
int img_num;
bool is_Line;
int square_size;//Size_of_checkerboard_square mm

double focal_length;
double baseline;
double cx;
double cy;
std::ostringstream ss;
std::string message;
std::ostringstream ss2;
std::string message2;
std::ostringstream ss3;
std::string message3;

std::string calibpath_L;
std::string calibpath_R;
std::string projpath;
std::string projpath_test;
std::vector<string> images1_path;
std::vector<string> images2_path;
cv::Mat images1;
cv::Mat images2;
cv::Mat images1_rec;
cv::Mat images2_rec;
cv::Mat images_test1;
cv::Mat images_test2;
cv::Mat images_show1;
cv::Mat images_show2;
cv::Mat images1_chess , images2_chess;
std::vector<std::string> imagePaths1;
std::vector<std::string> imagePaths2;

cv::Size patternSize; // 假设的棋盘格尺寸，例如9x6
std::vector<std::vector<cv::Point3f>> objectPoints_LC; // 3D世界坐标
std::vector<std::vector<cv::Point2f>> imagePoints1_LC, imagePoints2_LC; // 图像坐标
cv::Size imageSize;
cv::Mat cameraMatrix1_LC, distCoeffs1_LC, cameraMatrix2_LC, distCoeffs2_LC;
std::vector <cv::Mat> rvecs1_LC, tvecs1_LC, rvecs2_LC, tvecs2_LC;
cv::Mat R_LC, T_LC, E_LC, F_LC, R1_LC, R2_LC, P1_LC, P2_LC, Q_LC;
cv::Mat map11_LC, map12_LC, map21_LC, map22_LC;

stereo_rectify::stereo_rectify(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    connect(ui.Calib_Image, &QPushButton::clicked, this, &stereo_rectify::on_LeftImage_clicked);
    connect(ui.calibration, &QPushButton::clicked, this, &stereo_rectify::on_Calib_clicked);
    connect(ui.import_paramter, &QPushButton::clicked, this, &stereo_rectify::on_Import_clicked);
    connect(ui.Test_Image, &QPushButton::clicked, this, &stereo_rectify::on_TestImage_clicked);
    connect(ui.rectification, &QPushButton::clicked, this, &stereo_rectify::on_Rectify_clicked);
    connect(ui.show_rectify, &QPushButton::clicked, this, &stereo_rectify::on_showRectify_cliked);
    connect(ui.Crop, &QPushButton::clicked, this, &stereo_rectify::on_Crop_cliked);
    connect(ui.computeRange, &QPushButton::clicked, this, &stereo_rectify::on_computeRange_cliked);
    connect(ui.disLine, &QPushButton::clicked, this, &stereo_rectify::on_disLine_cliked);
    connect(ui.Confirm_chess, &QPushButton::clicked, this, &stereo_rectify::on_ConfirmChess_cliked);
    connect(ui.stereo, &QPushButton::clicked, this, &stereo_rectify::on_stereo_cliked);
    connect(ui.next_img, &QPushButton::clicked, this, &stereo_rectify::on_next_img_cliked);
    connect(ui.test1, &QPushButton::clicked, this, &stereo_rectify::on_test1_cliked);
    
    
    ui.progressBar->setValue(0);
    is_rec = 1;
    is_remap = 0;
    shouldDrawLine = 0;
    is_workfinish = 0;
    is_Crop = 0;
    is_Line = 0;
    img_num = 0;
    patternSize.height = ui.pSizeH->text().toInt();
    patternSize.width = ui.pSizeW->text().toInt();
    square_size = ui.sqSize->text().toInt();
    cout << "patternSize.height =  " << patternSize.height << std::endl;
    cout << "patternSize.width =  " << patternSize.width << std::endl;
    cout << "square_size =  " << square_size << std::endl;
    
    ui.line1->setHidden(true);
    ui.line2->setHidden(true);
    ui.line3->setHidden(true);
    ui.line4->setHidden(true);
    QPalette palette = ui.line1->palette();
    palette.setColor(QPalette::Dark, Qt::green);
    ui.line1->setPalette(palette);
    ui.line2->setPalette(palette);
    ui.line3->setPalette(palette);
    ui.line4->setPalette(palette);

    worker = new StereoCalibrationWorker();
    workerThread = new QThread(this);
    worker->moveToThread(workerThread);
    connect(workerThread, &QThread::started, worker, &StereoCalibrationWorker::doWork);
    connect(worker, &StereoCalibrationWorker::updateText, this, &stereo_rectify::updateTextBrowser);
    connect(worker, &StereoCalibrationWorker::param_fresh, this, &stereo_rectify::param_fresh);
    connect(worker, &StereoCalibrationWorker::updateProgress, this, &stereo_rectify::updateProgress);
    connect(worker, &StereoCalibrationWorker::updateImage, this, &stereo_rectify::updateImage);

    //worker2 = new stereo_sgm();
    //worker2Thread = new QThread(this);


}

stereo_rectify::~stereo_rectify()
{
    workerThread->wait();
    delete worker;
}

std::string dTob_PoN(double number) {
    int decimal_bit = 20; // 保留几位二进制小数
    // 处理小数部分，将其转换为整数
    double dec = number * std::pow(2, decimal_bit);
    long long intDec = static_cast<long long>(dec); // 将double转换为整数

    std::string bina; // 用于存储最终的二进制字符串

    if (intDec >= 0) {
        // 非负数的处理
        bina = std::bitset<22>(intDec).to_string(); // 转换为二进制字符串，使用足够大的bitset以容纳数值
        // 移除前导0，直到小数点或二进制位数达到decimal_bit
        //auto pos = bina.find('1');
        //if (pos != std::string::npos) {
        //    bina = bina.substr(pos);
        //}
        //if (bina.length() < decimal_bit + 1) {
        //    bina = std::string(decimal_bit + 1 - bina.length(), '0') + bina; // 前面补0
        //}
        bina.insert(bina.length() - decimal_bit, ","); // 插入逗号作为小数点
    }
    else {
        // 负数的处理（这里简化了Python中的逻辑，直接使用二进制补码表示负数）
        bina = std::bitset<22>(intDec).to_string(); // 直接使用补码形式
        // 移除多余的前导1，保留足够的位数
        //auto pos = bina.find_last_of('1');
        //if (pos != std::string::npos) {
        //    bina = bina.substr(pos - decimal_bit, decimal_bit + 1);
        //}
        bina.insert(bina.length() - decimal_bit, ","); // 插入逗号作为小数点

    }
    return bina;
}
void stereo_rectify::param_fresh()
{
    ui.Param_Browser->clear();
    ss2.str("");
    ss2.clear(); // 可选，重置状态标志
    
    ss2 << "Image Size : " << imageSize << std::endl;
    ss2 << "fov_L = " << 2 * (atan(imageSize.width / 2 / cameraMatrix1_LC.at<double>(0, 0))) / 3.1415926 * 180;
    ss2 << "  fov_R = " << 2 * (atan(imageSize.width / 2 / cameraMatrix2_LC.at<double>(0, 0))) / 3.1415926 * 180 << std::endl;
    ss2 << "Camera Matrix_L: \n" << cameraMatrix1_LC << std::endl;
    ss2 << "Distortion Coefficients_L: \n" << distCoeffs1_LC.t() << std::endl;
    ss2 << "Camera Matrix_R: \n" << cameraMatrix2_LC << std::endl;
    ss2 << "Distortion Coefficients_R: \n" << distCoeffs2_LC.t() << std::endl;

    ss2 << "Single Parameter: " << std::endl;
    ss2 << "K1_L =  " << distCoeffs1_LC.at<double>(0, 0) << std::endl;
    ss2 << "K2_L =  " << distCoeffs1_LC.at<double>(1, 0) << std::endl;
    ss2 << "fx_L =  " << cameraMatrix1_LC.at<double>(0, 0) << std::endl;
    ss2 << "fy_L =  " << cameraMatrix1_LC.at<double>(1, 1) << std::endl;
    ss2 << "cx_L =  " << cameraMatrix1_LC.at<double>(0, 2) << std::endl;
    ss2 << "cy_L =  " << cameraMatrix1_LC.at<double>(1, 2) << std::endl;
    ss2 << "K1_R =  " << distCoeffs2_LC.at<double>(0, 0) << std::endl;
    ss2 << "K2_R =  " << distCoeffs2_LC.at<double>(1, 0) << std::endl;
    ss2 << "fx_R =  " << cameraMatrix2_LC.at<double>(0, 0) << std::endl;
    ss2 << "fy_R =  " << cameraMatrix2_LC.at<double>(1, 1) << std::endl;
    ss2 << "cx_R =  " << cameraMatrix2_LC.at<double>(0, 2) << std::endl;
    ss2 << "cy_R =  " << cameraMatrix2_LC.at<double>(1, 2) << std::endl;

    ui.Param_Browser->setText(QString::fromStdString(ss2.str()));

    param2_fresh();
}
void stereo_rectify::param2_fresh()
{
    ui.Param_Browser_2->clear();
    ss3.str("");
    ss3.clear(); // 可选，重置状态标志
    focal_length = abs(Q_LC.at<double>(2, 3));
    baseline = abs(1 / Q_LC.at<double>(3, 2));
    cx = abs(Q_LC.at<double>(0, 3));
    cy = abs(Q_LC.at<double>(1, 3));
    ss3 << "Tx = " << baseline << std::endl;
    ss3 << "fx*Tx = " << focal_length * baseline << std::endl;
    ss3 << "nearest(mm) = " << focal_length * baseline/255 << std::endl;
    ss3 << "fov = " << 2 * (atan(imageSize.width/2 / Q_LC.at<double>(2, 3))) / 3.1415926 * 180 << std::endl;
    ss3 << "Q: \n" << Q_LC << std::endl;
    ss3 << "R: \n" << R_LC << std::endl;
    ss3 << "T: \n" << T_LC.t() << std::endl;
    ss3 << "P1: \n" << P1_LC.t() << std::endl;
    ss3 << "P2: \n" << P2_LC.t() << std::endl;
    double yaw = atan2(R_LC.at<double>(1,0), R_LC.at<double>(0, 0));
    ss3 << "yaw: \n" << yaw << std::endl;
    
    ui.Param_Browser_2->setText(QString::fromStdString(ss3.str()));
}

std::vector<cv::Mat> loadImagesFromDirectory(const std::string& directoryPath) {
    std::vector<cv::Mat> images;
    for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
        cv::Mat img = cv::imread(entry.path().string());
        if (img.empty()) {
            ss << "Failed to load image: " << entry.path() << std::endl;
            message = ss.str();
            continue;
        }
        images.push_back(img);
    }
    return images;
}
std::vector<std::string> getImagePathsFromDirectory(const std::string& directoryPath) {
    std::vector<std::string> paths;
    std::vector<std::string> validExtensions = { ".jpg", ".bmp", ".png" };
    for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
        if (entry.is_regular_file()) {
            std::string extension = entry.path().extension().string();
            std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower); // 转换为小写

            if (std::find(validExtensions.begin(), validExtensions.end(), extension) != validExtensions.end()) {
                paths.push_back(entry.path().string());
            }
        }
    }
    return paths;
}
// 检测棋盘格角点
bool findChessboard(const cv::Mat& image, const cv::Size& patternSize, std::vector<cv::Point2f>& corners) {
    //cv::Mat gray, gray1, imgcolcor;
    //cv::cvtColor(image, gray1, cv::COLOR_BGR2GRAY);
    ////cv::imshow("raw", gray1);
    //cv::cvtColor(gray1, imgcolcor, cv::COLOR_BayerBG2BGR_VNG);
    ////cv::imshow("color", imgcolcor);
    //cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    ////cv::imshow("gray", gray);
    ////cv::waitKey(0);
    bool found = cv::findChessboardCornersSB(image, patternSize, corners, cv::CALIB_CB_ACCURACY | cv::CALIB_CB_EXHAUSTIVE);// cv::CALIB_CB_NORMALIZE_IMAGE |

    return found;
}

// 单目标定
void calibrateSingleCamera(const std::vector<std::vector<cv::Point3f>>& objectPoints,
    const std::vector<std::vector<cv::Point2f>>& imagePoints,
    const cv::Size& imageSize,
    cv::Mat& cameraMatrix, cv::Mat& distCoeffs, std::vector<cv::Mat>& rvecs, std::vector<cv::Mat>& tvecs) {
    cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
    distCoeffs = cv::Mat::zeros(8, 1, CV_64F);
    cv::calibrateCamera(objectPoints, imagePoints, imageSize, cameraMatrix, distCoeffs, rvecs, tvecs);
    std::cout << "Camera Matrix: \n" << cameraMatrix << std::endl;
    std::cout << "Distortion Coefficients: \n" << distCoeffs << std::endl;
    //std::cout << "rvecs[0]" << rvecs[0] << endl;
    //std::cout << "tvecs[0]" << tvecs[0] << endl;
}
double computeReprojectionErrors(
    const std::vector<std::vector<cv::Point3f>>& objectPoints,
    const std::vector<std::vector<cv::Point2f>>& imagePoints,
    const std::vector<cv::Mat>& rvecs, const std::vector<cv::Mat>& tvecs,
    const cv::Mat& cameraMatrix, const cv::Mat& distCoeffs,
    std::vector<float>& perViewErrors) {

    if (objectPoints.size() != imagePoints.size() ||
        objectPoints.size() != rvecs.size() ||
        objectPoints.size() != tvecs.size()) {
        std::cout << "objectPoints:" << objectPoints.size() << endl;
        std::cout << "rvecs:" << rvecs.size() << endl;
        std::cout << "tvecs:" << tvecs.size() << endl;
        std::cerr << "Error: Mismatched number of images, rotation vectors, or translation vectors." << std::endl;
        return -1;
    }

    std::vector<cv::Point2f> imagePoints2;
    size_t totalPoints = 0;
    double totalErr = 0, err;
    perViewErrors.resize(objectPoints.size());

    for (size_t i = 0; i < objectPoints.size(); ++i) {
        try {
            cv::projectPoints(objectPoints[i], rvecs[i], tvecs[i], cameraMatrix, distCoeffs, imagePoints2);
            err = cv::norm(imagePoints[i], imagePoints2, cv::NORM_L2);

            size_t n = objectPoints[i].size();
            perViewErrors[i] = (float)std::sqrt(err * err / n);
            totalErr += err * err;
            totalPoints += n;
        }
        catch (const cv::Exception& e) {
            std::cerr << "OpenCV Error: " << e.what() << std::endl;
            perViewErrors[i] = -1; // Indicate an error for this view
        }
        catch (const std::exception& e) {
            std::cerr << "Standard Exception: " << e.what() << std::endl;
            perViewErrors[i] = -1; // Indicate an error for this view
        }
        catch (...) {
            std::cerr << "Unknown Exception occurred" << std::endl;
            perViewErrors[i] = -1; // Indicate an error for this view
        }
    }

    if (totalPoints == 0) {
        std::cerr << "Error: No points were processed." << std::endl;
        return -1;
    }

    return std::sqrt(totalErr / totalPoints);
}

cv::Mat combineImagesSideBySide(const cv::Mat& img1, const cv::Mat& img2) {
    // 创建一个新的图像，宽度是两幅图像宽度之和，高度和类型与输入图像相同
    cv::Mat img_combined(img1.rows, img1.cols + img2.cols, img1.type());

    // 将第一幅图像复制到新图像的左侧
    img1.copyTo(img_combined(cv::Rect(0, 0, img1.cols, img1.rows)));

    // 将第二幅图像复制到新图像的右侧
    img2.copyTo(img_combined(cv::Rect(img1.cols, 0, img2.cols, img2.rows)));
    // 在合并后的图像上画横线以对比矫正结果
    int interval = 80; // 间隔80行画一条线
    for (int i = 0; i < img_combined.rows; i += interval) {
        cv::line(img_combined, cv::Point(0, i), cv::Point(img_combined.cols, i), cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
    }
    return img_combined;
}

void StereoCalibrationWorker::processChessboardImages(std::vector<std::string>& images1_path,
    std::vector<std::string>& images2_path,
    const cv::Size& patternSize,
    float square_size,
    std::vector<std::vector<cv::Point3f>>& objectPoints,
    std::vector<std::vector<cv::Point2f>>& imagePoints1,
    std::vector<std::vector<cv::Point2f>>& imagePoints2) 
{
    bool found1, found2, is_1chess;
    is_1chess = 1;
    std::vector<int> reject_num;
    cv::Mat img1 ;
    cv::Mat img2 ;
    for (int i = 0; i < images1_path.size(); i++) {
        img1 = cv::imread(images1_path[i]);
        img2 = cv::imread(images2_path[i]);
        std::vector<cv::Point2f> corners1, corners2;
        found1 = findChessboard(img1, patternSize, corners1);
        found2 = findChessboard(img2, patternSize, corners2);
        if (found1 && found2) {
            if (1) {
                is_1chess = 0;
            }
            cv::Mat images1_t = img1;
            cv::Mat images2_t = img2;
            cv::drawChessboardCorners(images1_t, patternSize, corners1, 1);
            cv::drawChessboardCorners(images2_t, patternSize, corners2, 1);
            cv::Mat img_pointed = combineImagesSideBySide(images1_t, images2_t);
            /*cv::imshow("pointed Image pair", img_pointed);
            cv::waitKey(0);*/

            imagePoints1.push_back(corners1);
            imagePoints2.push_back(corners2);
            // 填充objectPoints，例如对于9x6的棋盘格，每个格子的大小为1x1
            std::vector<cv::Point3f> obj;
            for (int j = 0; j < patternSize.height; j++)
                for (int k = 0; k < patternSize.width; k++)
                    obj.push_back(cv::Point3f(k * square_size, j * square_size, 0));
            objectPoints.push_back(obj);
        }
        else {
            cv::Mat img_reject(img1.rows, img1.cols * 2, img1.type());
            img1.copyTo(img_reject(cv::Rect(0, 0, img1.cols, img1.rows)));
            img2.copyTo(img_reject(cv::Rect(img1.cols, 0, img1.cols, img1.rows)));
            String str = "rejected image pair " + std::to_string(i + 1);
            std::cout << "rejected image pair " << str << std::endl;
            /*cv::imshow(str, img_reject);
            cv::waitKey(0);*/
            reject_num.push_back(i + 1);
        }

        float progress = (i + 1) * 50.0 / images1_path.size();
        std::cout << "\rfindChessboard Progress: " << std::fixed << std::setprecision(2) << progress*2 << "%" << std::flush;
        //ui.progressBar->setValue(0);
        
        emit updateProgress(progress);
    }
    ss << "\ndetected " << imagePoints1.size() << " image pairs." << std::endl;
    for (int i = 0; i < reject_num.size(); i++) {
        images1_path.erase(images1_path.begin() + reject_num[i]-1);
        images2_path.erase(images2_path.begin() + reject_num[i]-1);
        ss << "image pair " << reject_num[i] << " is rejected" << std::endl;

    }
    cout << "\ndetected " << images1_path.size() << " image pairs." << std::endl;
    emit updateText(QString::fromStdString(ss.str()));
    ss.str("");
}
void StereoCalibrationWorker::calibrateCameras(
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
    std::vector<cv::Mat>& tvecs2
) {

    // 相机1标定
    std::cout << "calibrateCamera1..." << std::endl;
    calibrateSingleCamera(objectPoints, imagePoints1, imageSize, cameraMatrix1, distCoeffs1, rvecs1, tvecs1);
    emit updateProgress(60);
    // 相机2标定
    std::cout << "calibrateCamera2..." << std::endl;
    calibrateSingleCamera(objectPoints, imagePoints2, imageSize, cameraMatrix2, distCoeffs2, rvecs2, tvecs2);
    emit updateProgress(70);
    // 计算相机1重投影误差
    std::vector<float> perViewErrors1;
    computeReprojectionErrors(objectPoints, imagePoints1, rvecs1, tvecs1, cameraMatrix1, distCoeffs1, perViewErrors1);
    // 计算相机2重投影误差
    std::vector<float> perViewErrors2;
    computeReprojectionErrors(objectPoints, imagePoints2, rvecs2, tvecs2, cameraMatrix2, distCoeffs2, perViewErrors2);
    // 设定重投影误差阈值
    float reprojectionErrorThreshold = 1.2; // 例如1像素
    bool iserase = false;
    // 根据重投影误差去除数据组
    emit updateText("High reprojection error pair of th = 1.2:");
    for (int i = objectPoints.size() - 1; i >= 0; --i) {
        std::ostringstream sss;
        sss << "pair " << i << " : " << perViewErrors1[i] << " " << perViewErrors2[i] << std::endl;
        if (perViewErrors1[i] > reprojectionErrorThreshold || perViewErrors2[i] > reprojectionErrorThreshold) {
            objectPoints.erase(objectPoints.begin() + i);
            imagePoints1.erase(imagePoints1.begin() + i);
            imagePoints2.erase(imagePoints2.begin() + i);

            iserase = true;
        }
    }
    emit updateProgress(75);
    // 使用过滤后的数据组再次标定相机1
    if (!objectPoints.empty()) {
        if (iserase) {
            std::cout << "Re-calibrating Camera 1 with filtered data..." << std::endl;
            calibrateSingleCamera(objectPoints, imagePoints1, imageSize, cameraMatrix1, distCoeffs1, rvecs1, tvecs1);
            emit updateProgress(80);
            std::cout << "Re-calibrating Camera 2 with filtered data..." << std::endl;
            calibrateSingleCamera(objectPoints, imagePoints2, imageSize, cameraMatrix2, distCoeffs2, rvecs2, tvecs2);
        }
        else {
            std::cout << "all the image pairs are maintained..." << std::endl;
            emit updateText("all the image pairs are maintained...");
        }
    }
    else {
        std::cout << "all the image pairs are filtered..." << std::endl;
        emit updateText("all the image pairs are filtered...");
        return;
    }
    emit updateProgress(85);
}


void StereoCalibrationWorker::stereoCalibration(
    const std::vector<std::vector<cv::Point3f>>& objectPoints,
    const std::vector<std::vector<cv::Point2f>>& imagePoints1,
    const std::vector<std::vector<cv::Point2f>>& imagePoints2,
    cv::Mat& cameraMatrix1, cv::Mat& distCoeffs1,
    cv::Mat& cameraMatrix2, cv::Mat& distCoeffs2,
    const cv::Size& imageSize, cv::Mat& R, cv::Mat& T, cv::Mat& E, cv::Mat& F) {

    cout << "cameraMatrix1 before refine: " << endl << cameraMatrix1 << endl;
    cout << "cameraMatrix2 before refine: " << endl << cameraMatrix2 << endl;

    // 双目标定
    std::cout << "stereoCalibrate" << std::endl;
    cv::Mat perViewErrors;

    cv::stereoCalibrate(objectPoints, imagePoints1, imagePoints2,
        cameraMatrix1, distCoeffs1, cameraMatrix2, distCoeffs2,
        imageSize, R, T, E, F, perViewErrors, cv::CALIB_USE_INTRINSIC_GUESS);
    cout << "R_rl before re-stereoCalibrate: " << endl << R << endl;
    cout << "t_rl before re-stereoCalibrate: " << endl << T.t() << endl;
    emit updateProgress(95);

    //cv::stereoCalibrate(objectPoints, imagePoints1, imagePoints2,
    //	cameraMatrix1, distCoeffs1, cameraMatrix2, distCoeffs2,
    //	imageSize, R, T, E, F,cv::CALIB_FIX_INTRINSIC, perViewErrors);

    // 设置重投影误差的阈值
    double threshold = 1.2;

    // 存储重投影误差较小的图像索引
    std::vector<int> goodViews;
    emit updateText("High stereo reprojection error pair of th = 1.2:");
    
    for (int i = 0; i < perViewErrors.rows; ++i) {
        double error = perViewErrors.at<double>(i, 0);
        double error1 = perViewErrors.at<double>(i, 1);
        if ((error < threshold) && (error1 < threshold)) {
            goodViews.push_back(i);
            //std::ostringstream sss;
            //sss << "View " << i << " has low reprojection error: " << error << " and " << error1 << std::endl;
            //emit updateText(QString::fromStdString(sss.str()));
        }
        else {
            std::ostringstream sss;
            sss << "pair " << i << " : " << error << " " << error1 ;
            emit updateText(QString::fromStdString(sss.str()));
        }
    }

    // 根据选定的好的视图创建新的数据集
    std::vector<std::vector<cv::Point3f>> newObjectPoints(goodViews.size());
    std::vector<std::vector<cv::Point2f>> newImagePoints1(goodViews.size()), newImagePoints2(goodViews.size());
    for (size_t i = 0; i < goodViews.size(); ++i) {
        int idx = goodViews[i];
        newObjectPoints[i] = objectPoints[idx];
        newImagePoints1[i] = imagePoints1[idx];
        newImagePoints2[i] = imagePoints2[idx];
    }
    if (newObjectPoints.empty()) {
        std::cout << "There are no more image pairs in the newObjectPoints.." << std::endl;
        return;
    }
    std::cout << "Re-stereoCalibrate" << std::endl;
    cv::stereoCalibrate(newObjectPoints, newImagePoints1, newImagePoints2,
        cameraMatrix1, distCoeffs1, cameraMatrix2, distCoeffs2,
        imageSize, R, T, E, F, perViewErrors, cv::CALIB_USE_INTRINSIC_GUESS);
    // 可以选择使用新的数据集再次进行标定或其他操作

    cout << "R_rl after re-stereoCalibrate: " << endl << R << endl;
    cout << "t_rl after re-stereoCalibrate: " << endl << T.t() << endl;

    cout << "cameraMatrix1 after refine: " << endl << cameraMatrix1 << endl;
    cout << "cameraMatrix2 after refine: " << endl << cameraMatrix2 << endl;

    return; // 或者根据情况返回不同的状态码
}


void createpath(fs::path path) {
    if (!fs::exists(path)) {
        // 文件夹不存在，尝试创建文件夹
        if (fs::create_directories(path)) {
            std::cout << "文件夹创建成功！" << std::endl;
        }
        else {
            std::cerr << "文件夹创建失败！" << std::endl;
            //return 1;
        }
    }
    else {
        std::cout << "文件夹已存在！" << std::endl;
    }
    return;
}

QPixmap matToQPixmap(const cv::Mat& mat) {
    // 处理不同类型的Mat
    switch (mat.type()) {
        // 8-bit, 4 channel
    case CV_8UC4: {
        QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return QPixmap::fromImage(image);
    }
                // 8-bit, 3 channel
    case CV_8UC3: {
        QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return QPixmap::fromImage(image.rgbSwapped());
    }
                // 8-bit, 1 channel
    case CV_8UC1: {
        QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8);
        return QPixmap::fromImage(image);
    }
    default:
        // 未处理的Mat类型
        qWarning("matToQPixmap() - cv::Mat image type not handled in switch:");
        return QPixmap();
    }
}
void stereo_rectify::on_LeftImage_clicked()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, tr("选择文件夹"), "/home",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (folderPath.isEmpty()) {
        return; // 如果没有选择文件夹，直接退出函数
    }
    // 处理folderPath
    // 例如，你可以将路径显示在状态栏或者文本框中
    ui.statusBar->showMessage(folderPath);
    projpath = folderPath.toStdString();
    ui.textBrowser->setText(folderPath);
    std::cout << projpath << std::endl;
    const std::string folder1 = projpath + "/img_L";
    const std::string folder2 = projpath + "/img_R";
    images1_path = getImagePathsFromDirectory(folder1);
    images2_path = getImagePathsFromDirectory(folder2);
    ui.textBrowser->setText(QString::fromStdString(message));
    ss << "Loaded " << images1_path.size() << " images from img_L." << endl;
    ss << "Loaded " << images2_path.size() << " images from img_R." << endl;
    updateTextBrowser(QString::fromStdString(ss.str()));
    ss.str("");
    if (images1_path.size()==0) {
        return; // 如果没有选择文件夹，直接退出函数
    }
    cout << images1_path[0] << endl;
    images1 = cv::imread(images1_path[0]);
    images2 = cv::imread(images2_path[0]);
    imageSize = images1.size();

    updateImage(images1, images2);
}
void stereo_rectify::on_Calib_clicked()
{
    if (is_workfinish) {
        workerThread->terminate();
        is_workfinish = 0;
    }
    workerThread->start();
}

void StereoCalibrationWorker::doWork()
{
    bool isLoad = 1;
    
    emit updateProgress(0);
    emit updateText("findChessboardLC... ");
    processChessboardImages(images1_path, images2_path, patternSize, square_size, objectPoints_LC, imagePoints1_LC, imagePoints2_LC);

    emit updateText("calibrateCamerasLC... ");
    calibrateCameras(projpath, objectPoints_LC, imagePoints1_LC, imagePoints2_LC, imageSize, cameraMatrix1_LC, distCoeffs1_LC, rvecs1_LC, tvecs1_LC, cameraMatrix2_LC, distCoeffs2_LC, rvecs2_LC, tvecs2_LC);

    emit updateText("stereoCalibrationLC... ");
    stereoCalibration(objectPoints_LC, imagePoints1_LC, imagePoints2_LC, cameraMatrix1_LC, distCoeffs1_LC, cameraMatrix2_LC, distCoeffs2_LC,
        imageSize, R_LC, T_LC, E_LC, F_LC);

    emit updateText("save parameter... ");

    cv::FileStorage fs(projpath + "/opencvStereoParam_LC.xml", cv::FileStorage::WRITE);
    fs << "size" << imageSize;
    fs << "M1" << cameraMatrix1_LC;
    fs << "D1" << distCoeffs1_LC;
    fs << "M2" << cameraMatrix2_LC;
    fs << "D2" << distCoeffs2_LC;
    fs << "R" << R_LC;
    fs << "T" << T_LC;
    fs.release();
    cv::stereoRectify(cameraMatrix1_LC, distCoeffs1_LC, cameraMatrix2_LC, distCoeffs2_LC,
        imageSize, R_LC, T_LC, R1_LC, R2_LC, P1_LC, P2_LC, Q_LC, -1, 1);
    emit param_fresh();
    emit updateProgress(100);
    cv::initUndistortRectifyMap(cameraMatrix1_LC, distCoeffs1_LC, R1_LC, P1_LC, imageSize, CV_16SC2, map11_LC, map12_LC);
    cv::initUndistortRectifyMap(cameraMatrix2_LC, distCoeffs2_LC, R2_LC, P2_LC, imageSize, CV_16SC2, map21_LC, map22_LC);

    std::cout << "remap" << std::endl;
    cv::remap(images1, images1_rec, map11_LC, map12_LC, cv::INTER_LINEAR);
    cv::remap(images2, images2_rec, map21_LC, map22_LC, cv::INTER_LINEAR);
    is_remap = 1;
    is_workfinish = 1;


}

void stereo_rectify::on_Import_clicked()
{
    ss << "Load StereoParam_LC... " << std::endl;
    updateTextBrowser(QString::fromStdString(ss.str()));
    ss.str("");

    cv::FileStorage fs(projpath + "/opencvStereoParam_LC.xml", cv::FileStorage::READ); // Replace with your actual file path
    if (!fs.isOpened()) {
        ss << "Failed to open calibration file" << std::endl;
        updateTextBrowser(QString::fromStdString(ss.str()));
        ss.str("");
        return;
    }
    fs["M1"] >> cameraMatrix1_LC;
    fs["D1"] >> distCoeffs1_LC;
    fs["M2"] >> cameraMatrix2_LC;
    fs["D2"] >> distCoeffs2_LC;
    fs["R"] >> R_LC;
    fs["T"] >> T_LC;
    fs.release(); // Close the file
    cv::stereoRectify(cameraMatrix1_LC, distCoeffs1_LC, cameraMatrix2_LC, distCoeffs2_LC,
        imageSize, R_LC, T_LC, R1_LC, R2_LC, P1_LC, P2_LC, Q_LC, -1, 1);
    param_fresh();
    cout << "M1" << cameraMatrix1_LC << endl;
    cout << "D1" << distCoeffs1_LC << endl;
    cout << "M2" << cameraMatrix2_LC << endl;
    cout << "D2" << distCoeffs2_LC << endl;
    cout << "R" << R_LC << endl;
    cout << "T" << T_LC << endl;

    //cv::FileStorage fs1(projpath + "/opencvStereoParam_LC-2.xml", cv::FileStorage::READ); // Replace with your actual file path
    //if (!fs1.isOpened()) {
    //    ss << "Failed to open calibration file" << std::endl;
    //    updateTextBrowser(QString::fromStdString(ss.str()));
    //    ss.str("");
    //    return;
    //}
    //fs1["M1"] >> cameraMatrix1_LC;
    //fs1["D1"] >> distCoeffs1_LC;
    //fs1["M2"] >> cameraMatrix2_LC;
    //fs1["D2"] >> distCoeffs2_LC;
    //fs1["R"] >> R_LC;
    //fs1["T"] >> T_LC;
    //fs1.release(); // Close the file
    //cv::stereoRectify(cameraMatrix1_LC, distCoeffs1_LC, cameraMatrix2_LC, distCoeffs2_LC,
    //    imageSize, R_LC, T_LC, R1_LC, R2_LC, P1_LC, P2_LC, Q_LC, -1, 1);
    //param_fresh();

    //cout << "M1" << cameraMatrix1_LC << endl;
    //cout << "D1" << distCoeffs1_LC << endl;
    //cout << "M2" << cameraMatrix2_LC << endl;
    //cout << "D2" << distCoeffs2_LC << endl;
    //cout << "R" << R_LC << endl;
    //cout << "T" << T_LC << endl;


    // 将 R_21 和 t_21 转换为两个校正相机坐标系的变换
    Mat R_LC_ar = R2_LC * R_LC * R1_LC.t();
    cout << "R_ after rectification: " << endl << R_LC_ar << endl;
    Mat T_LC_ar = R2_LC * T_LC;
    cout << "T_ after rectification: " << endl << T_LC_ar.t() << endl;

    // 打印投影矩阵
    cout << "Pl: " << endl << P1_LC << endl;
    cout << "Pr: " << endl << P2_LC << endl;
    cout << "Q: " << endl << Q_LC << endl;
    std::cout << "initUndistortRectifyMap" << std::endl;
    
    cv::initUndistortRectifyMap(cameraMatrix1_LC, distCoeffs1_LC, R1_LC, P1_LC, imageSize, CV_16SC2, map11_LC, map12_LC);
    cv::initUndistortRectifyMap(cameraMatrix2_LC, distCoeffs2_LC, R2_LC, P2_LC, imageSize, CV_16SC2, map21_LC, map22_LC);

    std::cout << "remap" << std::endl;
    cv::remap(images1, images1_rec, map11_LC, map12_LC, cv::INTER_LINEAR);
    cv::remap(images2, images2_rec, map21_LC, map22_LC, cv::INTER_LINEAR);
    is_remap = 1;

}


void stereo_rectify::on_TestImage_clicked()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, tr("选择文件夹"), "/home",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (folderPath.isEmpty()) {
        return; // 如果没有选择文件夹，直接退出函数
    }

    // 处理folderPath
    // 例如，你可以将路径显示在状态栏或者文本框中
    ui.statusBar->showMessage(folderPath);
    projpath_test = folderPath.toStdString();
    ui.textBrowser->setText(folderPath);
    std::cout << projpath_test << std::endl;
    const std::string folder1 = projpath_test + "/test_L";
    const std::string folder2 = projpath_test + "/test_R";
    //images_test1 = loadImagesFromDirectory(folder1);
    //images_test2 = loadImagesFromDirectory(folder2);
    imagePaths1 = getImagePathsFromDirectory(folder1);
    imagePaths2 = getImagePathsFromDirectory(folder2);
    ui.textBrowser->setText(QString::fromStdString(message));
    ss << "Loaded " << imagePaths1.size() << " images from test_L." << endl;
    ss << "Loaded " << imagePaths2.size() << " images from test_R." << endl;
    updateTextBrowser(QString::fromStdString(ss.str()));
    ss.str("");
    if (imagePaths1.size() == 0 || imagePaths2.size() == 0) {
        return; // 如果没有选择文件夹，直接退出函数
    }
    std::cout << imagePaths1[0] << std::endl;
    images1 = cv::imread(imagePaths1[0]);
    images2 = cv::imread(imagePaths2[0]);
    imageSize = images1.size();
    cv::remap(images1, images1_rec, map11_LC, map12_LC, cv::INTER_LINEAR);
    cv::remap(images2, images2_rec, map21_LC, map22_LC, cv::INTER_LINEAR);
    updateImage(images1, images2);
}

void stereo_rectify::on_Rectify_clicked()
{
    fs::path path1(projpath_test + String("/rec_img/img_L"));
    fs::path path2(projpath_test + String("/rec_img/img_R"));
    createpath(path1);
    createpath(path2);
    int imagePaths_size;
    if (imagePaths1.size() <= imagePaths2.size()) {
        imagePaths_size = imagePaths1.size();
    }
    else {
        imagePaths_size = imagePaths2.size();
    }
    for (int i = 0; i < imagePaths_size; i++) {
	cv::Mat img1_rectified_t_LC, img2_rectified_t_LC;
    cv::Mat img1 = cv::imread(imagePaths1[i]);
    cv::Mat img2 = cv::imread(imagePaths2[i]);
	cv::remap(img1, img1_rectified_t_LC, map11_LC, map12_LC, cv::INTER_LINEAR);
	cv::remap(img2, img2_rectified_t_LC, map21_LC, map22_LC, cv::INTER_LINEAR);
	cv::imwrite(projpath_test + "/rec_img/img_L/" + std::to_string(i) + ".bmp", img1_rectified_t_LC);
	cv::imwrite(projpath_test + "/rec_img/img_R/" + std::to_string(i) + ".bmp", img2_rectified_t_LC);
	float progress = (i + 1) * 100.0 / imagePaths_size;
    ui.progressBar->setValue(progress);
    }

}

void stereo_rectify::on_showRectify_cliked()
{
    if (!is_remap) {
        emit updateTextBrowser("Please import calibration parameters");
        return;
    }

    if (is_rec) {
        is_rec = 0;
        updateImage(images1_rec, images2_rec);
        QPalette palette = ui.show_rectify->palette();
        palette.setColor(QPalette::ButtonText, Qt::red);  // 设置按钮的文字颜色为红色
        ui.show_rectify->setPalette(palette);
    }
    else {
        is_rec = 1;
        updateImage(images1, images2);
        QPalette palette = ui.show_rectify->palette();
        palette.setColor(QPalette::ButtonText, Qt::black);
        ui.show_rectify->setPalette(palette);
    }
   
}

void stereo_rectify::on_Crop_cliked()
{
    if (is_Crop == 0) {
        cv::stereoRectify(cameraMatrix1_LC, distCoeffs1_LC, cameraMatrix2_LC, distCoeffs2_LC,
            imageSize, R_LC, T_LC, R1_LC, R2_LC, P1_LC, P2_LC, Q_LC, -1, 0);
        param_fresh();

        // 将 R_21 和 t_21 转换为两个校正相机坐标系的变换
        Mat R_LC_ar = R2_LC * R_LC * R1_LC.t();
        cout << "R_ after rectification: " << endl << R_LC_ar << endl;
        Mat T_LC_ar = R2_LC * T_LC;
        cout << "T_ after rectification: " << endl << T_LC_ar.t() << endl;

        // 打印投影矩阵
        cout << "Pl: " << endl << P1_LC << endl;
        cout << "Pr: " << endl << P2_LC << endl;
        cout << "Q: " << endl << Q_LC << endl;
        std::cout << "initUndistortRectifyMap" << std::endl;

        cv::initUndistortRectifyMap(cameraMatrix1_LC, distCoeffs1_LC, R1_LC, P1_LC, imageSize, CV_16SC2, map11_LC, map12_LC);
        cv::initUndistortRectifyMap(cameraMatrix2_LC, distCoeffs2_LC, R2_LC, P2_LC, imageSize, CV_16SC2, map21_LC, map22_LC);

        std::cout << "remap" << std::endl;
        cv::remap(images1, images1_rec, map11_LC, map12_LC, cv::INTER_LINEAR);
        cv::remap(images2, images2_rec, map21_LC, map22_LC, cv::INTER_LINEAR);
        updateImage(images1_rec, images2_rec);

        QPalette palette = ui.Crop->palette();
        palette.setColor(QPalette::ButtonText, Qt::red);  // 设置按钮的文字颜色为红色
        ui.Crop->setPalette(palette);
        is_Crop = 1;
        QPalette palette1 = ui.show_rectify->palette();
        palette1.setColor(QPalette::ButtonText, Qt::red);  // 设置按钮的文字颜色为红色
        ui.show_rectify->setPalette(palette1);
    }
    else if(is_Crop == 2) {
        cv::stereoRectify(cameraMatrix1_LC, distCoeffs1_LC, cameraMatrix2_LC, distCoeffs2_LC,
            imageSize, R_LC, T_LC, R1_LC, R2_LC, P1_LC, P2_LC, Q_LC, -1, 1);
        param_fresh();
        cv::initUndistortRectifyMap(cameraMatrix1_LC, distCoeffs1_LC, R1_LC, P1_LC, imageSize, CV_16SC2, map11_LC, map12_LC);
        cv::initUndistortRectifyMap(cameraMatrix2_LC, distCoeffs2_LC, R2_LC, P2_LC, imageSize, CV_16SC2, map21_LC, map22_LC);

        std::cout << "remap" << std::endl;
        cv::remap(images1, images1_rec, map11_LC, map12_LC, cv::INTER_LINEAR);
        cv::remap(images2, images2_rec, map21_LC, map22_LC, cv::INTER_LINEAR);
        updateImage(images1_rec, images2_rec);
        QPalette palette = ui.Crop->palette();
        palette.setColor(QPalette::ButtonText, Qt::black);  
        ui.Crop->setPalette(palette);
        is_Crop = 0;
        QPalette palette1 = ui.show_rectify->palette();
        palette1.setColor(QPalette::ButtonText, Qt::red);  // 设置按钮的文字颜色为红色
        ui.show_rectify->setPalette(palette1);
    }
    else {
        cv::Mat R1_LC_noc, R2_LC_noc, P1_LC_noc, P2_LC_noc, Q_LC_noc;
        cv::Mat R1_LC_c, R2_LC_c, P1_LC_c, P2_LC_c, Q_LC_c;
        cv::stereoRectify(cameraMatrix1_LC, distCoeffs1_LC, cameraMatrix2_LC, distCoeffs2_LC,
            imageSize, R_LC, T_LC, R1_LC_noc, R2_LC_noc, P1_LC_noc, P2_LC_noc, Q_LC_noc, -1, 0);
        cv::stereoRectify(cameraMatrix1_LC, distCoeffs1_LC, cameraMatrix2_LC, distCoeffs2_LC,
            imageSize, R_LC, T_LC, R1_LC_c, R2_LC_c, P1_LC_c, P2_LC_c, Q_LC_c, -1, 1);
        P1_LC = (P1_LC_noc + P1_LC_c) / 2;
        P2_LC = (P2_LC_noc + P2_LC_c) / 2;
        Q_LC = (Q_LC_noc + Q_LC_c) / 2;
        param_fresh();
        cv::initUndistortRectifyMap(cameraMatrix1_LC, distCoeffs1_LC, R1_LC, P1_LC, imageSize, CV_16SC2, map11_LC, map12_LC);
        cv::initUndistortRectifyMap(cameraMatrix2_LC, distCoeffs2_LC, R2_LC, P2_LC, imageSize, CV_16SC2, map21_LC, map22_LC);

        std::cout << "remap" << std::endl;
        cv::remap(images1, images1_rec, map11_LC, map12_LC, cv::INTER_LINEAR);
        cv::remap(images2, images2_rec, map21_LC, map22_LC, cv::INTER_LINEAR);
        updateImage(images1_rec, images2_rec);
        QPalette palette = ui.Crop->palette();
        palette.setColor(QPalette::ButtonText, Qt::green);
        ui.Crop->setPalette(palette);
        is_Crop = 2;
        QPalette palette1 = ui.show_rectify->palette();
        palette1.setColor(QPalette::ButtonText, Qt::red);  // 设置按钮的文字颜色为红色
        ui.show_rectify->setPalette(palette1);
    
    }
    
}
void stereo_rectify::on_computeRange_cliked()
{
    cv::Mat a1 = cv::Mat::zeros(3, 3, CV_64F);
    cv::Mat iR_L = cv::Mat::zeros(3, 3, CV_64F);
    P1_LC.col(0).copyTo(a1.col(0));
    P1_LC.col(1).copyTo(a1.col(1));
    P1_LC.col(2).copyTo(a1.col(2));
    cv::Mat a2 = cv::Mat::zeros(3, 3, CV_64F);
    cv::Mat iR_R = cv::Mat::zeros(3, 3, CV_64F);
    P2_LC.col(0).copyTo(a2.col(0));
    P2_LC.col(1).copyTo(a2.col(1));
    P2_LC.col(2).copyTo(a2.col(2));
    cout << "P1_LC: " << endl << P1_LC << endl;
    cout << "a1: " << endl << a1 << endl;
    // Matrix multiplication and inversion
    iR_L = a1 * R1_LC;
    cout << "iR_L: " << endl << iR_L << endl;
    iR_L = iR_L.inv();
    cout << "iR_L_inv: " << endl << iR_L << endl;
    iR_R = a2 * R2_LC;
    iR_R = iR_R.inv();
    double k1, k2, k3, k4, k5, k6, fx, fy, u0, v0, p1, p2;
    u0 = cameraMatrix1_LC.at<double>(0, 2);
    cout << "u0: " << endl << u0 << endl;
    v0 = cameraMatrix1_LC.at<double>(1, 2);
    cout << "v0: " << endl << v0 << endl;
    fx = cameraMatrix1_LC.at<double>(0, 0);
    cout << "fx: " << endl << fx << endl;
    fy = cameraMatrix1_LC.at<double>(1, 1);
    cout << "fy: " << endl << fy << endl;
    k1 = distCoeffs1_LC.at<double>(0, 0);
    cout << "k1: " << endl << k1 << endl;
    k2 = distCoeffs1_LC.at<double>(1, 0);
    cout << "k2: " << endl << k2 << endl;
    p1 = p2 = k3 = k4 = k5 = k6 = 0;
    k1 = k2 = 0;
    double x_range = 0;
    double y_range = 0;
    double progress_interval = 10.0; // Define your progress interval
    //cv::Mat map1 = cv::Mat::zeros(imageSize, CV_64F);
    //cv::Mat map2 = cv::Mat::zeros(imageSize, CV_64F);
    cv::Mat map1, map2;
    map1.create(imageSize, CV_32FC1);
    map2.create(imageSize, CV_32FC1);
    for (int i = 0; i < imageSize.height; ++i) {
        double _x = i * iR_L.at<double>(0, 1) + iR_L.at<double>(0, 2);
        double _y = i * iR_L.at<double>(1, 1) + iR_L.at<double>(1, 2);
        double _w = i * iR_L.at<double>(2, 1) + iR_L.at<double>(2, 2);

        // Print progress
        float progress = (i + 1) * 100.0 / imageSize.height;
        ui.progressBar->setValue(progress);
        //cout << "_x =  " << _x << std::endl;
        //cout << "_y =  " << _y << std::endl;
        //cout << "_w =  " << _w << std::endl;

        for (int j = 0; j < imageSize.width; ++j) {
            double w = 1.0 / _w;
            double x = _x * w;
            double y = _y * w;

            double r2 = x * x + y * y;
            double _2xy = 2 * x * y;
            double kr = (1 + ((k3 * r2 + k2) * r2 + k1) * r2) / (1 + ((k6 * r2 + k5) * r2 + k4) * r2);
            double u = fx * (x * kr + p1 * _2xy + p2 * (r2 + 2 * x * x)) + u0;
            double v = fy * (y * kr + p1 * (r2 + 2 * y * y) + p2 * _2xy) + v0;

            //cout << "x =  " << x << std::endl;
            //cout << "y =  " << y << std::endl;
            //cout << "w =  " << w << std::endl;
            //cout << "u =  " << u << std::endl;
            //cout << "v =  " << v << std::endl;
            map1.at<float>(i, j) = static_cast<float>(u);
            map2.at<float>(i, j) = static_cast<float>(v);

            double x_range1 = std::abs(u - j);
            double y_range1 = std::abs(v - i);
            double xdiv_w1 = std::abs(x);
            double ydiv_w1 = std::abs(y);

            if (x_range < x_range1) x_range = x_range1;
            if (y_range < y_range1) y_range = y_range1;

            _x += iR_L.at<double>(0, 0);
            _y += iR_L.at<double>(1, 0);
            _w += iR_L.at<double>(2, 0);

        }
    }
    cv::Mat images1_rec_handby;
    cv::remap(images1, images1_rec_handby, map1, map2, cv::INTER_LANCZOS4, cv::BORDER_CONSTANT, 0);
    cv::imwrite("Left_rec_handby.png", images1_rec_handby);

    ss << "x_range= " << x_range << endl;
    ss << "y_range= " << y_range << endl;

    cout << "K1_L =  " << dTob_PoN(distCoeffs1_LC.at<double>(0, 0)) << std::endl;
    cout << "K2_L =  " << dTob_PoN(distCoeffs1_LC.at<double>(1, 0)) << std::endl;
    cout << "fx_L =  " << cameraMatrix1_LC.at<double>(0, 0) << std::endl;
    cout << "fy_L =  " << cameraMatrix1_LC.at<double>(1, 1) << std::endl;
    cout << "cx_L =  " << cameraMatrix1_LC.at<double>(0, 2) << std::endl;
    cout << "cy_L =  " << cameraMatrix1_LC.at<double>(1, 2) << std::endl;
    cout << "K1_R =  " << dTob_PoN(distCoeffs2_LC.at<double>(0, 0)) << std::endl;
    cout << "K2_R =  " << dTob_PoN(distCoeffs2_LC.at<double>(1, 0)) << std::endl;
    cout << "fx_R =  " << cameraMatrix2_LC.at<double>(0, 0) << std::endl;
    cout << "fy_R =  " << cameraMatrix2_LC.at<double>(1, 1) << std::endl;
    cout << "cx_R =  " << cameraMatrix2_LC.at<double>(0, 2) << std::endl;
    cout << "cy_R =  " << cameraMatrix2_LC.at<double>(1, 2) << std::endl;
    int a_n = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            a_n++;
            std::string iR_n = dTob_PoN(iR_L.at<double>(i, j));
            std::cout << "iR_L" << a_n << "=22'b" << iR_n << std::endl;
        }
    }
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            a_n++;
            std::string iR_n = dTob_PoN(iR_R.at<double>(i, j));
            std::cout << "iR_R" << a_n << "=22'b" << iR_n << std::endl;
        }
    }


    updateTextBrowser(QString::fromStdString(ss.str()));
    ss.str("");

}
void stereo_rectify::on_disLine_cliked()
{
    //string a = dTob_PoN(-0.8);
    //string b = dTob_PoN(-0.25);
    //string d = dTob_PoN(-0.465456);
    //string d2 = dTob_PoN(-0.4);
    //string c = dTob_PoN(-0.75);

    //dTob_PoN(0.75);
    //dTob_PoN(0.25);
    //dTob_PoN(0);
    //dTob_PoN(0.465456);

    if (!is_Line) {
        ui.line1->setHidden(false);
        ui.line2->setHidden(false);
        ui.line3->setHidden(false);
        ui.line4->setHidden(false);
        is_Line = 1;
    }
    else {
        ui.line1->setHidden(true);
        ui.line2->setHidden(true);
        ui.line3->setHidden(true);
        ui.line4->setHidden(true);
        is_Line = 0;
    }
}

void stereo_rectify::updateTextBrowser(const QString& text) 
{
    ui.textBrowser->append(text);
}
void stereo_rectify::updateProgress(float progress)
{
    ui.progressBar->setValue(progress);
}
void stereo_rectify::updateImage(cv::Mat images1, cv::Mat images2)
{
    QPixmap pixmap_L = matToQPixmap(images1);
    QPixmap pixmap_R = matToQPixmap(images2);
    ui.image_L->setPixmap(pixmap_L.scaled(ui.image_L->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui.image_R->setPixmap(pixmap_R.scaled(ui.image_R->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
void stereo_rectify::on_ConfirmChess_cliked()
{
    patternSize.height = ui.pSizeH->text().toInt();
    patternSize.width = ui.pSizeW->text().toInt();
    square_size = ui.sqSize->text().toInt();
    ss << "Chessboard parameter confirmed! " << endl;
    cout << "patternSize.height =  " << patternSize.height << std::endl;
    cout << "patternSize.width =  " << patternSize.width << std::endl;
    cout << "square_size =  " << square_size << std::endl;
    updateTextBrowser(QString::fromStdString(ss.str()));
    ss.str("");
}

void stereo_rectify::on_stereo_cliked() {
    QString folderPath = QFileDialog::getExistingDirectory(this, tr("选择文件夹"), "/home",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (folderPath.isEmpty()) {
        return; // 如果没有选择文件夹，直接退出函数
    }
    // 处理folderPath
    // 例如，你可以将路径显示在状态栏或者文本框中
    ui.statusBar->showMessage(folderPath);
    std::string stereopath = folderPath.toStdString();
    ui.textBrowser->setText(folderPath);
    std::cout << stereopath << std::endl;
    const std::string folder1 = stereopath + "/img_L";
    const std::string folder2 = stereopath + "/img_R";
    std::vector<string> img_L_paths = getImagePathsFromDirectory(folder1);
    std::vector<string> img_R_paths= getImagePathsFromDirectory(folder2);
    ui.textBrowser->setText(QString::fromStdString(message));
    ss << "Loaded " << img_L_paths.size() << " images from img_L for stereo." << endl;
    ss << "Loaded " << img_R_paths.size() << " images from img_R for stereo." << endl;
    updateTextBrowser(QString::fromStdString(ss.str()));
    ss.str("");
    cout << "Loaded " << img_L_paths.size() << " images from img_L for stereo." << endl;
    cout << "Loaded " << img_R_paths.size() << " images from img_R for stereo." << endl;
    if (img_L_paths.size() == 0) {
        cout << " error: no images" << endl;
        return; // 如果没有选择文件夹，直接退出函数
    }
    cout << img_L_paths[0] << endl;
    cv::Mat img_L = cv::imread(img_L_paths[0]);
    cv::Mat img_R = cv::imread(img_R_paths[0]);
    updateImage(img_L, img_R);
    fs::path path1(stereopath + String("/disp"));
    createpath(path1);
    stereo_sgm stereo_sgm;
    //stereo_sgm.main(stereopath);

    for (int i = 0; i < img_L_paths.size(); i++) {
        cv::Mat disp_sgbm_norm;
        disp_sgbm_norm = stereo_sgm.sgbm_main(img_L_paths[i], img_R_paths[i]);
        cv::Mat img_cloud = cv::imread(img_L_paths[0]);
        cv::imwrite(stereopath + "/disp/" + std::to_string(i) + "_sgbm.bmp", disp_sgbm_norm);
        cv::Mat disp_rsgm_norm;
        disp_rsgm_norm = stereo_sgm.main(img_L_paths[i], img_R_paths[i], stereopath + "/disp/");
        cv::imwrite(stereopath + "/disp/" + std::to_string(i) + "_rsgm.bmp", disp_rsgm_norm);
        float progress = (i + 1) * 100.0 / img_L_paths.size();
        ui.progressBar->setValue(progress);
        //cv::Mat PointCloud 2=0 stereo_sgm.Disp2PCloud(disp_sgbm_norm,  focal_length,  baseline,cx,cy);
        //stereo_sgm.Mat2PCL(img_L, PointCloud, stereopath + "/disp/" + std::to_string(i) + ".pcd");
    }

}

void stereo_rectify::on_next_img_cliked() {
    
    
    //images1 = cv::imread(images1_path[img_num]);
    //images2 = cv::imread(images2_path[img_num]);
    //cv::remap(images1, images1_rec, map11_LC, map12_LC, cv::INTER_LINEAR);
    //cv::remap(images2, images2_rec, map21_LC, map22_LC, cv::INTER_LINEAR);
    //if (is_workfinish == 1) {
    //    images1_chess = images1;
    //    images2_chess = images2;
    //    cv::drawChessboardCorners(images1_chess, patternSize, imagePoints1_LC[img_num], 1);
    //    cv::drawChessboardCorners(images2_chess, patternSize, imagePoints2_LC[img_num], 1);
    //    updateImage(images1_chess, images2_chess);
    //}
    //else {
    //    updateImage(images1, images2);
    //}
    //img_num = img_num + 1;
    //if (img_num == images1_path.size()) {
    //    img_num = 0;
    //}
    stereo_sgm stereo_sgm;
    //std::string outputDir = "E:\\Studyproj\\24Proj\\meinong\\scene0421\\JIM240419\\home\\";
    //std::string outputDir = "E:\\Studyproj\\24Proj\\meinong\\scene0421\\jvyuan_jvli\\20cm\\";
    std::string outputDir = "E:\\Studyproj\\24Proj\\meinong\\scene0421\\jvyuan_jvli\\100cm\\";
    
    std::string img_L_P = "E:\\Studyproj\\24Proj\\meinong\\scene0421\\jvyuan_jvli\\100cm\\img_L.png";
    std::string img_R_P = "E:\\Studyproj\\24Proj\\meinong\\scene0421\\jvyuan_jvli\\100cm\\img_R.png";
    cv::Mat img_L = cv::imread(img_L_P);
    cv::Mat disp_sgm;
    cv::Mat disp_sgm1;
    cv::Mat disp_sgbm_norm;
    cv::Mat disp_sgm_blurred;
    cv::Mat disp_sgm_blurred_norm;
    disp_sgm = stereo_sgm.main(img_L_P, img_R_P, outputDir);
    cv::GaussianBlur(disp_sgm, disp_sgm_blurred, cv::Size(7, 7), 2.5);  // 使用5x5的核和1.5的标准差
    //cv:imwrite(outputDir + "disp_w_mf_11_filling1.tiff",disp_sgm1);
    //disp_sgm = cv::imread(outputDir+"disp_w_mf_11_filling1.tiff");
    //cv::Mat disp_sgm_ROI = reserveROI(disp_sgm,1720,1150);
    double minVal, maxVal;
    cv::minMaxLoc(disp_sgm, &minVal, &maxVal);
    std::cout << "minVal = " << minVal << " maxVal = " << maxVal << std::endl;
    disp_sgm.convertTo(disp_sgbm_norm, CV_8UC1, 255 / (maxVal - minVal));
    cv::minMaxLoc(disp_sgm_blurred, &minVal, &maxVal);
    std::cout << "minVal = " << minVal << " maxVal = " << maxVal << std::endl;
    disp_sgm_blurred.convertTo(disp_sgm_blurred_norm, CV_8UC1, 255 / (maxVal - minVal));

    cv::Mat color_disparity;
    cv::Mat color_disparity_blurred;

    cv::applyColorMap(disp_sgbm_norm, color_disparity, cv::COLORMAP_JET);
    cv::applyColorMap(disp_sgm_blurred_norm, color_disparity_blurred, cv::COLORMAP_JET);
    //cv::imshow("disp_norm", color_disparity);
    //cv::imshow("disp_norm_blurred", color_disparity_blurred);
    //cv::waitKey(0);
    cv::imwrite(outputDir+"color_disparity.png", color_disparity);
    cv::imwrite(outputDir + "color_disparity_blurred.png", color_disparity_blurred);

    cv::Mat PointCloud_blurred =stereo_sgm.Disp2PCloud(disp_sgm_blurred, 800, 120, 960, 600);
    cv::Mat PointCloud = stereo_sgm.Disp2PCloud(disp_sgm, 800, 120, 960, 600);
    FileStorage fs_w("pointCloud.xml", FileStorage::WRITE);
    // 将点云数据保存到文件
    fs_w << "PointCloud" << PointCloud;
    // 释放 FileStorage 对象
    fs_w.release();
    cout << "Point cloud data saved to pointCloud.xml" << endl;

   
    stereo_sgm.Mat2PCL(img_L, PointCloud_blurred, outputDir + "PointCloud_blurred.pcd");
    stereo_sgm.Mat2PCL(img_L, PointCloud, outputDir + "PointCloud.pcd");


    std::cout << "pcd saved" << endl;

}

void stereo_rectify::on_test1_cliked() {
    std::cout << "tset1 go" << endl;
    // 初始化点云对象
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);

    //// 生成随机点云
    //cloud->width = 1000;
    //cloud->height = 1;
    //cloud->is_dense = false;
    //cloud->points.resize(cloud->width * cloud->height);

    //for (auto& point : cloud->points)
    //{
    //    point.x = 1024 * rand() / (RAND_MAX + 1.0f);
    //    point.y = 1024 * rand() / (RAND_MAX + 1.0f);
    //    point.z = 1024 * rand() / (RAND_MAX + 1.0f);
    //}

    //// 保存随机生成的点云
    //pcl::io::savePCDFileASCII("random_cloud.pcd", *cloud);
    //std::cout << "Saved " << cloud->points.size() << " data points to random_cloud.pcd." << std::endl;
    std::string img_L_P = "E:\\Studyproj\\24Proj\\meinong\\scene0421\\jvyuan_jvli\\100cm\\img_L.png";
    cv::Mat img_L = cv::imread(img_L_P);
    FileStorage fs_r("pointCloud.xml", FileStorage::READ);
// 定义一个 Mat 对象来存储读取的数据
    Mat pointCloud_xml;
// 从文件中读取点云数据
    fs_r["PointCloud"] >> pointCloud_xml;
// 释放 FileStorage 对象
    fs_r.release();
    cout << "Point cloud data loaded from pointCloud.xml" << endl;
    stereo_sgm stereo_sgm;
    stereo_sgm.performGF(pointCloud_xml, img_L);
    //FileStorage fs_r("pointCloud.xml", FileStorage::READ);
    //// 定义一个 Mat 对象来存储读取的数据
    //Mat pointCloud_xml;
    //// 从文件中读取点云数据
    //fs_r["PointCloud"] >> pointCloud_xml;
    //// 释放 FileStorage 对象
    //fs_r.release();
    //cout << "Point cloud data loaded from pointCloud.xml" << endl;
    //stereo_sgm stereo_sgm;
    //stereo_sgm.performMLS(pointCloud_xml);

}


cv::Mat stereo_rectify::reserveROI(cv::Mat disp_sgm, int center_width, int center_height) {
    if (!disp_sgm.empty()) {

        // 创建一个与原始图像大小相同的新图像，所有像素值设置为0
        cv::Mat disp_centered = cv::Mat::zeros(disp_sgm.size(), disp_sgm.type());

        // 计算中心区域的 ROI
        int x_offset = (disp_sgm.cols - center_width) / 2;
        int y_offset = (disp_sgm.rows - center_height) / 2;
        cv::Rect center_roi(x_offset, y_offset, center_width, center_height);

        // 将中心区域从原始图像复制到新图像
        disp_sgm(center_roi).copyTo(disp_centered(center_roi));
        return disp_centered;
        // 现在 disp_centered 中心区域是原始图像的中心区域，其余部分为零
    }
    else {
        std::cerr << "Failed to load image." << std::endl;
    }
}
