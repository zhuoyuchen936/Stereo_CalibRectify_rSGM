#include "stereo_rectify.h"
#include <stdio.h>
#include <iostream>
#include <QFileDialog>


using namespace std;
using namespace cv;
namespace fs = std::filesystem;
std::ostringstream ss;
std::string message;
std::ostringstream ss2;
std::string message2;

std::string projpath;
std::vector<cv::Mat> images1;
std::vector<cv::Mat> images2;

cv::Size patternSize(19, 14); // 假设的棋盘格尺寸，例如9x6
std::vector<std::vector<cv::Point3f>> objectPoints_LC; // 3D世界坐标
std::vector<std::vector<cv::Point2f>> imagePoints1_LC, imagePoints2_LC; // 图像坐标
cv::Size imageSize = images1[0].size();
cv::Mat cameraMatrix1_LC, distCoeffs1_LC, cameraMatrix2_LC, distCoeffs2_LC;
std::vector <cv::Mat> rvecs1_LC, tvecs1_LC, rvecs2_LC, tvecs2_LC;
cv::Mat R_LC, T_LC, E_LC, F_LC, R1_LC, R2_LC, P1_LC, P2_LC, Q_LC;

stereo_rectify::stereo_rectify(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    connect(ui.Calib_Image, &QPushButton::clicked, this, &stereo_rectify::on_LeftImage_clicked);
    connect(ui.calibration, &QPushButton::clicked, this, &stereo_rectify::on_Calib_clicked);
    connect(ui.import_paramter, &QPushButton::clicked, this, &stereo_rectify::on_Import_clicked);
    connect(ui.Test_Image, &QPushButton::clicked, this, &stereo_rectify::on_TestImage_clicked);
    connect(ui.rectification, &QPushButton::clicked, this, &stereo_rectify::on_Rectify_clicked);
    ui.progressBar->setValue(0);
}

stereo_rectify::~stereo_rectify()
{}

void stereo_rectify::param_fresh()
{
    ui.Param_Browser->clear();
    ss2 << "Camera Matrix_L: \n" << cameraMatrix1_LC << std::endl;
    message2 = ss2.str();
    ui.Param_Browser->setText(QString::fromStdString(message));
    ss2 << "Distortion Coefficients_L: \n" << distCoeffs1_LC << std::endl;
    message2 = ss2.str();
    ui.Param_Browser->setText(QString::fromStdString(message));
    ss2 << "Camera Matrix_R: \n" << cameraMatrix2_LC << std::endl;
    message2 = ss2.str();
    ui.Param_Browser->setText(QString::fromStdString(message));
    ss2 << "Distortion Coefficients_R: \n" << distCoeffs2_LC << std::endl;
    message2 = ss2.str();
    ui.Param_Browser->setText(QString::fromStdString(message));
    ss2 << "R: \n" << R_LC << std::endl;
    message2 = ss2.str();
    ui.Param_Browser->setText(QString::fromStdString(message));
    ss2 << "T: \n" << T_LC.t() << std::endl;
    message2 = ss2.str();
    ui.Param_Browser->setText(QString::fromStdString(message));
    ss2 << "Q: \n" << Q_LC.t() << std::endl;
    message2 = ss2.str();
    ui.Param_Browser->setText(QString::fromStdString(message));
}

std::vector<cv::Mat> loadImagesFromDirectory(const std::string& directoryPath) {
    std::vector<cv::Mat> images;
    for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
        cv::Mat img = cv::imread(entry.path().string());
        if (img.empty()) {
            ss << "Failed to load image: " << entry.path() << std::endl;
            message = ss.str();
            //ui.textBrowser->setText(QString::fromStdString(message));
            continue;
        }
        else {
            ss << "successed to load image: " << entry.path() << std::endl;
            message = ss.str();
        }
        images.push_back(img);
    }

    return images;
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

void processChessboardImages(const std::vector<cv::Mat>& images1,
    const std::vector<cv::Mat>& images2,
    const cv::Size& patternSize,
    float square_size,
    std::vector<std::vector<cv::Point3f>>& objectPoints,
    std::vector<std::vector<cv::Point2f>>& imagePoints1,
    std::vector<std::vector<cv::Point2f>>& imagePoints2) {
    bool found1, found2;
    std::vector<int> reject_num;
    for (int i = 0; i < images1.size(); i++) {
        std::vector<cv::Point2f> corners1, corners2;
        found1 = findChessboard(images1[i], patternSize, corners1);
        found2 = findChessboard(images2[i], patternSize, corners2);
        if (found1 && found2) {

            /*	cv::Mat images1_t , images2_t;
                images1_t = images1[i];
                images2_t = images2[i];
                cv::drawChessboardCorners(images1_t, patternSize, corners1, found1);
                cv::drawChessboardCorners(images2_t, patternSize, corners2, found2);
                cv::Mat img_pointed = combineImagesSideBySide(images1_t, images2_t);
                cv_show("pointed Image pair", img_pointed);
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
            cv::Mat img_reject(images2[i].rows, images2[i].cols * 2, images2[i].type());
            images1[i].copyTo(img_reject(cv::Rect(0, 0, images2[i].cols, images2[i].rows)));
            images2[i].copyTo(img_reject(cv::Rect(images2[i].cols, 0, images2[i].cols, images2[i].rows)));
            String str = "rejected image pair " + std::to_string(i + 1);
            /*cv::imshow(str, img_reject);
            cv::waitKey(0);*/
            reject_num.push_back(i + 1);
        }

        float progress = (i + 1) * 100.0 / images1.size();
        std::cout << "\rfindChessboard Progress: " << std::fixed << std::setprecision(2) << progress << "%" << std::flush;
    }
    std::cout << "\ndetected " << imagePoints1.size() << " image pairs." << std::endl;
    for (int i = 0; i < reject_num.size(); i++) {
        std::cout << "image pair " << reject_num[i] << " is rejected" << std::endl;
    }
}
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
    std::vector<cv::Mat>& tvecs2
) {

    // 相机1标定
    std::cout << "calibrateCamera1..." << std::endl;
    calibrateSingleCamera(objectPoints, imagePoints1, imageSize, cameraMatrix1, distCoeffs1, rvecs1, tvecs1);
    // 相机2标定
    std::cout << "calibrateCamera2..." << std::endl;
    calibrateSingleCamera(objectPoints, imagePoints2, imageSize, cameraMatrix2, distCoeffs2, rvecs2, tvecs2);

    // 计算相机1重投影误差
    std::vector<float> perViewErrors1;
    computeReprojectionErrors(objectPoints, imagePoints1, rvecs1, tvecs1, cameraMatrix1, distCoeffs1, perViewErrors1);
    // 计算相机2重投影误差
    std::vector<float> perViewErrors2;
    computeReprojectionErrors(objectPoints, imagePoints2, rvecs2, tvecs2, cameraMatrix2, distCoeffs2, perViewErrors2);
    // 设定重投影误差阈值
    float reprojectionErrorThreshold = 1.0; // 例如1像素
    bool iserase = false;
    // 根据重投影误差去除数据组
    for (int i = objectPoints.size() - 1; i >= 0; --i) {
        std::cout << "reprojectionError of image pair " << i << " : " << perViewErrors1[i] << " " << perViewErrors2[i] << std::endl;
        if (perViewErrors1[i] > reprojectionErrorThreshold || perViewErrors2[i] > reprojectionErrorThreshold) {
            objectPoints.erase(objectPoints.begin() + i);
            imagePoints1.erase(imagePoints1.begin() + i);
            imagePoints2.erase(imagePoints2.begin() + i);
            std::cout << "erase image pair " << i << std::endl;
            iserase = true;
        }
    }

    // 使用过滤后的数据组再次标定相机1
    if (!objectPoints.empty()) {
        if (iserase) {
            std::cout << "Re-calibrating Camera 1 with filtered data..." << std::endl;
            calibrateSingleCamera(objectPoints, imagePoints1, imageSize, cameraMatrix1, distCoeffs1, rvecs1, tvecs1);
            std::cout << "Re-calibrating Camera 2 with filtered data..." << std::endl;
            calibrateSingleCamera(objectPoints, imagePoints2, imageSize, cameraMatrix2, distCoeffs2, rvecs2, tvecs2);
        }
        else {
            std::cout << "all the image pairs are maintained..." << std::endl;
        }
    }
    else {
        std::cout << "all the image pairs are filtered..." << std::endl;
        return;
    }

}


void stereoCalibration(
    const std::vector<std::vector<cv::Point3f>>& objectPoints,
    const std::vector<std::vector<cv::Point2f>>& imagePoints1,
    const std::vector<std::vector<cv::Point2f>>& imagePoints2,
    cv::Mat& cameraMatrix1, cv::Mat& distCoeffs1,
    cv::Mat& cameraMatrix2, cv::Mat& distCoeffs2,
    const cv::Size& imageSize, cv::Mat& R, cv::Mat& T, cv::Mat& E, cv::Mat& F) {


    // 双目标定
    std::cout << "stereoCalibrate" << std::endl;
    cv::Mat perViewErrors;

    cv::stereoCalibrate(objectPoints, imagePoints1, imagePoints2,
        cameraMatrix1, distCoeffs1, cameraMatrix2, distCoeffs2,
        imageSize, R, T, E, F, perViewErrors, 0x00100);
    cout << "R_rl before re-stereoCalibrate: " << endl << R << endl;
    cout << "t_rl before re-stereoCalibrate: " << endl << T.t() << endl;


    //cv::stereoCalibrate(objectPoints, imagePoints1, imagePoints2,
    //	cameraMatrix1, distCoeffs1, cameraMatrix2, distCoeffs2,
    //	imageSize, R, T, E, F,cv::CALIB_FIX_INTRINSIC, perViewErrors);

    // 设置重投影误差的阈值
    double threshold = 0.7;

    // 存储重投影误差较小的图像索引
    std::vector<int> goodViews;
    for (int i = 0; i < perViewErrors.rows; ++i) {
        double error = perViewErrors.at<double>(i, 0);
        double error1 = perViewErrors.at<double>(i, 1);
        if ((error < threshold) && (error1 < threshold)) {
            goodViews.push_back(i);
            std::cout << "View " << i << " has low reprojection error: " << error << " and " << error1 << std::endl;
        }
        else {
            std::cout << "View " << i << " has high reprojection error: " << error << " and " << error1 << std::endl;
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
        imageSize, R, T, E, F, perViewErrors, 0x00100);
    // 可以选择使用新的数据集再次进行标定或其他操作
    cout << "R_rl after re-stereoCalibrate: " << endl << R << endl;
    cout << "t_rl after re-stereoCalibrate: " << endl << T.t() << endl;

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

    // 处理folderPath
    // 例如，你可以将路径显示在状态栏或者文本框中
    ui.statusBar->showMessage(folderPath);
    projpath = folderPath.toStdString();
    ui.textBrowser->setText(folderPath);
    std::cout << projpath << std::endl;
    const std::string folder1 = projpath + "/img_L";
    const std::string folder2 = projpath + "/img_R";
    images1 = loadImagesFromDirectory(folder1);
    images2 = loadImagesFromDirectory(folder2);
    ui.textBrowser->setText(QString::fromStdString(message));
    ss << "Loaded " << images1.size() << " images from img_L."<<endl;
    message = ss.str();
    ui.textBrowser->setText(QString::fromStdString(message));
    ss << "Loaded " << images2.size() << " images from img_R." << endl;
    message = ss.str();
    ui.textBrowser->setText(QString::fromStdString(message));
    QPixmap pixmap_L = matToQPixmap(images1[0]);
    QPixmap pixmap_R = matToQPixmap(images2[0]);
    ui.image_L->setPixmap(pixmap_L.scaled(ui.image_L->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui.image_R->setPixmap(pixmap_R.scaled(ui.image_R->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

}
void stereo_rectify::on_Calib_clicked()
{
    bool isLoad = 1;
    int square_size = 15;//Size_of_checkerboard_square mm
    std::cout << "findChessboardLC... " << std::endl;
    processChessboardImages(images1, images2, patternSize, square_size, objectPoints_LC, imagePoints1_LC, imagePoints2_LC);

    std::cout << "calibrateCamerasLC... " << std::endl;
    calibrateCameras(projpath, objectPoints_LC, imagePoints1_LC, imagePoints2_LC, imageSize, cameraMatrix1_LC, distCoeffs1_LC, rvecs1_LC, tvecs1_LC, cameraMatrix2_LC, distCoeffs2_LC, rvecs2_LC, tvecs2_LC);

    std::cout << "stereoCalibrationLC... " << std::endl;
    stereoCalibration(objectPoints_LC, imagePoints1_LC, imagePoints2_LC, cameraMatrix1_LC, distCoeffs1_LC, cameraMatrix2_LC, distCoeffs2_LC,
        imageSize, R_LC, T_LC, E_LC, F_LC);
    std::cout << "save parameter... " << std::endl;
    cv::FileStorage fs(projpath + "opencvStereoParam_LC.yml", cv::FileStorage::WRITE);
    fs << "M1" << cameraMatrix1_LC;
    fs << "D1" << distCoeffs1_LC;
    fs << "imageSize" << imageSize;
    fs << "M2" << cameraMatrix2_LC;
    fs << "D2" << distCoeffs2_LC;
    fs << "R" << R_LC;
    fs << "T" << T_LC;
    fs.release();
    param_fresh();
}

void stereo_rectify::on_Import_clicked()
{
    ss << "Load StereoParam_LC... " << std::endl;
    std::string message = ss.str();
    ui.textBrowser->setText(QString::fromStdString(message));

    cv::FileStorage fs(projpath + "opencvStereoParam_LC.yml", cv::FileStorage::READ); // Replace with your actual file path
    if (!fs.isOpened()) {
        ss << "Failed to open calibration file" << std::endl;
        std::string message = ss.str();
        ui.textBrowser->setText(QString::fromStdString(message));
        return;
    }

    fs["M1"] >> cameraMatrix1_LC;
    fs["D1"] >> distCoeffs1_LC;
    fs["M2"] >> cameraMatrix2_LC;
    fs["D2"] >> distCoeffs2_LC;
    fs["R"] >> R_LC;
    fs["T"] >> T_LC;

    fs.release(); // Close the file
    param_fresh();
}
void stereo_rectify::on_TestImage_clicked()
{
}
void stereo_rectify::on_Rectify_clicked()
{
}