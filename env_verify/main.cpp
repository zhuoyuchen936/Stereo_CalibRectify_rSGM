#include <QApplication>
#include <QLabel>
#include "opencv2/opencv.hpp"
#include <stdio.h>
#include <cmath>
#include <bitset>
#include <iostream>
// #include <pcl/io/pcd_io.h>
// #include <pcl/point_types.h>


int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // 使用Qt显示文字
    QLabel label("Hello Qt + OpenCV!");
    label.show();
    
    //使用OpenCV读取图像
    //cv::Mat img = cv::imread("../../Image/Left_rec_handby.png");
    cv::Mat img = cv::imread("../../Image/Left_rec_handby.png");
    
    if (!img.empty()) {
        cv::imshow("OpenCV Image", img);
        cv::waitKey(0);
    }
    
    

    // pcl::PointCloud<pcl::PointXYZ> cloud;
 
    // // Fill in the cloud data
    // cloud.width    = 5;
    // cloud.height   = 1;
    // cloud.is_dense = false;
    // cloud.points.resize (cloud.width * cloud.height);
    
    // for (size_t i = 0; i < cloud.points.size (); ++i)
    // {
    //   cloud.points[i].x = 1024 * rand () / (RAND_MAX + 1.0f);
    //   cloud.points[i].y = 1024 * rand () / (RAND_MAX + 1.0f);
    //   cloud.points[i].z = 1024 * rand () / (RAND_MAX + 1.0f);
    // }
  
    // pcl::io::savePCDFileASCII ("test_pcd.pcd", cloud);
    // std::cerr << "Saved " << cloud.points.size () << " data points to test_pcd.pcd." << std::endl;
  
    // for (size_t i = 0; i < cloud.points.size (); ++i)
    //   std::cerr << "    " << cloud.points[i].x << " " << cloud.points[i].y << " " << cloud.points[i].z << std::endl;
  
    return app.exec();
}