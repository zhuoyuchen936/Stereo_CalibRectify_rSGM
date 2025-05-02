#pragma once
#include <opencv2/opencv.hpp>
#include <QObject>

class stereo_sgm : public QObject
{
	Q_OBJECT
public:
	stereo_sgm();
	~stereo_sgm();

	cv::Mat sgbm_main(std::string img_L_paths, std::string img_R_paths);
	cv::Mat main(std::string img_L_paths, std::string img_R_paths, const std::string& outputDir);
	cv::Mat Disp2PCloud(const cv::Mat& disparityMap, float focalLength, float baseline, float cx, float cy);
	void Mat2PCL(const cv::Mat& image, cv::Mat& pointCloud, const std::string& outputFilename);
	void performMLS(cv::Mat& pointCloud);
	void performBF(cv::Mat& pointCloud, cv::Mat& image);
	void performGF(cv::Mat& pointCloud, cv::Mat& image);
};