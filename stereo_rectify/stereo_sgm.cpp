// #include <pcl/io/pcd_io.h>
// #include <pcl/point_types.h>
// #include <pcl/kdtree/kdtree_flann.h>
// #include <pcl/surface/mls.h>
// #include <pcl/visualization/cloud_viewer.h>
// #include <pcl/visualization/pcl_visualizer.h>
// #include <pcl/search/kdtree.h>
// #include <pcl/filters/bilateral.h>
// #include <pcl/filters/convolution_3d.h>
// #include <pcl/console/time.h>

#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <algorithm>
#include<bitset>
#include <Windows.h>


#include "stereo_sgm.h"
using namespace cv;
using namespace pcl;
using namespace std;
stereo_sgm::stereo_sgm() {
	// 构造函数实现
}
stereo_sgm::~stereo_sgm() {
	// 析构函数体，哪怕是空的
}



#include"opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include<algorithm>
#include<bitset>
#include <filesystem>


using namespace std;
using namespace cv;
//namespace fs = std::filesystem;

#define BLUR_RADIUS 3
#define PATHS_PER_SCAN 4
#define MAX_SHORT 0xffff //std::numeric_limits<unsigned short>::(max)()
#define SMALL_PENALTY 0.8
#define LARGE_PENALTY 3.2
#define DEBUG false
#define Disparity_Range 256
#define Window_Size 3
#define Groupnumber 4

#define GRAD 32 ;//正则化参数
#define CTg 16 ;//正则化参数

extern int flagg, maxx = 0;

struct path {
	short rowDiff;
	short colDiff;
	short index;
};

void printArray(unsigned short*** array, int rows, int cols, int depth)
{
	for (int d = 0; d < depth; ++d) {
		std::cout << "disparity: " << d << std::endl;
		for (int row = 0; row < rows; ++row) {
			for (int col = 0; col < cols; ++col) {
				std::cout << "\t" << array[row][col][d];
			}
			std::cout << std::endl;
		}
	}
}

//------一、匹配代价计算------
//------①对图像进行CLAHE处理------
//增强图像的对比度同时能够抑制噪声
Mat CLAHE_process(Mat& img) {

	Mat img_CLAHE = Mat(img.rows, img.cols, CV_8UC1, Scalar::all(0));
	Ptr <CLAHE> clahe = createCLAHE();
	clahe->setClipLimit(3);
	clahe->setTilesGridSize(cv::Size(10, 10));
	clahe->apply(img, img_CLAHE);
	return img_CLAHE;
}
//------②进行增强梯度信息处理------
//----------③对图像进行census编码---------------
//          综合图像梯度信息进行census编码
//输入图片应该是梯度增强后的

Mat ProcessImg(Mat& Img)
{
	int64 start;
	start = getTickCount();

	Mat Img_census = Mat(Img.rows, Img.cols, CV_8UC1, Scalar::all(0));
	short center = 0;
	short neighbor = 0;
	int ImgHeight = Img.rows;
	int ImgWidth = Img.cols;
	int hWind = (Window_Size - 1) / 2;
	printf("hwind: %d\n", hWind);
	for (int i = 0; i < ImgHeight - hWind; i++)
	{
		for (int j = 0; j < ImgWidth - hWind; j++)
		{
			center = Img.at<short>(i + hWind, j + hWind);
			int census = 0;

			for (int p = i; p <= i + 2 * hWind; p++)//行
			{
				for (int q = j; q <= j + 2 * hWind; q++)//列
				{
					if (p >= 0 && p < ImgHeight && q >= 0 && q < ImgWidth)
					{

						if (!(p == i + hWind && q == j + hWind))
						{
							//--------- 将二进制数存在变量中-----
							neighbor = Img.at<short>(p, q);

							if (neighbor < center)
							{
								census = census * 2 + 1;//向左移一位，相当于在二进制后面增添0
							}
							else
							{
								census = census * 2;//向左移一位并加一，相当于在二进制后面增添1
							}
						}
					}
				}

			}
			Img_census.at<uchar>(i + hWind, j + hWind) = census;
		}
	}
	/*end = getTickCount();
	cout << "time is = " << end - start << " ms" << endl;*/
	return Img_census;
}

//------------④得到汉明距离---------------
int GetHammingWeight(uchar value)
{
	int num = 0;
	if (value == 0)
		return 0;
	while (value)
	{
		++num;
		value = (value - 1) & value;
	}
	return num;

}


float exponentialCost(float x) {

	float result;

	if (abs(x) >= 32) {
		result = 0.00000000 * abs(x) + 0.00000000;
	}
	else if (0 <= abs(x) < 0.125) {
		result = -0.87500000 * abs(x) + 1.00000000;
	}
	else if (0.125 <= abs(x) < 0.25) {
		result = -0.8125000000 * abs(x) + 0.93750000;
	}
	else if (0.25 <= abs(x) < 0.5) {
		result = -0.68750000 * abs(x) + 0.937500000;
	}
	else if (0.5 <= abs(x) < 1) {
		result = -0.43750000 * abs(x) + 0.81250000;
	}
	else if (1 <= abs(x) < 1.5) {
		result = -0.25000000 * abs(x) + 0.62500000;
	}
	else if (1.5 <= abs(x) < 2) {
		result = -0.1250000 * abs(x) + 0.43750000;
	}
	else if (2 <= abs(x) < 3) {
		result = -0.06250000 * abs(x) + 0.25000000;
	}
	//



	else if (3 <= abs(x) < 4) {
		result = -0.0312500000000 * abs(x) + 0.14062500000;
	}
	else if (4 <= abs(x) < 5) {
		result = 0.0625;
	}
	//6bit-
	else if (5 <= abs(x) < 6) {
		result = 0.015625;
	}
	else if (6 <= abs(x) < 16) {
		result = 0.00000000 * abs(x) + 0.00000000;
	}
	else if (16 <= abs(x) < 32) {
		result = 0.00000000 * abs(x) + 0.00000000;
	}


	return result;
}


//------⑤计算像素对之间的cost值------
//将census 和 grad代价 加权
//正则化参数 GRAD CTg
void calculatePixelCost(Mat& leftimage_grad, Mat& rightimage_grad, Mat& leftimage, Mat& rightimage, Mat& left_grad_x, Mat& left_grad_y, Mat& right_grad_x, Mat& right_grad_y,
	float*** B, int disparityRange, float*** C, int Grad, int Ctg)
{
	Mat Left_census, Right_census;
	Left_census = ProcessImg(leftimage_grad);//处理左图，得到左图的CENSUS图像 Left_census

	Right_census = ProcessImg(rightimage_grad);
	//imshow("Left_census", Left_census);
	//imshow("Right_census", Right_census);

	int ImgHeight = leftimage.rows;
	int ImgWidth = leftimage.cols;

	for (int i = 0; i < ImgHeight; i++)
	{
		for (int j = 0; j < ImgWidth; j++)
		{
			uchar L;
			uchar R1;
			uchar R2;
			uchar R3;
			uchar R4;
			uchar diff1;
			uchar diff2;
			uchar diff3;
			uchar diff4;

			L = Left_census.at<uchar>(i, j);
			for (int k = 0; k < disparityRange / 4; k++)
			{
				int temp = 4 * k;
				int temp1 = 4 * k + 1;
				int temp2 = 4 * k + 2;
				int temp3 = 4 * k + 3;

				int y1 = j - temp;
				int y2 = j - temp1;
				int y3 = j - temp2;
				int y4 = j - temp3;


				if (y4 < 0)
				{
					C[i][j][k] = Window_Size * Window_Size;
				}
				else
				{
					R1 = Right_census.at<uchar>(i, y1);
					R2 = Right_census.at<uchar>(i, y2);
					R3 = Right_census.at<uchar>(i, y3);
					R4 = Right_census.at<uchar>(i, y4);

					diff1 = L ^ R1;
					diff2 = L ^ R2;
					diff3 = L ^ R3;
					diff4 = L ^ R4;

					int hamming[4]{};
					hamming[0] = GetHammingWeight(diff1);
					hamming[1] = GetHammingWeight(diff2);
					hamming[2] = GetHammingWeight(diff3);
					hamming[3] = GetHammingWeight(diff4);





					/*if(hamming[0]>0)
						cout << "hamming[0]" << hamming[0] << endl;
					if (hamming[1] > 0)
						cout << "hamming[1]" << hamming[1] << endl;
					if (hamming[2] > 0)
						cout << "hamming[2]" << hamming[2] << endl;
					if (hamming[3] > 0)
						cout << "hamming[3]" << hamming[3] << endl;*/


					int cost_grad[4]{};
					cost_grad[0] = abs(left_grad_x.at<short>(i, j) - right_grad_x.at<short>(i, y1)) + abs(left_grad_y.at<short>(i, j) - right_grad_y.at<short>(i, y1));
					cost_grad[1] = abs(left_grad_x.at<short>(i, j) - right_grad_x.at<short>(i, y2)) + abs(left_grad_y.at<short>(i, j) - right_grad_y.at<short>(i, y2));
					cost_grad[2] = abs(left_grad_x.at<short>(i, j) - right_grad_x.at<short>(i, y3)) + abs(left_grad_y.at<short>(i, j) - right_grad_y.at<short>(i, y3));
					cost_grad[3] = abs(left_grad_x.at<short>(i, j) - right_grad_x.at<short>(i, y4)) + abs(left_grad_y.at<short>(i, j) - right_grad_y.at<short>(i, y4));

					/*cout << "cost_grad[0]" << cost_grad[0] << endl;
					cout << "cost_grad[1]" << cost_grad[1] << endl;
					cout << "cost_grad[2]" << cost_grad[2] << endl;
					cout << "cost_grad[3]" << cost_grad[3] << endl;
*/


					float cost[4]{};

					float cost_hamming[4];

					switch (hamming[0]) {
					case 0:
						cost_hamming[0] = 1;
						break;
					case 1:
						cost_hamming[0] = 0.9375;
						break;
					case 2:
						cost_hamming[0] = 0.875;
						break;
					case 3:
						cost_hamming[0] = 0.828125;
						break;
					case 4:
						cost_hamming[0] = 0.765625;
						break;
					case 5:
						cost_hamming[0] = 0.71875;
						break;
					case 6:
						cost_hamming[0] = 0.671875;
						break;
					case 7:
						cost_hamming[0] = 0.640625;
						break;
					case 8:
						cost_hamming[0] = 0.59375;
						break;
					case 9:
						cost_hamming[0] = 0.5625;
						break;
					}


					switch (hamming[1]) {
					case 0:
						cost_hamming[1] = 1;
						break;
					case 1:
						cost_hamming[1] = 0.9375;
						break;
					case 2:
						cost_hamming[1] = 0.875;
						break;
					case 3:
						cost_hamming[1] = 0.828125;
						break;
					case 4:
						cost_hamming[1] = 0.765625;
						break;
					case 5:
						cost_hamming[1] = 0.71875;
						break;
					case 6:
						cost_hamming[1] = 0.671875;
						break;
					case 7:
						cost_hamming[1] = 0.640625;
						break;
					case 8:
						cost_hamming[1] = 0.59375;
						break;
					case 9:
						cost_hamming[1] = 0.5625;
						break;
					}


					switch (hamming[2]) {
					case 0:
						cost_hamming[2] = 1;
						break;
					case 1:
						cost_hamming[2] = 0.9375;
						break;
					case 2:
						cost_hamming[2] = 0.875;
						break;
					case 3:
						cost_hamming[2] = 0.828125;
						break;
					case 4:
						cost_hamming[2] = 0.765625;
						break;
					case 5:
						cost_hamming[2] = 0.71875;
						break;
					case 6:
						cost_hamming[2] = 0.671875;
						break;
					case 7:
						cost_hamming[2] = 0.640625;
						break;
					case 8:
						cost_hamming[2] = 0.59375;
						break;
					case 9:
						cost_hamming[2] = 0.5625;
						break;
					}

					switch (hamming[3]) {
					case 0:
						cost_hamming[3] = 1;
						break;
					case 1:
						cost_hamming[3] = 0.9375;
						break;
					case 2:
						cost_hamming[3] = 0.875;
						break;
					case 3:
						cost_hamming[3] = 0.828125;
						break;
					case 4:
						cost_hamming[3] = 0.765625;
						break;
					case 5:
						cost_hamming[3] = 0.71875;
						break;
					case 6:
						cost_hamming[3] = 0.671875;
						break;
					case 7:
						cost_hamming[3] = 0.640625;
						break;
					case 8:
						cost_hamming[3] = 0.59375;
						break;
					case 9:
						cost_hamming[3] = 0.5625;
						break;
					}


					//cost_hamming 代替 exp(float(0 - hamming[0]) / float(Ctg))


					float cost_exponent[4];
					cost_exponent[0] = (float(0 - cost_grad[0]) / float(Grad));
					cost_exponent[1] = (float(0 - cost_grad[1]) / float(Grad));
					cost_exponent[2] = (float(0 - cost_grad[2]) / float(Grad));
					cost_exponent[3] = (float(0 - cost_grad[3]) / float(Grad));

					//////////////////////////////////////////Taylor 展开////////////////////////////////////////////////////////////////
					float cost_1_stage1[4];
					cost_1_stage1[0] = float(cost_exponent[0] * 0.008331298828125) + 0.02117919921875;
					cost_1_stage1[1] = float(cost_exponent[1] * 0.008331298828125) + 0.02117919921875;
					cost_1_stage1[2] = float(cost_exponent[2] * 0.008331298828125) + 0.02117919921875;
					cost_1_stage1[3] = float(cost_exponent[3] * 0.008331298828125) + 0.02117919921875;

					float cost_1_stage2[4];
					cost_1_stage2[0] = float(cost_exponent[0] * cost_1_stage1[0]) + 0.1034698486328125;
					cost_1_stage2[1] = float(cost_exponent[1] * cost_1_stage1[1]) + 0.1034698486328125;
					cost_1_stage2[2] = float(cost_exponent[2] * cost_1_stage1[2]) + 0.1034698486328125;
					cost_1_stage2[3] = float(cost_exponent[3] * cost_1_stage1[3]) + 0.1034698486328125;

					float cost_1_stage3[4];
					cost_1_stage3[0] = float(cost_exponent[0] * cost_1_stage2[0]) + 0.3025970458984375;
					cost_1_stage3[1] = float(cost_exponent[1] * cost_1_stage2[1]) + 0.3025970458984375;
					cost_1_stage3[2] = float(cost_exponent[2] * cost_1_stage2[2]) + 0.3025970458984375;
					cost_1_stage3[3] = float(cost_exponent[3] * cost_1_stage2[3]) + 0.3025970458984375;

					float cost_1_stage4[4];
					cost_1_stage4[0] = float(cost_exponent[0] * cost_1_stage3[0]) + 0.6065826416015625;
					cost_1_stage4[1] = float(cost_exponent[1] * cost_1_stage3[1]) + 0.6065826416015625;
					cost_1_stage4[2] = float(cost_exponent[2] * cost_1_stage3[2]) + 0.6065826416015625;
					cost_1_stage4[3] = float(cost_exponent[3] * cost_1_stage3[3]) + 0.6065826416015625;

					float cost_1_stage5[4];
					cost_1_stage5[0] = float(cost_exponent[0] * cost_1_stage4[0]) + 0.6065216064453125;
					cost_1_stage5[1] = float(cost_exponent[1] * cost_1_stage4[1]) + 0.6065216064453125;
					cost_1_stage5[2] = float(cost_exponent[2] * cost_1_stage4[2]) + 0.6065216064453125;
					cost_1_stage5[3] = float(cost_exponent[3] * cost_1_stage4[3]) + 0.6065216064453125;

					float cost_1[4];

					/*cost_1[0] = cost_1_stage5[0] * 1.648712158203125 ;
					cost_1[1] = cost_1_stage5[1] * 1.648712158203125 ;
					cost_1[2] = cost_1_stage5[2] * 1.648712158203125 ;
					cost_1[3] = cost_1_stage5[3] * 1.648712158203125 ;*/
					/////////////////////////////////////////////////////////////taylor expansion////////////////////////////////////////////////////////////////////////////


					//interpolation
					cost_1[0] = exponentialCost(cost_exponent[0]);
					cost_1[1] = exponentialCost(cost_exponent[1]);
					cost_1[2] = exponentialCost(cost_exponent[2]);
					cost_1[3] = exponentialCost(cost_exponent[3]);


					int bit = 3;
					float cost_temp[4];
					cost_temp[0] = 2 - cost_1[0] - cost_hamming[0];
					cost_temp[1] = 2 - cost_1[1] - cost_hamming[1];
					cost_temp[2] = 2 - cost_1[2] - cost_hamming[2];
					cost_temp[3] = 2 - cost_1[3] - cost_hamming[3];

					cost[0] = float(int(cost_temp[0] * (pow(2, bit)))) / (pow(2, bit));
					cost[1] = float(int(cost_temp[1] * (pow(2, bit)))) / (pow(2, bit));
					cost[2] = float(int(cost_temp[2] * (pow(2, bit)))) / (pow(2, bit));
					cost[3] = float(int(cost_temp[3] * (pow(2, bit)))) / (pow(2, bit));



					////////////////////////////////////////////////////////原始计算公式///////////////////////////////////////////////////////////////////////////////////////////////////////
					/*cost[0] = 2 - exp(float(0 - cost_grad[0]) / float(Grad)) - exp(float(0 - hamming[0]) / float(Ctg));
					cost[1] = 2 - exp(float(0 - cost_grad[1]) / float(Grad)) - exp(float(0 - hamming[1]) / float(Ctg));
					cost[2] = 2 - exp(float(0 - cost_grad[2]) / float(Grad)) - exp(float(0 - hamming[2]) / float(Ctg));
					cost[3] = 2 - exp(float(0 - cost_grad[3]) / float(Grad)) - exp(float(0 - hamming[3]) / float(Ctg));
*/
////////////////////////////////////////////////////////原始计算公式///////////////////////////////////////////////////////////////////////////////////////////////////////

					// origin census
					/*cost[0] = GetHammingWeight(diff1);
					cost[1] = GetHammingWeight(diff2);
					cost[2] = GetHammingWeight(diff3);
					cost[3] = GetHammingWeight(diff4);*/






					//////////////////////////////////////////////////////////////指数分为整数和小数，移位操作  方法1 舍弃///////////////////////////////////////////////////////////////////////////
					//float cost_grad_DivGrad[4];
					//cost_grad_DivGrad[0] = cost_grad[0] / float(Grad);
					//cost_grad_DivGrad[1] = cost_grad[1] / float(Grad);
					//cost_grad_DivGrad[2] = cost_grad[2] / float(Grad);
					//cost_grad_DivGrad[3] = cost_grad[3] / float(Grad);

					//float cost_grad_DivGrad_MulLog2e[4];
					//cost_grad_DivGrad_MulLog2e[0] = cost_grad_DivGrad[0] * 1.4375;
					//cost_grad_DivGrad_MulLog2e[1] = cost_grad_DivGrad[1] * 1.4375;
					//cost_grad_DivGrad_MulLog2e[2] = cost_grad_DivGrad[2] * 1.4375;
					//cost_grad_DivGrad_MulLog2e[3] = cost_grad_DivGrad[3] * 1.4375;


					////2^(-x)     
					////x=u+v u是整数部分，v是小数部分
					//int u[4];
					//u[0] = int(cost_grad_DivGrad_MulLog2e[0]);
					//u[1] = int(cost_grad_DivGrad_MulLog2e[1]);
					//u[2] = int(cost_grad_DivGrad_MulLog2e[2]);
					//u[3] = int(cost_grad_DivGrad_MulLog2e[3]);

					//float v[4];
					//v[0] = cost_grad_DivGrad_MulLog2e[0] - u[0];
					//v[1] = cost_grad_DivGrad_MulLog2e[1] - u[1];
					//v[2] = cost_grad_DivGrad_MulLog2e[2] - u[2];
					//v[3] = cost_grad_DivGrad_MulLog2e[3] - u[3];

					//if(u[0]>32)
					//	cout << "u[0]=" << u[0]<<endl;
					//if (u[1] > 32)
					//	cout << "u[1]=" << u[1]<<endl;
					//if (u[2] > 32)
					//	cout << "u[2]=" << u[2]<<endl;
					//if (u[3] > 32)
					//	cout << "u[3]=" << u[3]<<endl;

					//
					////2^(-v)
					//float NegV2[4];
					//NegV2[0] = 1 - 0.6875 * v[0];
					//NegV2[1] = 1 - 0.6875 * v[1];
					//NegV2[2] = 1 - 0.6875 * v[2];
					//NegV2[3] = 1 - 0.6875 * v[3];

					//float cost_GRAD[4];
					//cost_GRAD[0] = NegV2[0] * (2 ^ (-u[0]));
					//cost_GRAD[1] = NegV2[1] * (2 ^ (-u[1]));
					//cost_GRAD[2] = NegV2[2] * (2 ^ (-u[2]));
					//cost_GRAD[3] = NegV2[3] * (2 ^ (-u[3]));

					////保留小数位

					//float cost_origin[4];

					//cost_origin[0] = 2 - cost_GRAD[0] - cost_hamming[0];
					//cost_origin[1] = 2 - cost_GRAD[1] - cost_hamming[1];
					//cost_origin[2] = 2 - cost_GRAD[2] - cost_hamming[2];
					//cost_origin[3] = 2 - cost_GRAD[3] - cost_hamming[3];


					//int demical_bit = 32;
					//int cost_2n[4];
					//cost_2n[0] = int(cost_origin[0] * (2 ^ (demical_bit)));
					//cost_2n[1] = int(cost_origin[1] * (2 ^ (demical_bit)));
					//cost_2n[2] = int(cost_origin[2] * (2 ^ (demical_bit)));
					//cost_2n[3] = int(cost_origin[3] * (2 ^ (demical_bit)));



					//cost[0] = cost_2n[0] / (2 ^ (demical_bit));
					//cost[1] = cost_2n[1] / (2 ^ (demical_bit));
					//cost[2] = cost_2n[2] / (2 ^ (demical_bit));
					//cost[3] = cost_2n[3] / (2 ^ (demical_bit));


					/*float cost_temp[4];
					cost_temp[0] = 2 - exp(float(0 - cost_grad[0]) / float(Grad)) - exp(float(0 - hamming[0]) / float(Ctg));
					cost_temp[1] = 2 - exp(float(0 - cost_grad[1]) / float(Grad)) - exp(float(0 - hamming[1]) / float(Ctg));
					cost_temp[2] = 2 - exp(float(0 - cost_grad[2]) / float(Grad)) - exp(float(0 - hamming[2]) / float(Ctg));
					cost_temp[3] = 2 - exp(float(0 - cost_grad[3]) / float(Grad)) - exp(float(0 - hamming[3]) / float(Ctg));*/



					/*cout << "cost_grad[0]" << exp(float(0 - cost_grad[0]) / float(Grad)) << endl;
					 cout << "cost_grad[1]" << exp(float(0 - cost_grad[1]) / float(Grad)) << endl;
					 cout << "cost_grad[2]" << exp(float(0 - cost_grad[2]) / float(Grad)) << endl;
					 cout << "cost_grad[3]" << exp(float(0 - cost_grad[3]) / float(Grad))<< endl;*/


					 /*int cost_bit =9;

					 cost[0] = float(int(cost_temp[0] * (2 ^ (cost_bit)))) / (2 ^ (cost_bit));
					 cost[1] = float(int(cost_temp[1] * (2 ^ (cost_bit))))/ (2 ^ (cost_bit));
					 cost[2] = float(int(cost_temp[2] * (2 ^ (cost_bit)))) / (2 ^ (cost_bit));
					 cost[3] = float(int(cost_temp[3] * (2 ^ (cost_bit)))) / (2 ^ (cost_bit));*/

					 //////////////////////////////////////////////////////////////指数分为整数和小数，移位操作  方法1 舍弃///////////////////////////////////////////////////////////////////////////


					 /// save the prefer position
					float min = cost[0];
					int mindex = 0;
					for (int i = 1; i < 4; i++) {
						if (min > cost[i]) {
							min = cost[i];
							mindex = i;
						}
					}

					B[i][j][k] = mindex;
					//归一化
					//C[i][j][k] = 2 - exp((-1) * float(cost_grad[mindex]) / float(Grad)) - exp((-1) * float(hamming[mindex]) / float(Ctg));
					C[i][j][k] = min;



					//保留整数小数位 
					/*stringstream ss;
					ss.setf(ios::fixed);
					ss.precision(2);
					ss << min;
					C[i][j][k] = stof(ss.str().c_str());*/


				}
			}
		}
	}

}

void calculatePixelCost2(Mat& leftimage_grad, Mat& rightimage_grad, Mat& leftimage, Mat& rightimage, Mat& left_grad_x, Mat& left_grad_y, Mat& right_grad_x, Mat& right_grad_y,
	float*** B, int disparityRange, float*** C, int Grad, int Ctg)
{
	Mat Left_census, Right_census;
	Left_census = ProcessImg(leftimage_grad);//      ͼ   õ   ͼ  CENSUSͼ   Left_census

	Right_census = ProcessImg(rightimage_grad);
	//imshow("Left_census", Left_census);
	//imshow("Right_census", Right_census);

	int ImgHeight = leftimage.rows;
	int ImgWidth = leftimage.cols;

	int height = 0;
	int width = 0;
	for (int i = 0; i < ImgHeight; i = i + 2)
	{
		for (int j = 0; j < ImgWidth; j = j + 2)
		{
			uchar L1;
			uchar L2;
			uchar L3;
			uchar L4;

			uchar R1;
			uchar R2;
			uchar R3;
			uchar R4;
			uchar diff1;
			uchar diff2;
			uchar diff3;
			uchar diff4;

			L1 = Left_census.at<uchar>(i, j);
			L2 = Left_census.at<uchar>(i + 1, j);
			L3 = Left_census.at<uchar>(i, j + 1);
			L4 = Left_census.at<uchar>(i + 1, j + 1);
			for (int k = 0; k < disparityRange; k++)
			{
				if (((i + 1) > ImgHeight - 1) || ((j + 1) > ImgWidth - 1) || ((j - k) < 0))
				{
					C[height][width][k] = Window_Size * Window_Size;
					//C[height][width][k] = 0;
				}
				else
				{
					R1 = Right_census.at<uchar>(i, j - k);
					R2 = Right_census.at<uchar>(i + 1, j - k);
					R3 = Right_census.at<uchar>(i, j + 1 - k);
					R4 = Right_census.at<uchar>(i + 1, j + 1 - k);

					diff1 = L1 ^ R1;
					diff2 = L2 ^ R2;
					diff3 = L3 ^ R3;
					diff4 = L4 ^ R4;

					int hamming[4]{};
					hamming[0] = GetHammingWeight(diff1);
					hamming[1] = GetHammingWeight(diff2);
					hamming[2] = GetHammingWeight(diff3);
					hamming[3] = GetHammingWeight(diff4);

					/*if(hamming[0]>0)
						cout << "hamming[0]" << hamming[0] << endl;
					if (hamming[1] > 0)
						cout << "hamming[1]" << hamming[1] << endl;
					if (hamming[2] > 0)
						cout << "hamming[2]" << hamming[2] << endl;
					if (hamming[3] > 0)
						cout << "hamming[3]" << hamming[3] << endl;*/


					int cost_grad[4]{};
					cost_grad[0] = abs(left_grad_x.at<short>(i, j) - right_grad_x.at<short>(i, j - k)) + abs(left_grad_y.at<short>(i, j) - right_grad_y.at<short>(i, j - k));
					cost_grad[1] = abs(left_grad_x.at<short>(i, j) - right_grad_x.at<short>(i + 1, j - k)) + abs(left_grad_y.at<short>(i, j) - right_grad_y.at<short>(i + 1, j - k));
					cost_grad[2] = abs(left_grad_x.at<short>(i, j) - right_grad_x.at<short>(i, j + 1 - k)) + abs(left_grad_y.at<short>(i, j) - right_grad_y.at<short>(i, j + 1 - k));
					cost_grad[3] = abs(left_grad_x.at<short>(i, j) - right_grad_x.at<short>(i + 1, j + 1 - k)) + abs(left_grad_y.at<short>(i, j) - right_grad_y.at<short>(i + 1, j + 1 - k));

					/*cout << "cost_grad[0]" << cost_grad[0] << endl;
					cout << "cost_grad[1]" << cost_grad[1] << endl;
					cout << "cost_grad[2]" << cost_grad[2] << endl;
					cout << "cost_grad[3]" << cost_grad[3] << endl;
*/


					float cost[4]{};

					float cost_hamming[4];

					switch (hamming[0]) {
					case 0:
						cost_hamming[0] = 1;
						break;
					case 1:
						cost_hamming[0] = 0.9375;
						break;
					case 2:
						cost_hamming[0] = 0.875;
						break;
					case 3:
						cost_hamming[0] = 0.828125;
						break;
					case 4:
						cost_hamming[0] = 0.765625;
						break;
					case 5:
						cost_hamming[0] = 0.71875;
						break;
					case 6:
						cost_hamming[0] = 0.671875;
						break;
					case 7:
						cost_hamming[0] = 0.640625;
						break;
					case 8:
						cost_hamming[0] = 0.59375;
						break;
					case 9:
						cost_hamming[0] = 0.5625;
						break;
					}


					switch (hamming[1]) {
					case 0:
						cost_hamming[1] = 1;
						break;
					case 1:
						cost_hamming[1] = 0.9375;
						break;
					case 2:
						cost_hamming[1] = 0.875;
						break;
					case 3:
						cost_hamming[1] = 0.828125;
						break;
					case 4:
						cost_hamming[1] = 0.765625;
						break;
					case 5:
						cost_hamming[1] = 0.71875;
						break;
					case 6:
						cost_hamming[1] = 0.671875;
						break;
					case 7:
						cost_hamming[1] = 0.640625;
						break;
					case 8:
						cost_hamming[1] = 0.59375;
						break;
					case 9:
						cost_hamming[1] = 0.5625;
						break;
					}


					switch (hamming[2]) {
					case 0:
						cost_hamming[2] = 1;
						break;
					case 1:
						cost_hamming[2] = 0.9375;
						break;
					case 2:
						cost_hamming[2] = 0.875;
						break;
					case 3:
						cost_hamming[2] = 0.828125;
						break;
					case 4:
						cost_hamming[2] = 0.765625;
						break;
					case 5:
						cost_hamming[2] = 0.71875;
						break;
					case 6:
						cost_hamming[2] = 0.671875;
						break;
					case 7:
						cost_hamming[2] = 0.640625;
						break;
					case 8:
						cost_hamming[2] = 0.59375;
						break;
					case 9:
						cost_hamming[2] = 0.5625;
						break;
					}

					switch (hamming[3]) {
					case 0:
						cost_hamming[3] = 1;
						break;
					case 1:
						cost_hamming[3] = 0.9375;
						break;
					case 2:
						cost_hamming[3] = 0.875;
						break;
					case 3:
						cost_hamming[3] = 0.828125;
						break;
					case 4:
						cost_hamming[3] = 0.765625;
						break;
					case 5:
						cost_hamming[3] = 0.71875;
						break;
					case 6:
						cost_hamming[3] = 0.671875;
						break;
					case 7:
						cost_hamming[3] = 0.640625;
						break;
					case 8:
						cost_hamming[3] = 0.59375;
						break;
					case 9:
						cost_hamming[3] = 0.5625;
						break;
					}


					//cost_hamming      exp(float(0 - hamming[0]) / float(Ctg))


					float cost_exponent[4];
					cost_exponent[0] = (float(0 - cost_grad[0]) / float(Grad));
					cost_exponent[1] = (float(0 - cost_grad[1]) / float(Grad));
					cost_exponent[2] = (float(0 - cost_grad[2]) / float(Grad));
					cost_exponent[3] = (float(0 - cost_grad[3]) / float(Grad));

					//////////////////////////////////////////Taylor չ  ////////////////////////////////////////////////////////////////
					float cost_1_stage1[4];
					cost_1_stage1[0] = float(cost_exponent[0] * 0.008331298828125) + 0.02117919921875;
					cost_1_stage1[1] = float(cost_exponent[1] * 0.008331298828125) + 0.02117919921875;
					cost_1_stage1[2] = float(cost_exponent[2] * 0.008331298828125) + 0.02117919921875;
					cost_1_stage1[3] = float(cost_exponent[3] * 0.008331298828125) + 0.02117919921875;

					float cost_1_stage2[4];
					cost_1_stage2[0] = float(cost_exponent[0] * cost_1_stage1[0]) + 0.1034698486328125;
					cost_1_stage2[1] = float(cost_exponent[1] * cost_1_stage1[1]) + 0.1034698486328125;
					cost_1_stage2[2] = float(cost_exponent[2] * cost_1_stage1[2]) + 0.1034698486328125;
					cost_1_stage2[3] = float(cost_exponent[3] * cost_1_stage1[3]) + 0.1034698486328125;

					float cost_1_stage3[4];
					cost_1_stage3[0] = float(cost_exponent[0] * cost_1_stage2[0]) + 0.3025970458984375;
					cost_1_stage3[1] = float(cost_exponent[1] * cost_1_stage2[1]) + 0.3025970458984375;
					cost_1_stage3[2] = float(cost_exponent[2] * cost_1_stage2[2]) + 0.3025970458984375;
					cost_1_stage3[3] = float(cost_exponent[3] * cost_1_stage2[3]) + 0.3025970458984375;

					float cost_1_stage4[4];
					cost_1_stage4[0] = float(cost_exponent[0] * cost_1_stage3[0]) + 0.6065826416015625;
					cost_1_stage4[1] = float(cost_exponent[1] * cost_1_stage3[1]) + 0.6065826416015625;
					cost_1_stage4[2] = float(cost_exponent[2] * cost_1_stage3[2]) + 0.6065826416015625;
					cost_1_stage4[3] = float(cost_exponent[3] * cost_1_stage3[3]) + 0.6065826416015625;

					float cost_1_stage5[4];
					cost_1_stage5[0] = float(cost_exponent[0] * cost_1_stage4[0]) + 0.6065216064453125;
					cost_1_stage5[1] = float(cost_exponent[1] * cost_1_stage4[1]) + 0.6065216064453125;
					cost_1_stage5[2] = float(cost_exponent[2] * cost_1_stage4[2]) + 0.6065216064453125;
					cost_1_stage5[3] = float(cost_exponent[3] * cost_1_stage4[3]) + 0.6065216064453125;

					float cost_1[4];

					/*cost_1[0] = cost_1_stage5[0] * 1.648712158203125 ;
					cost_1[1] = cost_1_stage5[1] * 1.648712158203125 ;
					cost_1[2] = cost_1_stage5[2] * 1.648712158203125 ;
					cost_1[3] = cost_1_stage5[3] * 1.648712158203125 ;*/
					/////////////////////////////////////////////////////////////taylor expansion////////////////////////////////////////////////////////////////////////////


					//interpolation
					cost_1[0] = exponentialCost(cost_exponent[0]);
					cost_1[1] = exponentialCost(cost_exponent[1]);
					cost_1[2] = exponentialCost(cost_exponent[2]);
					cost_1[3] = exponentialCost(cost_exponent[3]);


					int bit = 3;
					float cost_temp[4];
					cost_temp[0] = 2 - cost_1[0] - cost_hamming[0];
					cost_temp[1] = 2 - cost_1[1] - cost_hamming[1];
					cost_temp[2] = 2 - cost_1[2] - cost_hamming[2];
					cost_temp[3] = 2 - cost_1[3] - cost_hamming[3];

					cost[0] = float(int(cost_temp[0] * (pow(2, bit)))) / (pow(2, bit));
					cost[1] = float(int(cost_temp[1] * (pow(2, bit)))) / (pow(2, bit));
					cost[2] = float(int(cost_temp[2] * (pow(2, bit)))) / (pow(2, bit));
					cost[3] = float(int(cost_temp[3] * (pow(2, bit)))) / (pow(2, bit));



					/// save the prefer position
					float ave = (cost[0] + cost[1] + cost[2] + cost[3]) / 4;

					B[height][width][k] = 0;
					//  һ  
					//C[i][j][k] = 2 - exp((-1) * float(cost_grad[mindex]) / float(Grad)) - exp((-1) * float(hamming[mindex]) / float(Ctg));
					C[height][width][k] = ave;



					//        С  λ 
					/*stringstream ss;
					ss.setf(ios::fixed);
					ss.precision(2);
					ss << min;
					C[i][j][k] = stof(ss.str().c_str());*/


				}
			}
			width++;
		}
		width = 0;
		height++;
	}
}
void calculatePixelCost3(Mat& leftimage_grad, Mat& rightimage_grad, Mat& leftimage, Mat& rightimage, Mat& left_grad_x, Mat& left_grad_y, Mat& right_grad_x, Mat& right_grad_y,
	float*** B, int disparityRange, float*** C, int Grad, int Ctg)
{
	Mat Left_census, Right_census;
	Left_census = ProcessImg(leftimage_grad);//      ͼ   õ   ͼ  CENSUSͼ   Left_census

	Right_census = ProcessImg(rightimage_grad);
	//imshow("Left_census", Left_census);
	//imshow("Right_census", Right_census);

	int ImgHeight = leftimage.rows;
	int ImgWidth = leftimage.cols;

	int height = 0;
	int width = 0;
	for (int i = 0; i < ImgHeight - 1; i++)
	{
		for (int j = 0; j < ImgWidth - 1; j = j + 2)
		{
			uchar L1;
			uchar L2;
			uchar L3;
			uchar L4;

			uchar R1;
			uchar R2;
			uchar R3;
			uchar R4;
			uchar diff1;
			uchar diff2;
			uchar diff3;
			uchar diff4;

			int x1 = i;
			int y1 = j;
			if ((x1 > ImgHeight - 2) || (y1 > ImgWidth - 2)) {
				x1 = ImgHeight - 2;
				y1 = ImgWidth - 2;
			}
			L1 = Left_census.at<uchar>(x1, y1);
			L2 = Left_census.at<uchar>(x1 + 1, y1);
			L3 = Left_census.at<uchar>(x1, y1 + 1);
			L4 = Left_census.at<uchar>(x1 + 1, y1 + 1);
			for (int k = 0; k < disparityRange / 4; k++)
			{
				float cost_ave[4]{};
				for (int x = 0; x < 4; x++)
				{
					int temp = 4 * k + x;
					int y = j - temp;
					if (((i + 1) > ImgHeight - 1) || ((y + 1) > ImgWidth - 1) || ((y) < 0))
					{
						cost_ave[x] = Window_Size * Window_Size;
						//C[height][width][k] = 0;
					}
					else
					{
						R1 = Right_census.at<uchar>(i, y);
						R2 = Right_census.at<uchar>(i + 1, y);
						R3 = Right_census.at<uchar>(i, y + 1);
						R4 = Right_census.at<uchar>(i + 1, y + 1);

						diff1 = L1 ^ R1;
						diff2 = L2 ^ R2;
						diff3 = L3 ^ R3;
						diff4 = L4 ^ R4;

						int hamming[4]{};
						hamming[0] = GetHammingWeight(diff1);
						hamming[1] = GetHammingWeight(diff2);
						hamming[2] = GetHammingWeight(diff3);
						hamming[3] = GetHammingWeight(diff4);

						/*if(hamming[0]>0)
							cout << "hamming[0]" << hamming[0] << endl;
						if (hamming[1] > 0)
							cout << "hamming[1]" << hamming[1] << endl;
						if (hamming[2] > 0)
							cout << "hamming[2]" << hamming[2] << endl;
						if (hamming[3] > 0)
							cout << "hamming[3]" << hamming[3] << endl;*/


						int cost_grad[4]{};
						cost_grad[0] = abs(left_grad_x.at<short>(i, j) - right_grad_x.at<short>(i, y)) + abs(left_grad_y.at<short>(i, j) - right_grad_y.at<short>(i, y));
						cost_grad[1] = abs(left_grad_x.at<short>(i, j) - right_grad_x.at<short>(i + 1, y)) + abs(left_grad_y.at<short>(i, j) - right_grad_y.at<short>(i + 1, y));
						cost_grad[2] = abs(left_grad_x.at<short>(i, j) - right_grad_x.at<short>(i, y + 1)) + abs(left_grad_y.at<short>(i, j) - right_grad_y.at<short>(i, y + 1));
						cost_grad[3] = abs(left_grad_x.at<short>(i, j) - right_grad_x.at<short>(i + 1, y + 1)) + abs(left_grad_y.at<short>(i, j) - right_grad_y.at<short>(i + 1, y + 1));

						/*cout << "cost_grad[0]" << cost_grad[0] << endl;
						cout << "cost_grad[1]" << cost_grad[1] << endl;
						cout << "cost_grad[2]" << cost_grad[2] << endl;
						cout << "cost_grad[3]" << cost_grad[3] << endl;
	*/


						float cost[4]{};

						float cost_hamming[4];

						switch (hamming[0]) {
						case 0:
							cost_hamming[0] = 1;
							break;
						case 1:
							cost_hamming[0] = 0.9375;
							break;
						case 2:
							cost_hamming[0] = 0.875;
							break;
						case 3:
							cost_hamming[0] = 0.828125;
							break;
						case 4:
							cost_hamming[0] = 0.765625;
							break;
						case 5:
							cost_hamming[0] = 0.71875;
							break;
						case 6:
							cost_hamming[0] = 0.671875;
							break;
						case 7:
							cost_hamming[0] = 0.640625;
							break;
						case 8:
							cost_hamming[0] = 0.59375;
							break;
						case 9:
							cost_hamming[0] = 0.5625;
							break;
						}


						switch (hamming[1]) {
						case 0:
							cost_hamming[1] = 1;
							break;
						case 1:
							cost_hamming[1] = 0.9375;
							break;
						case 2:
							cost_hamming[1] = 0.875;
							break;
						case 3:
							cost_hamming[1] = 0.828125;
							break;
						case 4:
							cost_hamming[1] = 0.765625;
							break;
						case 5:
							cost_hamming[1] = 0.71875;
							break;
						case 6:
							cost_hamming[1] = 0.671875;
							break;
						case 7:
							cost_hamming[1] = 0.640625;
							break;
						case 8:
							cost_hamming[1] = 0.59375;
							break;
						case 9:
							cost_hamming[1] = 0.5625;
							break;
						}


						switch (hamming[2]) {
						case 0:
							cost_hamming[2] = 1;
							break;
						case 1:
							cost_hamming[2] = 0.9375;
							break;
						case 2:
							cost_hamming[2] = 0.875;
							break;
						case 3:
							cost_hamming[2] = 0.828125;
							break;
						case 4:
							cost_hamming[2] = 0.765625;
							break;
						case 5:
							cost_hamming[2] = 0.71875;
							break;
						case 6:
							cost_hamming[2] = 0.671875;
							break;
						case 7:
							cost_hamming[2] = 0.640625;
							break;
						case 8:
							cost_hamming[2] = 0.59375;
							break;
						case 9:
							cost_hamming[2] = 0.5625;
							break;
						}

						switch (hamming[3]) {
						case 0:
							cost_hamming[3] = 1;
							break;
						case 1:
							cost_hamming[3] = 0.9375;
							break;
						case 2:
							cost_hamming[3] = 0.875;
							break;
						case 3:
							cost_hamming[3] = 0.828125;
							break;
						case 4:
							cost_hamming[3] = 0.765625;
							break;
						case 5:
							cost_hamming[3] = 0.71875;
							break;
						case 6:
							cost_hamming[3] = 0.671875;
							break;
						case 7:
							cost_hamming[3] = 0.640625;
							break;
						case 8:
							cost_hamming[3] = 0.59375;
							break;
						case 9:
							cost_hamming[3] = 0.5625;
							break;
						}


						//cost_hamming      exp(float(0 - hamming[0]) / float(Ctg))


						float cost_exponent[4];
						cost_exponent[0] = (float(0 - cost_grad[0]) / float(Grad));
						cost_exponent[1] = (float(0 - cost_grad[1]) / float(Grad));
						cost_exponent[2] = (float(0 - cost_grad[2]) / float(Grad));
						cost_exponent[3] = (float(0 - cost_grad[3]) / float(Grad));

						//////////////////////////////////////////Taylor չ  ////////////////////////////////////////////////////////////////
						float cost_1_stage1[4];
						cost_1_stage1[0] = float(cost_exponent[0] * 0.008331298828125) + 0.02117919921875;
						cost_1_stage1[1] = float(cost_exponent[1] * 0.008331298828125) + 0.02117919921875;
						cost_1_stage1[2] = float(cost_exponent[2] * 0.008331298828125) + 0.02117919921875;
						cost_1_stage1[3] = float(cost_exponent[3] * 0.008331298828125) + 0.02117919921875;

						float cost_1_stage2[4];
						cost_1_stage2[0] = float(cost_exponent[0] * cost_1_stage1[0]) + 0.1034698486328125;
						cost_1_stage2[1] = float(cost_exponent[1] * cost_1_stage1[1]) + 0.1034698486328125;
						cost_1_stage2[2] = float(cost_exponent[2] * cost_1_stage1[2]) + 0.1034698486328125;
						cost_1_stage2[3] = float(cost_exponent[3] * cost_1_stage1[3]) + 0.1034698486328125;

						float cost_1_stage3[4];
						cost_1_stage3[0] = float(cost_exponent[0] * cost_1_stage2[0]) + 0.3025970458984375;
						cost_1_stage3[1] = float(cost_exponent[1] * cost_1_stage2[1]) + 0.3025970458984375;
						cost_1_stage3[2] = float(cost_exponent[2] * cost_1_stage2[2]) + 0.3025970458984375;
						cost_1_stage3[3] = float(cost_exponent[3] * cost_1_stage2[3]) + 0.3025970458984375;

						float cost_1_stage4[4];
						cost_1_stage4[0] = float(cost_exponent[0] * cost_1_stage3[0]) + 0.6065826416015625;
						cost_1_stage4[1] = float(cost_exponent[1] * cost_1_stage3[1]) + 0.6065826416015625;
						cost_1_stage4[2] = float(cost_exponent[2] * cost_1_stage3[2]) + 0.6065826416015625;
						cost_1_stage4[3] = float(cost_exponent[3] * cost_1_stage3[3]) + 0.6065826416015625;

						float cost_1_stage5[4];
						cost_1_stage5[0] = float(cost_exponent[0] * cost_1_stage4[0]) + 0.6065216064453125;
						cost_1_stage5[1] = float(cost_exponent[1] * cost_1_stage4[1]) + 0.6065216064453125;
						cost_1_stage5[2] = float(cost_exponent[2] * cost_1_stage4[2]) + 0.6065216064453125;
						cost_1_stage5[3] = float(cost_exponent[3] * cost_1_stage4[3]) + 0.6065216064453125;

						float cost_1[4];

						/*cost_1[0] = cost_1_stage5[0] * 1.648712158203125 ;
						cost_1[1] = cost_1_stage5[1] * 1.648712158203125 ;
						cost_1[2] = cost_1_stage5[2] * 1.648712158203125 ;
						cost_1[3] = cost_1_stage5[3] * 1.648712158203125 ;*/
						/////////////////////////////////////////////////////////////taylor expansion////////////////////////////////////////////////////////////////////////////


						//interpolation
						cost_1[0] = exponentialCost(cost_exponent[0]);
						cost_1[1] = exponentialCost(cost_exponent[1]);
						cost_1[2] = exponentialCost(cost_exponent[2]);
						cost_1[3] = exponentialCost(cost_exponent[3]);


						int bit = 3;
						float cost_temp[4];
						cost_temp[0] = 2 - cost_1[0] - cost_hamming[0];
						cost_temp[1] = 2 - cost_1[1] - cost_hamming[1];
						cost_temp[2] = 2 - cost_1[2] - cost_hamming[2];
						cost_temp[3] = 2 - cost_1[3] - cost_hamming[3];

						cost[0] = float(int(cost_temp[0] * (pow(2, bit)))) / (pow(2, bit));
						cost[1] = float(int(cost_temp[1] * (pow(2, bit)))) / (pow(2, bit));
						cost[2] = float(int(cost_temp[2] * (pow(2, bit)))) / (pow(2, bit));
						cost[3] = float(int(cost_temp[3] * (pow(2, bit)))) / (pow(2, bit));



						/// save the prefer position
						float ave = (cost[0] + cost[1] + cost[2] + cost[3]) / 4;
						cost_ave[x] = ave;
					}
				}
				float min = cost_ave[0];
				int mindex = 0;
				for (int i = 1; i < 4; i++) {
					if (min > cost_ave[i]) {
						min = cost_ave[i];
						mindex = i;
						cost_ave[i] = 0;
					}
				}
				B[height][width][k] = mindex;
				C[height][width][k] = min;
				cost_ave[0] = 0;
			}
			width++;
		}
		width = 0;
		height++;
	}
}
void calculatePixelCost4(Mat& leftimage_grad, Mat& rightimage_grad, Mat& leftimage, Mat& rightimage, Mat& left_grad_x, Mat& left_grad_y, Mat& right_grad_x, Mat& right_grad_y,
	float*** B, int disparityRange, float*** C, int Grad, int Ctg)
{
	Mat Left_census, Right_census;
	Left_census = ProcessImg(leftimage_grad);//      ͼ   õ   ͼ  CENSUSͼ   Left_census

	Right_census = ProcessImg(rightimage_grad);
	//imshow("Left_census", Left_census);
	//imshow("Right_census", Right_census);

	int ImgHeight = leftimage.rows;
	int ImgWidth = leftimage.cols;

	int height = 0;
	int width = 0;
	for (int i = 0; i < ImgHeight - 1; i = i++) // i=i+2 for both col and row
	{
		for (int j = 0; j < ImgWidth - 1; j = j + 2)
		{
			uchar L1;
			uchar L2;
			uchar L3;
			uchar L4;

			uchar R1;
			uchar R2;
			uchar R3;
			uchar R4;
			uchar diff1;
			uchar diff2;
			uchar diff3;
			uchar diff4;

			int x1 = i;
			int y1 = j;
			if ((x1 > ImgHeight - 2) || (y1 > ImgWidth - 2)) {
				x1 = ImgHeight - 2;
				y1 = ImgWidth - 2;
			}
			L1 = Left_census.at<uchar>(x1, y1);
			L2 = Left_census.at<uchar>(x1 + 1, y1);
			L3 = Left_census.at<uchar>(x1, y1 + 1);
			L4 = Left_census.at<uchar>(x1 + 1, y1 + 1);
			for (int k = 0; k < disparityRange / 4; k++)
			{
				float cost_ave[4]{};
				for (int x = 0; x < 4; x++)
				{
					int temp = 4 * k + x;
					int y = j - temp;
					if (((i + 1) > ImgHeight - 1) || ((y + 1) > ImgWidth - 1) || ((y) < 0))
					{
						cost_ave[x] = Window_Size * Window_Size;
						//C[height][width][k] = 0;
					}
					else
					{
						R1 = Right_census.at<uchar>(i, y);
						R2 = Right_census.at<uchar>(i + 1, y);
						R3 = Right_census.at<uchar>(i, y + 1);
						R4 = Right_census.at<uchar>(i + 1, y + 1);

						diff1 = L1 ^ R1;
						diff2 = L2 ^ R2;
						diff3 = L3 ^ R3;
						diff4 = L4 ^ R4;

						int hamming[4]{};
						hamming[0] = GetHammingWeight(diff1);
						hamming[1] = GetHammingWeight(diff2);
						hamming[2] = GetHammingWeight(diff3);
						hamming[3] = GetHammingWeight(diff4);

						/*if(hamming[0]>0)
							cout << "hamming[0]" << hamming[0] << endl;
						if (hamming[1] > 0)
							cout << "hamming[1]" << hamming[1] << endl;
						if (hamming[2] > 0)
							cout << "hamming[2]" << hamming[2] << endl;
						if (hamming[3] > 0)
							cout << "hamming[3]" << hamming[3] << endl;*/


						int cost_grad[4]{};
						cost_grad[0] = abs(left_grad_x.at<short>(i, j) - right_grad_x.at<short>(i, y)) + abs(left_grad_y.at<short>(i, j) - right_grad_y.at<short>(i, y));
						cost_grad[1] = abs(left_grad_x.at<short>(i, j) - right_grad_x.at<short>(i + 1, y)) + abs(left_grad_y.at<short>(i, j) - right_grad_y.at<short>(i + 1, y));
						cost_grad[2] = abs(left_grad_x.at<short>(i, j) - right_grad_x.at<short>(i, y + 1)) + abs(left_grad_y.at<short>(i, j) - right_grad_y.at<short>(i, y + 1));
						cost_grad[3] = abs(left_grad_x.at<short>(i, j) - right_grad_x.at<short>(i + 1, y + 1)) + abs(left_grad_y.at<short>(i, j) - right_grad_y.at<short>(i + 1, y + 1));

						/*cout << "cost_grad[0]" << cost_grad[0] << endl;
						cout << "cost_grad[1]" << cost_grad[1] << endl;
						cout << "cost_grad[2]" << cost_grad[2] << endl;
						cout << "cost_grad[3]" << cost_grad[3] << endl;
	*/


						float cost[4]{};

						float cost_hamming[4];

						switch (hamming[0]) {
						case 0:
							cost_hamming[0] = 1;
							break;
						case 1:
							cost_hamming[0] = 0.9375;
							break;
						case 2:
							cost_hamming[0] = 0.875;
							break;
						case 3:
							cost_hamming[0] = 0.828125;
							break;
						case 4:
							cost_hamming[0] = 0.765625;
							break;
						case 5:
							cost_hamming[0] = 0.71875;
							break;
						case 6:
							cost_hamming[0] = 0.671875;
							break;
						case 7:
							cost_hamming[0] = 0.640625;
							break;
						case 8:
							cost_hamming[0] = 0.59375;
							break;
						case 9:
							cost_hamming[0] = 0.5625;
							break;
						}


						switch (hamming[1]) {
						case 0:
							cost_hamming[1] = 1;
							break;
						case 1:
							cost_hamming[1] = 0.9375;
							break;
						case 2:
							cost_hamming[1] = 0.875;
							break;
						case 3:
							cost_hamming[1] = 0.828125;
							break;
						case 4:
							cost_hamming[1] = 0.765625;
							break;
						case 5:
							cost_hamming[1] = 0.71875;
							break;
						case 6:
							cost_hamming[1] = 0.671875;
							break;
						case 7:
							cost_hamming[1] = 0.640625;
							break;
						case 8:
							cost_hamming[1] = 0.59375;
							break;
						case 9:
							cost_hamming[1] = 0.5625;
							break;
						}


						switch (hamming[2]) {
						case 0:
							cost_hamming[2] = 1;
							break;
						case 1:
							cost_hamming[2] = 0.9375;
							break;
						case 2:
							cost_hamming[2] = 0.875;
							break;
						case 3:
							cost_hamming[2] = 0.828125;
							break;
						case 4:
							cost_hamming[2] = 0.765625;
							break;
						case 5:
							cost_hamming[2] = 0.71875;
							break;
						case 6:
							cost_hamming[2] = 0.671875;
							break;
						case 7:
							cost_hamming[2] = 0.640625;
							break;
						case 8:
							cost_hamming[2] = 0.59375;
							break;
						case 9:
							cost_hamming[2] = 0.5625;
							break;
						}

						switch (hamming[3]) {
						case 0:
							cost_hamming[3] = 1;
							break;
						case 1:
							cost_hamming[3] = 0.9375;
							break;
						case 2:
							cost_hamming[3] = 0.875;
							break;
						case 3:
							cost_hamming[3] = 0.828125;
							break;
						case 4:
							cost_hamming[3] = 0.765625;
							break;
						case 5:
							cost_hamming[3] = 0.71875;
							break;
						case 6:
							cost_hamming[3] = 0.671875;
							break;
						case 7:
							cost_hamming[3] = 0.640625;
							break;
						case 8:
							cost_hamming[3] = 0.59375;
							break;
						case 9:
							cost_hamming[3] = 0.5625;
							break;
						}


						//cost_hamming      exp(float(0 - hamming[0]) / float(Ctg))


						float cost_exponent[4];
						cost_exponent[0] = (float(0 - cost_grad[0]) / float(Grad));
						cost_exponent[1] = (float(0 - cost_grad[1]) / float(Grad));
						cost_exponent[2] = (float(0 - cost_grad[2]) / float(Grad));
						cost_exponent[3] = (float(0 - cost_grad[3]) / float(Grad));

						//////////////////////////////////////////Taylor չ  ////////////////////////////////////////////////////////////////
						float cost_1_stage1[4];
						cost_1_stage1[0] = float(cost_exponent[0] * 0.008331298828125) + 0.02117919921875;
						cost_1_stage1[1] = float(cost_exponent[1] * 0.008331298828125) + 0.02117919921875;
						cost_1_stage1[2] = float(cost_exponent[2] * 0.008331298828125) + 0.02117919921875;
						cost_1_stage1[3] = float(cost_exponent[3] * 0.008331298828125) + 0.02117919921875;

						float cost_1_stage2[4];
						cost_1_stage2[0] = float(cost_exponent[0] * cost_1_stage1[0]) + 0.1034698486328125;
						cost_1_stage2[1] = float(cost_exponent[1] * cost_1_stage1[1]) + 0.1034698486328125;
						cost_1_stage2[2] = float(cost_exponent[2] * cost_1_stage1[2]) + 0.1034698486328125;
						cost_1_stage2[3] = float(cost_exponent[3] * cost_1_stage1[3]) + 0.1034698486328125;

						float cost_1_stage3[4];
						cost_1_stage3[0] = float(cost_exponent[0] * cost_1_stage2[0]) + 0.3025970458984375;
						cost_1_stage3[1] = float(cost_exponent[1] * cost_1_stage2[1]) + 0.3025970458984375;
						cost_1_stage3[2] = float(cost_exponent[2] * cost_1_stage2[2]) + 0.3025970458984375;
						cost_1_stage3[3] = float(cost_exponent[3] * cost_1_stage2[3]) + 0.3025970458984375;

						float cost_1_stage4[4];
						cost_1_stage4[0] = float(cost_exponent[0] * cost_1_stage3[0]) + 0.6065826416015625;
						cost_1_stage4[1] = float(cost_exponent[1] * cost_1_stage3[1]) + 0.6065826416015625;
						cost_1_stage4[2] = float(cost_exponent[2] * cost_1_stage3[2]) + 0.6065826416015625;
						cost_1_stage4[3] = float(cost_exponent[3] * cost_1_stage3[3]) + 0.6065826416015625;

						float cost_1_stage5[4];
						cost_1_stage5[0] = float(cost_exponent[0] * cost_1_stage4[0]) + 0.6065216064453125;
						cost_1_stage5[1] = float(cost_exponent[1] * cost_1_stage4[1]) + 0.6065216064453125;
						cost_1_stage5[2] = float(cost_exponent[2] * cost_1_stage4[2]) + 0.6065216064453125;
						cost_1_stage5[3] = float(cost_exponent[3] * cost_1_stage4[3]) + 0.6065216064453125;

						float cost_1[4];

						/*cost_1[0] = cost_1_stage5[0] * 1.648712158203125 ;
						cost_1[1] = cost_1_stage5[1] * 1.648712158203125 ;
						cost_1[2] = cost_1_stage5[2] * 1.648712158203125 ;
						cost_1[3] = cost_1_stage5[3] * 1.648712158203125 ;*/
						/////////////////////////////////////////////////////////////taylor expansion////////////////////////////////////////////////////////////////////////////


						//interpolation
						cost_1[0] = exponentialCost(cost_exponent[0]);
						cost_1[1] = exponentialCost(cost_exponent[1]);
						cost_1[2] = exponentialCost(cost_exponent[2]);
						cost_1[3] = exponentialCost(cost_exponent[3]);


						int bit = 3;
						float cost_temp[4];
						cost_temp[0] = 2 - cost_1[0] - cost_hamming[0];
						cost_temp[1] = 2 - cost_1[1] - cost_hamming[1];
						cost_temp[2] = 2 - cost_1[2] - cost_hamming[2];
						cost_temp[3] = 2 - cost_1[3] - cost_hamming[3];

						cost[0] = float(int(cost_temp[0] * (pow(2, bit)))) / (pow(2, bit));
						cost[1] = float(int(cost_temp[1] * (pow(2, bit)))) / (pow(2, bit));
						cost[2] = float(int(cost_temp[2] * (pow(2, bit)))) / (pow(2, bit));
						cost[3] = float(int(cost_temp[3] * (pow(2, bit)))) / (pow(2, bit));



						/// save the prefer position
						float minp = cost[0];
						for (int i = 1; i < 4; i++) {
							if (minp > cost[i]) {
								minp = cost[i];
								cost[i] = 0;
							}
						}
						cost_ave[x] = minp;
					}
				}
				float min = cost_ave[0];
				int mindex = 0;
				for (int i = 1; i < 4; i++) {
					if (min > cost_ave[i]) {
						min = cost_ave[i];
						mindex = i;
						cost_ave[i] = 0;
					}
				}
				B[height][width][k] = mindex;
				C[height][width][k] = min;
				cost_ave[0] = 0;
			}
			width++;
		}
		width = 0;
		height++;
		float progress = height * 100.0 / (ImgHeight - 1);
		std::cout << "\rcalculatePixelCost4 Progress: " << std::fixed << std::setprecision(2) << progress << "%" << std::flush;
	}
	std::cout << "\n" << std::endl;
}

//For right
void calculatePixelCost_R(Mat& leftimage_grad, Mat& rightimage_grad, Mat& leftimage, Mat& rightimage, Mat& left_grad_x, Mat& left_grad_y, Mat& right_grad_x, Mat& right_grad_y,
	float*** B, int disparityRange, float*** C, int Grad, int Ctg)
{
	Mat Left_census, Right_census;
	Left_census = ProcessImg(leftimage_grad);//处理左图，得到左图的CENSUS图像 Left_census
	Right_census = ProcessImg(rightimage_grad);

	int ImgHeight = rightimage.rows;
	int ImgWidth = rightimage.cols;

	for (int i = 0; i < ImgHeight; i++)
	{

		for (int j = 0; j < ImgWidth; j++)
		{

			uchar R;
			uchar L1;
			uchar L2;
			uchar L3;
			uchar L4;
			uchar diff1;
			uchar diff2;
			uchar diff3;
			uchar diff4;

			R = Right_census.at<uchar>(i, j);
			for (int k = 0; k < disparityRange / 4; k++)
			{
				int temp = 4 * k;
				int temp1 = 4 * k + 1;
				int temp2 = 4 * k + 2;
				int temp3 = 4 * k + 3;

				int y1 = j + temp;
				int y2 = j + temp1;
				int y3 = j + temp2;
				int y4 = j + temp3;


				if (y4 > ImgWidth - 1)
				{
					C[i][j][k] = Window_Size * Window_Size;
				}
				else
				{
					L1 = Left_census.at<uchar>(i, y1);
					L2 = Left_census.at<uchar>(i, y2);
					L3 = Left_census.at<uchar>(i, y3);
					L4 = Left_census.at<uchar>(i, y4);

					diff1 = R ^ L1;
					diff2 = R ^ L2;
					diff3 = R ^ L3;
					diff4 = R ^ L4;

					int hamming[4];
					hamming[0] = GetHammingWeight(diff1);
					hamming[1] = GetHammingWeight(diff2);
					hamming[2] = GetHammingWeight(diff3);
					hamming[3] = GetHammingWeight(diff4);

					int cost_grad[4];
					cost_grad[0] = abs(right_grad_x.at<short>(i, j) - left_grad_x.at<short>(i, y1)) + abs(right_grad_y.at<short>(i, j) - left_grad_y.at<short>(i, y1));
					cost_grad[1] = abs(right_grad_x.at<short>(i, j) - left_grad_x.at<short>(i, y2)) + abs(right_grad_y.at<short>(i, j) - left_grad_y.at<short>(i, y2));
					cost_grad[2] = abs(right_grad_x.at<short>(i, j) - left_grad_x.at<short>(i, y3)) + abs(right_grad_y.at<short>(i, j) - left_grad_y.at<short>(i, y3));
					cost_grad[3] = abs(right_grad_x.at<short>(i, j) - left_grad_x.at<short>(i, y4)) + abs(right_grad_y.at<short>(i, j) - left_grad_y.at<short>(i, y4));


					float cost[4];


					float cost_hamming[4];

					switch (hamming[0]) {
					case 0:
						cost_hamming[0] = 1;
						break;
					case 1:
						cost_hamming[0] = 0.9375;
						break;
					case 2:
						cost_hamming[0] = 0.875;
						break;
					case 3:
						cost_hamming[0] = 0.828125;
						break;
					case 4:
						cost_hamming[0] = 0.765625;
						break;
					case 5:
						cost_hamming[0] = 0.71875;
						break;
					case 6:
						cost_hamming[0] = 0.671875;
						break;
					case 7:
						cost_hamming[0] = 0.640625;
						break;
					case 8:
						cost_hamming[0] = 0.59375;
						break;
					case 9:
						cost_hamming[0] = 0.5625;
						break;
					}


					switch (hamming[1]) {
					case 0:
						cost_hamming[1] = 1;
						break;
					case 1:
						cost_hamming[1] = 0.9375;
						break;
					case 2:
						cost_hamming[1] = 0.875;
						break;
					case 3:
						cost_hamming[1] = 0.828125;
						break;
					case 4:
						cost_hamming[1] = 0.765625;
						break;
					case 5:
						cost_hamming[1] = 0.71875;
						break;
					case 6:
						cost_hamming[1] = 0.671875;
						break;
					case 7:
						cost_hamming[1] = 0.640625;
						break;
					case 8:
						cost_hamming[1] = 0.59375;
						break;
					case 9:
						cost_hamming[1] = 0.5625;
						break;
					}


					switch (hamming[2]) {
					case 0:
						cost_hamming[2] = 1;
						break;
					case 1:
						cost_hamming[2] = 0.9375;
						break;
					case 2:
						cost_hamming[2] = 0.875;
						break;
					case 3:
						cost_hamming[2] = 0.828125;
						break;
					case 4:
						cost_hamming[2] = 0.765625;
						break;
					case 5:
						cost_hamming[2] = 0.71875;
						break;
					case 6:
						cost_hamming[2] = 0.671875;
						break;
					case 7:
						cost_hamming[2] = 0.640625;
						break;
					case 8:
						cost_hamming[2] = 0.59375;
						break;
					case 9:
						cost_hamming[2] = 0.5625;
						break;
					}

					switch (hamming[3]) {
					case 0:
						cost_hamming[3] = 1;
						break;
					case 1:
						cost_hamming[3] = 0.9375;
						break;
					case 2:
						cost_hamming[3] = 0.875;
						break;
					case 3:
						cost_hamming[3] = 0.828125;
						break;
					case 4:
						cost_hamming[3] = 0.765625;
						break;
					case 5:
						cost_hamming[3] = 0.71875;
						break;
					case 6:
						cost_hamming[3] = 0.671875;
						break;
					case 7:
						cost_hamming[3] = 0.640625;
						break;
					case 8:
						cost_hamming[3] = 0.59375;
						break;
					case 9:
						cost_hamming[3] = 0.5625;
						break;
					}


					float cost_exponent[4];
					cost_exponent[0] = (float(0 - cost_grad[0]) / float(Grad));
					cost_exponent[1] = (float(0 - cost_grad[1]) / float(Grad));
					cost_exponent[2] = (float(0 - cost_grad[2]) / float(Grad));
					cost_exponent[3] = (float(0 - cost_grad[3]) / float(Grad));

					/////////////////////////////////////////////////////////////////////////////////////泰勒展开 乘法太多 舍弃/////////////////////////////////////////////////////////////////////////////////////////////////////
					float cost_1_stage1[4];
					cost_1_stage1[0] = float(cost_exponent[0] * 0.008331298828125) + 0.02117919921875;
					cost_1_stage1[1] = float(cost_exponent[1] * 0.008331298828125) + 0.02117919921875;
					cost_1_stage1[2] = float(cost_exponent[2] * 0.008331298828125) + 0.02117919921875;
					cost_1_stage1[3] = float(cost_exponent[3] * 0.008331298828125) + 0.02117919921875;

					float cost_1_stage2[4];
					cost_1_stage2[0] = float(cost_exponent[0] * cost_1_stage1[0]) + 0.1034698486328125;
					cost_1_stage2[1] = float(cost_exponent[1] * cost_1_stage1[1]) + 0.1034698486328125;
					cost_1_stage2[2] = float(cost_exponent[2] * cost_1_stage1[2]) + 0.1034698486328125;
					cost_1_stage2[3] = float(cost_exponent[3] * cost_1_stage1[3]) + 0.1034698486328125;

					float cost_1_stage3[4];
					cost_1_stage3[0] = float(cost_exponent[0] * cost_1_stage2[0]) + 0.3025970458984375;
					cost_1_stage3[1] = float(cost_exponent[1] * cost_1_stage2[1]) + 0.3025970458984375;
					cost_1_stage3[2] = float(cost_exponent[2] * cost_1_stage2[2]) + 0.3025970458984375;
					cost_1_stage3[3] = float(cost_exponent[3] * cost_1_stage2[3]) + 0.3025970458984375;

					float cost_1_stage4[4];
					cost_1_stage4[0] = float(cost_exponent[0] * cost_1_stage3[0]) + 0.6065826416015625;
					cost_1_stage4[1] = float(cost_exponent[1] * cost_1_stage3[1]) + 0.6065826416015625;
					cost_1_stage4[2] = float(cost_exponent[2] * cost_1_stage3[2]) + 0.6065826416015625;
					cost_1_stage4[3] = float(cost_exponent[3] * cost_1_stage3[3]) + 0.6065826416015625;

					float cost_1_stage5[4];
					cost_1_stage5[0] = float(cost_exponent[0] * cost_1_stage4[0]) + 0.6065216064453125;
					cost_1_stage5[1] = float(cost_exponent[1] * cost_1_stage4[1]) + 0.6065216064453125;
					cost_1_stage5[2] = float(cost_exponent[2] * cost_1_stage4[2]) + 0.6065216064453125;
					cost_1_stage5[3] = float(cost_exponent[3] * cost_1_stage4[3]) + 0.6065216064453125;

					float cost_1[4];

					//taylor expansion
					/*cost_1[0] = cost_1_stage5[0] * 1.648712158203125 ;
					cost_1[1] = cost_1_stage5[1] * 1.648712158203125 ;
					cost_1[2] = cost_1_stage5[2] * 1.648712158203125 ;
					cost_1[3] = cost_1_stage5[3] * 1.648712158203125 ;*/

					//interpolation
					cost_1[0] = exponentialCost(cost_exponent[0]);
					cost_1[1] = exponentialCost(cost_exponent[1]);
					cost_1[2] = exponentialCost(cost_exponent[2]);
					cost_1[3] = exponentialCost(cost_exponent[3]);
					/////////////////////////////////////////////////////////////////////////////////////泰勒展开 乘法太多 舍弃/////////////////////////////////////////////////////////////////////////////////////////////////////

					int bit = 3;
					float cost_temp[4];
					cost_temp[0] = 2 - cost_1[0] - cost_hamming[0];
					cost_temp[1] = 2 - cost_1[1] - cost_hamming[1];
					cost_temp[2] = 2 - cost_1[2] - cost_hamming[2];
					cost_temp[3] = 2 - cost_1[3] - cost_hamming[3];

					cost[0] = float(int(cost_temp[0] * (pow(2, bit)))) / (pow(2, bit));
					cost[1] = float(int(cost_temp[1] * (pow(2, bit)))) / (pow(2, bit));
					cost[2] = float(int(cost_temp[2] * (pow(2, bit)))) / (pow(2, bit));
					cost[3] = float(int(cost_temp[3] * (pow(2, bit)))) / (pow(2, bit));

					/*	cost[0] = GetHammingWeight(diff1);
						cost[1] = GetHammingWeight(diff2);
						cost[2] = GetHammingWeight(diff3);
						cost[3] = GetHammingWeight(diff4);*/


						/////////////////////////////////////////////////////////////////////////////////////原始公式计算/////////////////////////////////////////////////////////////////////////////////////////////////////
											/*cost[0] = 2 - exp(float(0 - cost_grad[0]) / float(Grad)) - exp(float(0 - hamming[0]) / float(Ctg));
											cost[1] = 2 - exp(float(0 - cost_grad[1]) / float(Grad)) - exp(float(0 - hamming[1]) / float(Ctg));
											cost[2] = 2 - exp(float(0 - cost_grad[2]) / float(Grad)) - exp(float(0 - hamming[2]) / float(Ctg));
											cost[3] = 2 - exp(float(0 - cost_grad[3]) / float(Grad)) - exp(float(0 - hamming[3]) / float(Ctg));*/
											/////////////////////////////////////////////////////////////////////////////////////原始公式计算/////////////////////////////////////////////////////////////////////////////////////////////////////




											//////////////////////////////////////////////////////////////////////////////////////////////指数部分分为整数和小数 移位等操作 在这里不适用///////////////////////////////////////////////////////////
															//	float cost_grad_DivGrad[4];
															//	cost_grad_DivGrad[0] = cost_grad[0] / float(Grad);
															//	cost_grad_DivGrad[1] = cost_grad[1] / float(Grad);
															//	cost_grad_DivGrad[2] = cost_grad[2] / float(Grad);
															//	cost_grad_DivGrad[3] = cost_grad[3] / float(Grad);

															//	float cost_grad_DivGrad_MulLog2e[4];
															//	cost_grad_DivGrad_MulLog2e[0] = cost_grad_DivGrad[0] * 1.4375;
															//	cost_grad_DivGrad_MulLog2e[1] = cost_grad_DivGrad[1] * 1.4375;
															//	cost_grad_DivGrad_MulLog2e[2] = cost_grad_DivGrad[2] * 1.4375;
															//	cost_grad_DivGrad_MulLog2e[3] = cost_grad_DivGrad[3] * 1.4375;


															//	//2^(-x) x=u+v u是整数部分，v是小数部分
															//	int u[4];
															//	u[0] = int(cost_grad_DivGrad_MulLog2e[0]);
															//	u[1] = int(cost_grad_DivGrad_MulLog2e[1]);
															//	u[2] = int(cost_grad_DivGrad_MulLog2e[2]);
															//	u[3] = int(cost_grad_DivGrad_MulLog2e[3]);

															//	float v[4];
															//	v[0] = cost_grad_DivGrad_MulLog2e[0] - u[0];
															//	v[1] = cost_grad_DivGrad_MulLog2e[1] - u[1];
															//	v[2] = cost_grad_DivGrad_MulLog2e[2] - u[2];
															//	v[3] = cost_grad_DivGrad_MulLog2e[3] - u[3];

															//	//2^(-v)
															//	float NegV2[4];
															//	NegV2[0] = 1 - 0.6875 * v[0];
															//	NegV2[1] = 1 - 0.6875 * v[1];
															//	NegV2[2] = 1 - 0.6875 * v[2];
															//	NegV2[3] = 1 - 0.6875 * v[3];

															//	float cost_GRAD[4];
															//	cost_GRAD[0] = NegV2[0] * (2 ^ (-u[0]));
															//	cost_GRAD[1] = NegV2[1] * (2 ^ (-u[1]));
															//	cost_GRAD[2] = NegV2[2] * (2 ^ (-u[2]));
															//	cost_GRAD[3] = NegV2[3] * (2 ^ (-u[3]));

															//	//保留小数位

															//	float cost_origin[4];

															//	cost_origin[0] = 2 - cost_GRAD[0] - cost_hamming[0];
															//	cost_origin[1] = 2 - cost_GRAD[1] - cost_hamming[1];
															//	cost_origin[2] = 2 - cost_GRAD[2] - cost_hamming[2];
															//	cost_origin[3] = 2 - cost_GRAD[3] - cost_hamming[3];


															//	int demical_bit = 32;
															//	int cost_2n[4];
															//	cost_2n[0] = int(cost_origin[0] * (2 ^ (demical_bit)));
															//	cost_2n[1] = int(cost_origin[1] * (2 ^ (demical_bit)));
															//	cost_2n[2] = int(cost_origin[2] * (2 ^ (demical_bit)));
															//	cost_2n[3] = int(cost_origin[3] * (2 ^ (demical_bit)));



															//	cost[0] = cost_2n[0] / (2 ^ (demical_bit));
															//	cost[1] = cost_2n[1] / (2 ^ (demical_bit));
															//	cost[2] = cost_2n[2] / (2 ^ (demical_bit));
															//	cost[3] = cost_2n[3] / (2 ^ (demical_bit));


																/*float cost_temp[4];

																cost_temp[0] = 2 - exp(float(0 - cost_grad[0]) / float(Grad)) - exp(float(0 - hamming[0]) / float(Ctg));
																cost_temp[1] = 2 - exp(float(0 - cost_grad[1]) / float(Grad)) - exp(float(0 - hamming[1]) / float(Ctg));
																cost_temp[2] = 2 - exp(float(0 - cost_grad[2]) / float(Grad)) - exp(float(0 - hamming[2]) / float(Ctg));
																cost_temp[3] = 2 - exp(float(0 - cost_grad[3]) / float(Grad)) - exp(float(0 - hamming[3]) / float(Ctg));


																int cost_bit =10;

																cost[0] = float(int(cost_temp[0]*(2^(cost_bit))))/ (2 ^ (cost_bit));
																cost[1] = float(int(cost_temp[1] * (2 ^ (cost_bit)))) / (2 ^ (cost_bit));
																cost[2] = float(int(cost_temp[2] * (2 ^ (cost_bit)))) / (2 ^ (cost_bit));
																cost[3] = float(int(cost_temp[3] * (2 ^ (cost_bit)))) / (2 ^ (cost_bit));*/


																//////////////////////////////////////////////////////////////////////////////////////////////指数部分分为整数和小数 移位等操作 在这里不适用///////////////////////////////////////////////////////////

																					/// save the prefer position
					float min = cost[0];
					int mindex = 0;
					for (int i = 1; i < 4; i++) {
						if (min > cost[i]) {
							min = cost[i];
							mindex = i;
						}
					}

					B[i][j][k] = mindex;
					//归一化
					//C[i][j][k] = 2 - exp((-1) * float(cost_grad[mindex]) / float(Grad)) - exp((-1) * float(hamming[mindex]) / float(Ctg));
					C[i][j][k] = min;


					//保留整数小数位
					/*stringstream ss;
					ss.setf(ios::fixed);
					ss.precision(2);
					ss << min;
					C[i][j][k] = stof(ss.str().c_str());
					*/

				}
			}
		}
		float progress = i * 100.0 / (ImgHeight - 1);
		std::cout << "\rcalculatePixelCost_R Progress: " << std::fixed << std::setprecision(2) << progress << "%" << std::flush;
	}
	std::cout << "\n" << std::endl;
}

void calculatePixelCost2_R(Mat& leftimage_grad, Mat& rightimage_grad, Mat& leftimage, Mat& rightimage, Mat& left_grad_x, Mat& left_grad_y, Mat& right_grad_x, Mat& right_grad_y,
	float*** B, int disparityRange, float*** C, int Grad, int Ctg)
{
	Mat Left_census, Right_census;
	Left_census = ProcessImg(leftimage_grad);//      ͼ   õ   ͼ  CENSUSͼ   Left_census
	Right_census = ProcessImg(rightimage_grad);

	int ImgHeight = rightimage.rows;
	int ImgWidth = rightimage.cols;
	int height = 0;
	int width = 0;
	for (int i = 0; i < ImgHeight; i = i + 2)
	{

		for (int j = 0; j < ImgWidth; j = j + 2)
		{

			uchar R1;
			uchar R2;
			uchar R3;
			uchar R4;
			uchar L1;
			uchar L2;
			uchar L3;
			uchar L4;
			uchar diff1;
			uchar diff2;
			uchar diff3;
			uchar diff4;

			R1 = Right_census.at<uchar>(i, j);
			R2 = Right_census.at<uchar>(i + 1, j);
			R3 = Right_census.at<uchar>(i, j + 1);
			R4 = Right_census.at<uchar>(i + 1, j + 1);
			for (int k = 0; k < disparityRange; k++)
			{
				int y = j + k;
				if (((i + 1) > ImgHeight - 1) || ((j + 1) > ImgWidth - 1) || ((y + 1) > ImgWidth - 1))
				{
					C[height][width][k] = Window_Size * Window_Size;
					//C[height][width][k] = 0;
				}
				else
				{
					L1 = Left_census.at<uchar>(i, y);
					L2 = Left_census.at<uchar>(i + 1, y);
					L3 = Left_census.at<uchar>(i, y + 1);
					L4 = Left_census.at<uchar>(i + 1, y + 1);


					diff1 = R1 ^ L1;
					diff2 = R2 ^ L2;
					diff3 = R3 ^ L3;
					diff4 = R4 ^ L4;

					int hamming[4];
					hamming[0] = GetHammingWeight(diff1);
					hamming[1] = GetHammingWeight(diff2);
					hamming[2] = GetHammingWeight(diff3);
					hamming[3] = GetHammingWeight(diff4);

					int cost_grad[4];
					cost_grad[0] = abs(right_grad_x.at<short>(i, j) - left_grad_x.at<short>(i, y)) + abs(right_grad_y.at<short>(i, j) - left_grad_y.at<short>(i, y));
					cost_grad[1] = abs(right_grad_x.at<short>(i, j) - left_grad_x.at<short>(i + 1, y)) + abs(right_grad_y.at<short>(i, j) - left_grad_y.at<short>(i + 1, y));
					cost_grad[2] = abs(right_grad_x.at<short>(i, j) - left_grad_x.at<short>(i, y + 1)) + abs(right_grad_y.at<short>(i, j) - left_grad_y.at<short>(i, y + 1));
					cost_grad[3] = abs(right_grad_x.at<short>(i, j) - left_grad_x.at<short>(i + 1, y + 1)) + abs(right_grad_y.at<short>(i, j) - left_grad_y.at<short>(i + 1, y + 1));


					float cost[4];


					float cost_hamming[4];

					switch (hamming[0]) {
					case 0:
						cost_hamming[0] = 1;
						break;
					case 1:
						cost_hamming[0] = 0.9375;
						break;
					case 2:
						cost_hamming[0] = 0.875;
						break;
					case 3:
						cost_hamming[0] = 0.828125;
						break;
					case 4:
						cost_hamming[0] = 0.765625;
						break;
					case 5:
						cost_hamming[0] = 0.71875;
						break;
					case 6:
						cost_hamming[0] = 0.671875;
						break;
					case 7:
						cost_hamming[0] = 0.640625;
						break;
					case 8:
						cost_hamming[0] = 0.59375;
						break;
					case 9:
						cost_hamming[0] = 0.5625;
						break;
					}


					switch (hamming[1]) {
					case 0:
						cost_hamming[1] = 1;
						break;
					case 1:
						cost_hamming[1] = 0.9375;
						break;
					case 2:
						cost_hamming[1] = 0.875;
						break;
					case 3:
						cost_hamming[1] = 0.828125;
						break;
					case 4:
						cost_hamming[1] = 0.765625;
						break;
					case 5:
						cost_hamming[1] = 0.71875;
						break;
					case 6:
						cost_hamming[1] = 0.671875;
						break;
					case 7:
						cost_hamming[1] = 0.640625;
						break;
					case 8:
						cost_hamming[1] = 0.59375;
						break;
					case 9:
						cost_hamming[1] = 0.5625;
						break;
					}


					switch (hamming[2]) {
					case 0:
						cost_hamming[2] = 1;
						break;
					case 1:
						cost_hamming[2] = 0.9375;
						break;
					case 2:
						cost_hamming[2] = 0.875;
						break;
					case 3:
						cost_hamming[2] = 0.828125;
						break;
					case 4:
						cost_hamming[2] = 0.765625;
						break;
					case 5:
						cost_hamming[2] = 0.71875;
						break;
					case 6:
						cost_hamming[2] = 0.671875;
						break;
					case 7:
						cost_hamming[2] = 0.640625;
						break;
					case 8:
						cost_hamming[2] = 0.59375;
						break;
					case 9:
						cost_hamming[2] = 0.5625;
						break;
					}

					switch (hamming[3]) {
					case 0:
						cost_hamming[3] = 1;
						break;
					case 1:
						cost_hamming[3] = 0.9375;
						break;
					case 2:
						cost_hamming[3] = 0.875;
						break;
					case 3:
						cost_hamming[3] = 0.828125;
						break;
					case 4:
						cost_hamming[3] = 0.765625;
						break;
					case 5:
						cost_hamming[3] = 0.71875;
						break;
					case 6:
						cost_hamming[3] = 0.671875;
						break;
					case 7:
						cost_hamming[3] = 0.640625;
						break;
					case 8:
						cost_hamming[3] = 0.59375;
						break;
					case 9:
						cost_hamming[3] = 0.5625;
						break;
					}


					float cost_exponent[4];
					cost_exponent[0] = (float(0 - cost_grad[0]) / float(Grad));
					cost_exponent[1] = (float(0 - cost_grad[1]) / float(Grad));
					cost_exponent[2] = (float(0 - cost_grad[2]) / float(Grad));
					cost_exponent[3] = (float(0 - cost_grad[3]) / float(Grad));

					/////////////////////////////////////////////////////////////////////////////////////̩  չ    ˷ ̫       /////////////////////////////////////////////////////////////////////////////////////////////////////
					float cost_1_stage1[4];
					cost_1_stage1[0] = float(cost_exponent[0] * 0.008331298828125) + 0.02117919921875;
					cost_1_stage1[1] = float(cost_exponent[1] * 0.008331298828125) + 0.02117919921875;
					cost_1_stage1[2] = float(cost_exponent[2] * 0.008331298828125) + 0.02117919921875;
					cost_1_stage1[3] = float(cost_exponent[3] * 0.008331298828125) + 0.02117919921875;

					float cost_1_stage2[4];
					cost_1_stage2[0] = float(cost_exponent[0] * cost_1_stage1[0]) + 0.1034698486328125;
					cost_1_stage2[1] = float(cost_exponent[1] * cost_1_stage1[1]) + 0.1034698486328125;
					cost_1_stage2[2] = float(cost_exponent[2] * cost_1_stage1[2]) + 0.1034698486328125;
					cost_1_stage2[3] = float(cost_exponent[3] * cost_1_stage1[3]) + 0.1034698486328125;

					float cost_1_stage3[4];
					cost_1_stage3[0] = float(cost_exponent[0] * cost_1_stage2[0]) + 0.3025970458984375;
					cost_1_stage3[1] = float(cost_exponent[1] * cost_1_stage2[1]) + 0.3025970458984375;
					cost_1_stage3[2] = float(cost_exponent[2] * cost_1_stage2[2]) + 0.3025970458984375;
					cost_1_stage3[3] = float(cost_exponent[3] * cost_1_stage2[3]) + 0.3025970458984375;

					float cost_1_stage4[4];
					cost_1_stage4[0] = float(cost_exponent[0] * cost_1_stage3[0]) + 0.6065826416015625;
					cost_1_stage4[1] = float(cost_exponent[1] * cost_1_stage3[1]) + 0.6065826416015625;
					cost_1_stage4[2] = float(cost_exponent[2] * cost_1_stage3[2]) + 0.6065826416015625;
					cost_1_stage4[3] = float(cost_exponent[3] * cost_1_stage3[3]) + 0.6065826416015625;

					float cost_1_stage5[4];
					cost_1_stage5[0] = float(cost_exponent[0] * cost_1_stage4[0]) + 0.6065216064453125;
					cost_1_stage5[1] = float(cost_exponent[1] * cost_1_stage4[1]) + 0.6065216064453125;
					cost_1_stage5[2] = float(cost_exponent[2] * cost_1_stage4[2]) + 0.6065216064453125;
					cost_1_stage5[3] = float(cost_exponent[3] * cost_1_stage4[3]) + 0.6065216064453125;

					float cost_1[4];

					//taylor expansion
					/*cost_1[0] = cost_1_stage5[0] * 1.648712158203125 ;
					cost_1[1] = cost_1_stage5[1] * 1.648712158203125 ;
					cost_1[2] = cost_1_stage5[2] * 1.648712158203125 ;
					cost_1[3] = cost_1_stage5[3] * 1.648712158203125 ;*/

					//interpolation
					cost_1[0] = exponentialCost(cost_exponent[0]);
					cost_1[1] = exponentialCost(cost_exponent[1]);
					cost_1[2] = exponentialCost(cost_exponent[2]);
					cost_1[3] = exponentialCost(cost_exponent[3]);
					/////////////////////////////////////////////////////////////////////////////////////̩  չ    ˷ ̫       /////////////////////////////////////////////////////////////////////////////////////////////////////

					int bit = 3;
					float cost_temp[4];
					cost_temp[0] = 2 - cost_1[0] - cost_hamming[0];
					cost_temp[1] = 2 - cost_1[1] - cost_hamming[1];
					cost_temp[2] = 2 - cost_1[2] - cost_hamming[2];
					cost_temp[3] = 2 - cost_1[3] - cost_hamming[3];

					cost[0] = float(int(cost_temp[0] * (pow(2, bit)))) / (pow(2, bit));
					cost[1] = float(int(cost_temp[1] * (pow(2, bit)))) / (pow(2, bit));
					cost[2] = float(int(cost_temp[2] * (pow(2, bit)))) / (pow(2, bit));
					cost[3] = float(int(cost_temp[3] * (pow(2, bit)))) / (pow(2, bit));


					float ave = (cost[0] + cost[1] + cost[2] + cost[3]) / 4;

					B[height][width][k] = 0;
					C[height][width][k] = ave;

				}
			}
			width++;
		}
		width = 0;
		height++;
	}
}
void calculatePixelCost3_R(Mat& leftimage_grad, Mat& rightimage_grad, Mat& leftimage, Mat& rightimage, Mat& left_grad_x, Mat& left_grad_y, Mat& right_grad_x, Mat& right_grad_y,
	float*** B, int disparityRange, float*** C, int Grad, int Ctg)
{
	Mat Left_census, Right_census;
	Left_census = ProcessImg(leftimage_grad);//      ͼ   õ   ͼ  CENSUSͼ   Left_census
	Right_census = ProcessImg(rightimage_grad);

	int ImgHeight = rightimage.rows;
	int ImgWidth = rightimage.cols;
	int height = 0;
	int width = 0;
	for (int i = 0; i < ImgHeight - 1; i++)
	{

		for (int j = 0; j < ImgWidth - 1; j = j + 2)
		{

			uchar R1;
			uchar R2;
			uchar R3;
			uchar R4;
			uchar L1;
			uchar L2;
			uchar L3;
			uchar L4;
			uchar diff1;
			uchar diff2;
			uchar diff3;
			uchar diff4;

			int x1 = i;
			int y1 = j;
			if ((x1 > ImgHeight - 2) || (y1 > ImgWidth - 2)) {
				x1 = ImgHeight - 2;
				y1 = ImgWidth - 2;
			}
			R1 = Right_census.at<uchar>(x1, y1);
			R2 = Right_census.at<uchar>(x1 + 1, y1);
			R3 = Right_census.at<uchar>(x1, y1 + 1);
			R4 = Right_census.at<uchar>(x1 + 1, y1 + 1);
			for (int k = 0; k < disparityRange / 4; k++)
			{
				float cost_ave[4]{};
				for (int x = 0; x < 4; x++)
				{
					int y = j + 4 * k + x;
					if (((i + 1) > ImgHeight - 1) || ((y + 1) > ImgWidth - 1) || ((y + 1) > ImgWidth - 1))
					{
						cost_ave[x] = Window_Size * Window_Size;
						//C[height][width][k] = 0;
					}
					else
					{
						L1 = Left_census.at<uchar>(i, y);
						L2 = Left_census.at<uchar>(i + 1, y);
						L3 = Left_census.at<uchar>(i, y + 1);
						L4 = Left_census.at<uchar>(i + 1, y + 1);


						diff1 = R1 ^ L1;
						diff2 = R2 ^ L2;
						diff3 = R3 ^ L3;
						diff4 = R4 ^ L4;

						int hamming[4];
						hamming[0] = GetHammingWeight(diff1);
						hamming[1] = GetHammingWeight(diff2);
						hamming[2] = GetHammingWeight(diff3);
						hamming[3] = GetHammingWeight(diff4);

						int cost_grad[4];
						cost_grad[0] = abs(right_grad_x.at<short>(i, j) - left_grad_x.at<short>(i, y)) + abs(right_grad_y.at<short>(i, j) - left_grad_y.at<short>(i, y));
						cost_grad[1] = abs(right_grad_x.at<short>(i, j) - left_grad_x.at<short>(i + 1, y)) + abs(right_grad_y.at<short>(i, j) - left_grad_y.at<short>(i + 1, y));
						cost_grad[2] = abs(right_grad_x.at<short>(i, j) - left_grad_x.at<short>(i, y + 1)) + abs(right_grad_y.at<short>(i, j) - left_grad_y.at<short>(i, y + 1));
						cost_grad[3] = abs(right_grad_x.at<short>(i, j) - left_grad_x.at<short>(i + 1, y + 1)) + abs(right_grad_y.at<short>(i, j) - left_grad_y.at<short>(i + 1, y + 1));


						float cost[4];


						float cost_hamming[4];

						switch (hamming[0]) {
						case 0:
							cost_hamming[0] = 1;
							break;
						case 1:
							cost_hamming[0] = 0.9375;
							break;
						case 2:
							cost_hamming[0] = 0.875;
							break;
						case 3:
							cost_hamming[0] = 0.828125;
							break;
						case 4:
							cost_hamming[0] = 0.765625;
							break;
						case 5:
							cost_hamming[0] = 0.71875;
							break;
						case 6:
							cost_hamming[0] = 0.671875;
							break;
						case 7:
							cost_hamming[0] = 0.640625;
							break;
						case 8:
							cost_hamming[0] = 0.59375;
							break;
						case 9:
							cost_hamming[0] = 0.5625;
							break;
						}


						switch (hamming[1]) {
						case 0:
							cost_hamming[1] = 1;
							break;
						case 1:
							cost_hamming[1] = 0.9375;
							break;
						case 2:
							cost_hamming[1] = 0.875;
							break;
						case 3:
							cost_hamming[1] = 0.828125;
							break;
						case 4:
							cost_hamming[1] = 0.765625;
							break;
						case 5:
							cost_hamming[1] = 0.71875;
							break;
						case 6:
							cost_hamming[1] = 0.671875;
							break;
						case 7:
							cost_hamming[1] = 0.640625;
							break;
						case 8:
							cost_hamming[1] = 0.59375;
							break;
						case 9:
							cost_hamming[1] = 0.5625;
							break;
						}


						switch (hamming[2]) {
						case 0:
							cost_hamming[2] = 1;
							break;
						case 1:
							cost_hamming[2] = 0.9375;
							break;
						case 2:
							cost_hamming[2] = 0.875;
							break;
						case 3:
							cost_hamming[2] = 0.828125;
							break;
						case 4:
							cost_hamming[2] = 0.765625;
							break;
						case 5:
							cost_hamming[2] = 0.71875;
							break;
						case 6:
							cost_hamming[2] = 0.671875;
							break;
						case 7:
							cost_hamming[2] = 0.640625;
							break;
						case 8:
							cost_hamming[2] = 0.59375;
							break;
						case 9:
							cost_hamming[2] = 0.5625;
							break;
						}

						switch (hamming[3]) {
						case 0:
							cost_hamming[3] = 1;
							break;
						case 1:
							cost_hamming[3] = 0.9375;
							break;
						case 2:
							cost_hamming[3] = 0.875;
							break;
						case 3:
							cost_hamming[3] = 0.828125;
							break;
						case 4:
							cost_hamming[3] = 0.765625;
							break;
						case 5:
							cost_hamming[3] = 0.71875;
							break;
						case 6:
							cost_hamming[3] = 0.671875;
							break;
						case 7:
							cost_hamming[3] = 0.640625;
							break;
						case 8:
							cost_hamming[3] = 0.59375;
							break;
						case 9:
							cost_hamming[3] = 0.5625;
							break;
						}


						float cost_exponent[4];
						cost_exponent[0] = (float(0 - cost_grad[0]) / float(Grad));
						cost_exponent[1] = (float(0 - cost_grad[1]) / float(Grad));
						cost_exponent[2] = (float(0 - cost_grad[2]) / float(Grad));
						cost_exponent[3] = (float(0 - cost_grad[3]) / float(Grad));

						/////////////////////////////////////////////////////////////////////////////////////̩  չ    ˷ ̫       /////////////////////////////////////////////////////////////////////////////////////////////////////
						float cost_1_stage1[4];
						cost_1_stage1[0] = float(cost_exponent[0] * 0.008331298828125) + 0.02117919921875;
						cost_1_stage1[1] = float(cost_exponent[1] * 0.008331298828125) + 0.02117919921875;
						cost_1_stage1[2] = float(cost_exponent[2] * 0.008331298828125) + 0.02117919921875;
						cost_1_stage1[3] = float(cost_exponent[3] * 0.008331298828125) + 0.02117919921875;

						float cost_1_stage2[4];
						cost_1_stage2[0] = float(cost_exponent[0] * cost_1_stage1[0]) + 0.1034698486328125;
						cost_1_stage2[1] = float(cost_exponent[1] * cost_1_stage1[1]) + 0.1034698486328125;
						cost_1_stage2[2] = float(cost_exponent[2] * cost_1_stage1[2]) + 0.1034698486328125;
						cost_1_stage2[3] = float(cost_exponent[3] * cost_1_stage1[3]) + 0.1034698486328125;

						float cost_1_stage3[4];
						cost_1_stage3[0] = float(cost_exponent[0] * cost_1_stage2[0]) + 0.3025970458984375;
						cost_1_stage3[1] = float(cost_exponent[1] * cost_1_stage2[1]) + 0.3025970458984375;
						cost_1_stage3[2] = float(cost_exponent[2] * cost_1_stage2[2]) + 0.3025970458984375;
						cost_1_stage3[3] = float(cost_exponent[3] * cost_1_stage2[3]) + 0.3025970458984375;

						float cost_1_stage4[4];
						cost_1_stage4[0] = float(cost_exponent[0] * cost_1_stage3[0]) + 0.6065826416015625;
						cost_1_stage4[1] = float(cost_exponent[1] * cost_1_stage3[1]) + 0.6065826416015625;
						cost_1_stage4[2] = float(cost_exponent[2] * cost_1_stage3[2]) + 0.6065826416015625;
						cost_1_stage4[3] = float(cost_exponent[3] * cost_1_stage3[3]) + 0.6065826416015625;

						float cost_1_stage5[4];
						cost_1_stage5[0] = float(cost_exponent[0] * cost_1_stage4[0]) + 0.6065216064453125;
						cost_1_stage5[1] = float(cost_exponent[1] * cost_1_stage4[1]) + 0.6065216064453125;
						cost_1_stage5[2] = float(cost_exponent[2] * cost_1_stage4[2]) + 0.6065216064453125;
						cost_1_stage5[3] = float(cost_exponent[3] * cost_1_stage4[3]) + 0.6065216064453125;

						float cost_1[4];

						//taylor expansion
						/*cost_1[0] = cost_1_stage5[0] * 1.648712158203125 ;
						cost_1[1] = cost_1_stage5[1] * 1.648712158203125 ;
						cost_1[2] = cost_1_stage5[2] * 1.648712158203125 ;
						cost_1[3] = cost_1_stage5[3] * 1.648712158203125 ;*/

						//interpolation
						cost_1[0] = exponentialCost(cost_exponent[0]);
						cost_1[1] = exponentialCost(cost_exponent[1]);
						cost_1[2] = exponentialCost(cost_exponent[2]);
						cost_1[3] = exponentialCost(cost_exponent[3]);
						/////////////////////////////////////////////////////////////////////////////////////̩  չ    ˷ ̫       /////////////////////////////////////////////////////////////////////////////////////////////////////

						int bit = 3;
						float cost_temp[4];
						cost_temp[0] = 2 - cost_1[0] - cost_hamming[0];
						cost_temp[1] = 2 - cost_1[1] - cost_hamming[1];
						cost_temp[2] = 2 - cost_1[2] - cost_hamming[2];
						cost_temp[3] = 2 - cost_1[3] - cost_hamming[3];

						cost[0] = float(int(cost_temp[0] * (pow(2, bit)))) / (pow(2, bit));
						cost[1] = float(int(cost_temp[1] * (pow(2, bit)))) / (pow(2, bit));
						cost[2] = float(int(cost_temp[2] * (pow(2, bit)))) / (pow(2, bit));
						cost[3] = float(int(cost_temp[3] * (pow(2, bit)))) / (pow(2, bit));
						/// save the prefer position
						float ave = (cost[0] + cost[1] + cost[2] + cost[3]) / 4;
						cost_ave[x] = ave;
					}
				}
				float min = cost_ave[0];
				int mindex = 0;
				for (int i = 1; i < 4; i++) {
					if (min > cost_ave[i]) {
						min = cost_ave[i];
						mindex = i;
						cost_ave[i] = 0;
					}
				}
				B[height][width][k] = mindex;
				C[height][width][k] = min;
				cost_ave[0] = 0;
			}
			width++;
		}
		width = 0;
		height++;
	}
}
void calculatePixelCost4_R(Mat& leftimage_grad, Mat& rightimage_grad, Mat& leftimage, Mat& rightimage, Mat& left_grad_x, Mat& left_grad_y, Mat& right_grad_x, Mat& right_grad_y,
	float*** B, int disparityRange, float*** C, int Grad, int Ctg)
{
	Mat Left_census, Right_census;
	Left_census = ProcessImg(leftimage_grad);//      ͼ   õ   ͼ  CENSUSͼ   Left_census
	Right_census = ProcessImg(rightimage_grad);

	int ImgHeight = rightimage.rows;
	int ImgWidth = rightimage.cols;
	int height = 0;
	int width = 0;
	for (int i = 0; i < ImgHeight - 1; i = i++)
	{

		for (int j = 0; j < ImgWidth - 1; j = j + 2)
		{

			uchar R1;
			uchar R2;
			uchar R3;
			uchar R4;
			uchar L1;
			uchar L2;
			uchar L3;
			uchar L4;
			uchar diff1;
			uchar diff2;
			uchar diff3;
			uchar diff4;

			int x1 = i;
			int y1 = j;
			if ((x1 > ImgHeight - 2) || (y1 > ImgWidth - 2)) {
				x1 = ImgHeight - 2;
				y1 = ImgWidth - 2;
			}
			R1 = Right_census.at<uchar>(x1, y1);
			R2 = Right_census.at<uchar>(x1 + 1, y1);
			R3 = Right_census.at<uchar>(x1, y1 + 1);
			R4 = Right_census.at<uchar>(x1 + 1, y1 + 1);
			for (int k = 0; k < disparityRange / 4; k++)
			{
				float cost_ave[4]{};
				for (int x = 0; x < 4; x++)
				{
					int y = j + 4 * k + x;
					if (((i + 1) > ImgHeight - 1) || ((y + 1) > ImgWidth - 1) || ((y + 1) > ImgWidth - 1))
					{
						cost_ave[x] = Window_Size * Window_Size;
						//C[height][width][k] = 0;
					}
					else
					{
						L1 = Left_census.at<uchar>(i, y);
						L2 = Left_census.at<uchar>(i + 1, y);
						L3 = Left_census.at<uchar>(i, y + 1);
						L4 = Left_census.at<uchar>(i + 1, y + 1);


						diff1 = R1 ^ L1;
						diff2 = R2 ^ L2;
						diff3 = R3 ^ L3;
						diff4 = R4 ^ L4;

						int hamming[4];
						hamming[0] = GetHammingWeight(diff1);
						hamming[1] = GetHammingWeight(diff2);
						hamming[2] = GetHammingWeight(diff3);
						hamming[3] = GetHammingWeight(diff4);

						int cost_grad[4];
						cost_grad[0] = abs(right_grad_x.at<short>(i, j) - left_grad_x.at<short>(i, y)) + abs(right_grad_y.at<short>(i, j) - left_grad_y.at<short>(i, y));
						cost_grad[1] = abs(right_grad_x.at<short>(i, j) - left_grad_x.at<short>(i + 1, y)) + abs(right_grad_y.at<short>(i, j) - left_grad_y.at<short>(i + 1, y));
						cost_grad[2] = abs(right_grad_x.at<short>(i, j) - left_grad_x.at<short>(i, y + 1)) + abs(right_grad_y.at<short>(i, j) - left_grad_y.at<short>(i, y + 1));
						cost_grad[3] = abs(right_grad_x.at<short>(i, j) - left_grad_x.at<short>(i + 1, y + 1)) + abs(right_grad_y.at<short>(i, j) - left_grad_y.at<short>(i + 1, y + 1));


						float cost[4];


						float cost_hamming[4];

						switch (hamming[0]) {
						case 0:
							cost_hamming[0] = 1;
							break;
						case 1:
							cost_hamming[0] = 0.9375;
							break;
						case 2:
							cost_hamming[0] = 0.875;
							break;
						case 3:
							cost_hamming[0] = 0.828125;
							break;
						case 4:
							cost_hamming[0] = 0.765625;
							break;
						case 5:
							cost_hamming[0] = 0.71875;
							break;
						case 6:
							cost_hamming[0] = 0.671875;
							break;
						case 7:
							cost_hamming[0] = 0.640625;
							break;
						case 8:
							cost_hamming[0] = 0.59375;
							break;
						case 9:
							cost_hamming[0] = 0.5625;
							break;
						}


						switch (hamming[1]) {
						case 0:
							cost_hamming[1] = 1;
							break;
						case 1:
							cost_hamming[1] = 0.9375;
							break;
						case 2:
							cost_hamming[1] = 0.875;
							break;
						case 3:
							cost_hamming[1] = 0.828125;
							break;
						case 4:
							cost_hamming[1] = 0.765625;
							break;
						case 5:
							cost_hamming[1] = 0.71875;
							break;
						case 6:
							cost_hamming[1] = 0.671875;
							break;
						case 7:
							cost_hamming[1] = 0.640625;
							break;
						case 8:
							cost_hamming[1] = 0.59375;
							break;
						case 9:
							cost_hamming[1] = 0.5625;
							break;
						}


						switch (hamming[2]) {
						case 0:
							cost_hamming[2] = 1;
							break;
						case 1:
							cost_hamming[2] = 0.9375;
							break;
						case 2:
							cost_hamming[2] = 0.875;
							break;
						case 3:
							cost_hamming[2] = 0.828125;
							break;
						case 4:
							cost_hamming[2] = 0.765625;
							break;
						case 5:
							cost_hamming[2] = 0.71875;
							break;
						case 6:
							cost_hamming[2] = 0.671875;
							break;
						case 7:
							cost_hamming[2] = 0.640625;
							break;
						case 8:
							cost_hamming[2] = 0.59375;
							break;
						case 9:
							cost_hamming[2] = 0.5625;
							break;
						}

						switch (hamming[3]) {
						case 0:
							cost_hamming[3] = 1;
							break;
						case 1:
							cost_hamming[3] = 0.9375;
							break;
						case 2:
							cost_hamming[3] = 0.875;
							break;
						case 3:
							cost_hamming[3] = 0.828125;
							break;
						case 4:
							cost_hamming[3] = 0.765625;
							break;
						case 5:
							cost_hamming[3] = 0.71875;
							break;
						case 6:
							cost_hamming[3] = 0.671875;
							break;
						case 7:
							cost_hamming[3] = 0.640625;
							break;
						case 8:
							cost_hamming[3] = 0.59375;
							break;
						case 9:
							cost_hamming[3] = 0.5625;
							break;
						}


						float cost_exponent[4];
						cost_exponent[0] = (float(0 - cost_grad[0]) / float(Grad));
						cost_exponent[1] = (float(0 - cost_grad[1]) / float(Grad));
						cost_exponent[2] = (float(0 - cost_grad[2]) / float(Grad));
						cost_exponent[3] = (float(0 - cost_grad[3]) / float(Grad));

						/////////////////////////////////////////////////////////////////////////////////////̩  չ    ˷ ̫       /////////////////////////////////////////////////////////////////////////////////////////////////////
						float cost_1_stage1[4];
						cost_1_stage1[0] = float(cost_exponent[0] * 0.008331298828125) + 0.02117919921875;
						cost_1_stage1[1] = float(cost_exponent[1] * 0.008331298828125) + 0.02117919921875;
						cost_1_stage1[2] = float(cost_exponent[2] * 0.008331298828125) + 0.02117919921875;
						cost_1_stage1[3] = float(cost_exponent[3] * 0.008331298828125) + 0.02117919921875;

						float cost_1_stage2[4];
						cost_1_stage2[0] = float(cost_exponent[0] * cost_1_stage1[0]) + 0.1034698486328125;
						cost_1_stage2[1] = float(cost_exponent[1] * cost_1_stage1[1]) + 0.1034698486328125;
						cost_1_stage2[2] = float(cost_exponent[2] * cost_1_stage1[2]) + 0.1034698486328125;
						cost_1_stage2[3] = float(cost_exponent[3] * cost_1_stage1[3]) + 0.1034698486328125;

						float cost_1_stage3[4];
						cost_1_stage3[0] = float(cost_exponent[0] * cost_1_stage2[0]) + 0.3025970458984375;
						cost_1_stage3[1] = float(cost_exponent[1] * cost_1_stage2[1]) + 0.3025970458984375;
						cost_1_stage3[2] = float(cost_exponent[2] * cost_1_stage2[2]) + 0.3025970458984375;
						cost_1_stage3[3] = float(cost_exponent[3] * cost_1_stage2[3]) + 0.3025970458984375;

						float cost_1_stage4[4];
						cost_1_stage4[0] = float(cost_exponent[0] * cost_1_stage3[0]) + 0.6065826416015625;
						cost_1_stage4[1] = float(cost_exponent[1] * cost_1_stage3[1]) + 0.6065826416015625;
						cost_1_stage4[2] = float(cost_exponent[2] * cost_1_stage3[2]) + 0.6065826416015625;
						cost_1_stage4[3] = float(cost_exponent[3] * cost_1_stage3[3]) + 0.6065826416015625;

						float cost_1_stage5[4];
						cost_1_stage5[0] = float(cost_exponent[0] * cost_1_stage4[0]) + 0.6065216064453125;
						cost_1_stage5[1] = float(cost_exponent[1] * cost_1_stage4[1]) + 0.6065216064453125;
						cost_1_stage5[2] = float(cost_exponent[2] * cost_1_stage4[2]) + 0.6065216064453125;
						cost_1_stage5[3] = float(cost_exponent[3] * cost_1_stage4[3]) + 0.6065216064453125;

						float cost_1[4];

						//taylor expansion
						/*cost_1[0] = cost_1_stage5[0] * 1.648712158203125 ;
						cost_1[1] = cost_1_stage5[1] * 1.648712158203125 ;
						cost_1[2] = cost_1_stage5[2] * 1.648712158203125 ;
						cost_1[3] = cost_1_stage5[3] * 1.648712158203125 ;*/

						//interpolation
						cost_1[0] = exponentialCost(cost_exponent[0]);
						cost_1[1] = exponentialCost(cost_exponent[1]);
						cost_1[2] = exponentialCost(cost_exponent[2]);
						cost_1[3] = exponentialCost(cost_exponent[3]);
						/////////////////////////////////////////////////////////////////////////////////////̩  չ    ˷ ̫       /////////////////////////////////////////////////////////////////////////////////////////////////////

						int bit = 3;
						float cost_temp[4];
						cost_temp[0] = 2 - cost_1[0] - cost_hamming[0];
						cost_temp[1] = 2 - cost_1[1] - cost_hamming[1];
						cost_temp[2] = 2 - cost_1[2] - cost_hamming[2];
						cost_temp[3] = 2 - cost_1[3] - cost_hamming[3];

						cost[0] = float(int(cost_temp[0] * (pow(2, bit)))) / (pow(2, bit));
						cost[1] = float(int(cost_temp[1] * (pow(2, bit)))) / (pow(2, bit));
						cost[2] = float(int(cost_temp[2] * (pow(2, bit)))) / (pow(2, bit));
						cost[3] = float(int(cost_temp[3] * (pow(2, bit)))) / (pow(2, bit));
						/// save the prefer position
						float minp = cost[0];
						for (int i = 1; i < 4; i++) {
							if (minp > cost[i]) {
								minp = cost[i];
								cost[i] = 0;
							}
						}
						cost_ave[x] = minp;
					}
				}
				float min = cost_ave[0];
				int mindex = 0;
				for (int i = 1; i < 4; i++) {
					if (min > cost_ave[i]) {
						min = cost_ave[i];
						mindex = i;
						cost_ave[i] = 0;
					}
				}
				B[height][width][k] = mindex;
				C[height][width][k] = min;
				cost_ave[0] = 0;
			}
			width++;
		}
		width = 0;
		height++;
		float progress = height * 100.0 / (ImgHeight - 1);
		std::cout << "\rcalculatePixelCost4_R Progress: " << std::fixed << std::setprecision(2) << progress << "%" << std::flush;
	}
	std::cout << "\n" << std::endl;
}
void initializeFirstScanPaths(vector<path>& paths, unsigned short pathCount)
{
	/* 这里用了 4 个或者 8 个 */
	for (unsigned short i = 0; i < pathCount; ++i)
	{
		paths.push_back(path());
	}
	/**/
	if (paths.size() >= 1)
	{
		paths[0].rowDiff = 0;
		paths[0].colDiff = -1;
		paths[0].index = 1;
	}

	if (paths.size() >= 2)
	{
		paths[1].rowDiff = -1;
		paths[1].colDiff = 1;
		paths[1].index = 4;
	}
	/**/
	if (paths.size() >= 3)
	{
		paths[2].rowDiff = -1;
		paths[2].colDiff = 0;
		paths[2].index = 2;
	}
	if (paths.size() >= 4) {
		paths[3].rowDiff = -1;
		paths[3].colDiff = -1;
		paths[3].index = 7;
	}
	if (paths.size() >= 5) {
		paths[4].rowDiff = 0;
		paths[4].colDiff = 1;
		paths[4].index = 0;
	}
	if (paths.size() >= 6) {
		paths[5].rowDiff = 1;
		paths[5].colDiff = 0;
		paths[5].index = 5;
	}
	if (paths.size() >= 7) {
		paths[6].rowDiff = 1;
		paths[6].colDiff = 1;
		paths[6].index = 3;
	}
	if (paths.size() >= 8) {
		paths[7].rowDiff = 1;
		paths[7].colDiff = -1;
		paths[7].index = 6;
	}
}

/*
聚合代价计算
int row, int col,    像素的位置坐标
int d,               当前的视差值
path &p,             扫描路径
int rows, int cols,  图像的宽度和高速
int disparityRange,  视差范围
unsigned short ***C, 像素匹配时的 cost 值
unsigned short ***A  对应路径的 cost 值
*/
float aggregateCost(int row, int col, int d, path& p, int rows, int cols, int disparityRange, float*** C, float*** A)
{
	float aggregatedCost = 0;

	aggregatedCost += C[row][col][d]; /* 像素匹配的 cost 值 */

	/*if (DEBUG)
	{
		printf("{P%d}[%d][%d](d%d)\n", p.index, row, col, d);
	}*/

	/* 处理超出图像边缘外的情况 */
	if (row + p.rowDiff < 0 || row + p.rowDiff >= rows || col + p.colDiff < 0 || col + p.colDiff >= cols)
	{
		// border
		A[row][col][d] += aggregatedCost;
		return A[row][col][d];
	}

	/* 没有超出图像边缘外 */
	float minPrev, minPrevOther, prev, prevPlus, prevMinus;

	/* 初始值为最大值     */
	prev = minPrev = minPrevOther = prevPlus = prevMinus = MAX_SHORT;

	/*
	minPrev: 对应路径的视差代价最小值
	*/

	/* 在视差之间进行循环，理解其中的意思 */
	for (int disp = 0; disp < disparityRange / Groupnumber; ++disp)
	{
		float tmp = A[row + p.rowDiff][col + p.colDiff][disp];
		//找到这个路径下，前一个像素取不同disparity值时最小的A，这就是最后减去的那一项
		if (minPrev > tmp)
		{
			minPrev = tmp;
		}
		//前一个像素disparity取值为d是，其最小的A，这是公式中的第一项
		if (disp == d)    /* 视差与当前像素的视差相等 */
		{
			prev = tmp;
		}
		else if (disp == d + 1)  /* 视差与当前像素的视差差一 */   //这是公式中的第三项，最后加的惩罚系数P1
		{
			prevPlus = tmp;
		}
		else if (disp == d - 1) //这是公式中的第二项，最后加的惩罚系数P1
		{
			prevMinus = tmp;
		}
		else
		{
			if (minPrevOther > tmp + LARGE_PENALTY)
			{
				minPrevOther = tmp + LARGE_PENALTY;
			}
		}
	} // for (int disp = 0; disp < disparityRange; ++disp)

	/* 计算最小值 */
	aggregatedCost += min(min((float)prevPlus + (float)SMALL_PENALTY, (float)prevMinus + (float)SMALL_PENALTY), min((float)prev, (float)minPrevOther));
	aggregatedCost -= minPrev;

	A[row][col][d] += aggregatedCost;

	/*if (DEBUG)
	{
		printf("{P%d}[%d][%d](d%d)-> %d<CALCULATED>\n", p.index, row, col, d, A[row][col][d]);
	}*/



	return A[row][col][d];
}


float printProgress(unsigned int current, unsigned int max, int lastProgressPrinted) {
	int progress = floor(100 * current / (float)max);
	if (progress >= lastProgressPrinted + 5) {
		lastProgressPrinted = lastProgressPrinted + 5;
		cout << "\r" << lastProgressPrinted << "%" << std::flush;
	}
	return lastProgressPrinted;
}


/*
功能：代价聚合
int rows, int cols, ：图像的宽度和高度
disparityRange：      视差范围
unsigned short ***C   输入：每个像素的代价
unsigned short ****A  聚合路径
unsigned short ***S   聚合后的代价

path 结构体的定义

struct path {
	short rowDiff;
	short colDiff;
	short index;
};

说实在的，其中的物理意义并不清楚 for left
*/
void aggregateCosts(int rows, int cols, int disparityRange, float*** C, float**** A, float*** S)
{

	vector<path> firstScanPaths;
	vector<path> secondScanPaths;

	/* 初始化扫描路径           */
	/* #define PATHS_PER_SCAN 4 */
	initializeFirstScanPaths(firstScanPaths, PATHS_PER_SCAN);

	/*
				9       8
			13  7   2   4    15
				1       0
			12  6   3   5    14
			   11      10

		first : 1,2,4,7, 8, 9,13,15
		second:    0,3,5,6,10,11,12,14
	*/

	int lastProgressPrinted = 0;
	cout << "First scan..." << endl;
	/* 第一次扫描 */
	for (int row = 0; row < rows; ++row)
	{
		for (int col = 0; col < cols; ++col)
		{
			for (unsigned int path = 0; path < firstScanPaths.size(); ++path)
			{
				for (int d = 0; d < disparityRange / Groupnumber; ++d)
				{
					S[row][col][d] += aggregateCost(row, col, d, firstScanPaths[path], rows, cols, disparityRange, C, A[path]
					);//当前路径下当前像素的C+Lr,换路径后呢？问题就在这
				}
			}
		}
		lastProgressPrinted = printProgress(row, rows - 1, lastProgressPrinted);
	}
	std::cout << "" << endl;

	lastProgressPrinted = 0;
	//cout << "Second scan..." << endl;
	/* 第二次扫描，顺序与第一次扫描不一样 */
//    for (int row = rows - 1; row >= 0; --row)
//    {
//        for (int col = cols - 1; col >= 0; --col)
//        {
//            for (unsigned int path = 0; path < secondScanPaths.size(); ++path)
//            {
//                for (int d = 0; d < disparityRange/2; ++d)
//                {
//                    S[row][col][d] += aggregateCost(row, col, d, secondScanPaths[path], rows, cols, disparityRange, C, A[path]);
//                }
//            }
//        }
//        //打印进度条
//        lastProgressPrinted = printProgress(rows - 1 - row, rows - 1, lastProgressPrinted);
//    }
}

//For right

void computeDisparity_origin(float*** S, int rows, int cols, int disparityRange, Mat& disparityMap, float*** B)
{
	float disparity = 0, minCost, cost_1, cost_2, denom;
	float disparity1 = 0;
	float numer = 0; float deno = 1;
	for (int row = 0; row < rows; ++row)
	{
		for (int col = 0; col < cols; ++col)
		{
			if (row == 1 && col == 1) {
				int dpc = 1;
			}
			minCost = MAX_SHORT;
			for (int d = disparityRange / Groupnumber - 1; d >= 0; --d)
			{

				if (minCost > S[row][col][d])
				{
					minCost = S[row][col][d];
					//disparity = Groupnumber * d;
					disparity = Groupnumber * d + B[row][col][d];
				}
			}
			disparityMap.at<float>(row, col) = disparity;
		}
	}
	printf("disparity finish\n");
}
//void computeDisparity_subpixel_4(float*** G, int rows, int cols, int disparityRange, Mat& disparityMap, float*** B)
//{
//	float disparity = 0, minCost, cost_1, cost_2, denom;
//	float disparity1 = 0;
//	float denom2 = 0;
//	float numer = 0; float deno = 1;
//	for (int row = 0; row < rows; ++row)
//	{
//		for (int col = 0; col < cols; ++col)
//		{
//			if (row == 1 && col == 1) {
//				int dpc = 1;
//			}
//			minCost = MAX_SHORT;
//			for (int d = disparityRange / Groupnumber - 1; d >= 0; --d)
//			{
//				//printf("S:");
//				//printf("%p\n", S[row][col][d]);
//				if (minCost > G[row][col][d])
//				{
//					minCost = G[row][col][d];
//					//disparity = Groupnumber * d;
//					disparity = Groupnumber * d + B[row][col][d];
//					float x = 0;
//					float interpFunction = 0;
//
//					int th = disparityRange / Groupnumber - 1;
//					if (0 < d && d < th) {
//						//float leftDif = (4*S[row][col][d - 1]+ B[row][col][d-1]) - (4 * S[row][col][d] + B[row][col][d]);
//						//float rightDif = (4 * S[row][col][d + 1] + B[row][col][d + 1]) - (4 * S[row][col][d] + B[row][col][d]);
//						float leftDif = G[row][col][d - 1] - G[row][col][d];
//						float rightDif = G[row][col][d + 1] - G[row][col][d];
//						denom2 = (max((leftDif + rightDif), (float)1)) / 16;
//						interpFunction = (leftDif - rightDif + denom2) / (denom2 * 2);
//						if (interpFunction >= 0) {
//							disparity1 = Groupnumber * d + B[row][col][d] + interpFunction;
//						}
//						else {
//							disparity1 = Groupnumber * d + B[row][col][d];
//						}
//
//					}
//				}
//			}
//			//cout << "disp_afterinterpolation" << disparity1 << endl;
//			disparityMap.at<float>(row, col) = int(disparity1 * 2) / 2;
//			//printf("disp:");
//			//printf("%p\n",disparity1);
//		}
//	}
//	printf("disparity finish\n");
//}

void computeDisparity_subpixel_4(float*** G, int rows, int cols, int disparityRange, Mat& disparityMap, float*** B)
{
	float disparity = 0, minCost, cost_1, cost_2, denom;
	float disparity1 = 0;
	float denom2 = 0;
	float numer = 0; float deno = 1;
	for (int row = 0; row < rows; ++row)
	{
		for (int col = 0; col < cols; ++col)
		{
			//if (row == 1 && col == 1) {
			//	int dpc = 1;
			//}
			minCost = MAX_SHORT;
			for (int d = disparityRange / Groupnumber - 1; d >= 0; --d)
			{
				//printf("S:");
				//printf("%p\n", S[row][col][d]);
				if (minCost > G[row][col][d])
				{
					minCost = G[row][col][d];
					//disparity = Groupnumber * d;
					disparity = Groupnumber * d + B[row][col][d];
					float x = 0;
					float interpFunction = 0;

					int th = disparityRange / Groupnumber - 1;
					if (0 < d && d < th) {
						//float leftDif = (4*S[row][col][d - 1]+ B[row][col][d-1]) - (4 * S[row][col][d] + B[row][col][d]);
						//float rightDif = (4 * S[row][col][d + 1] + B[row][col][d + 1]) - (4 * S[row][col][d] + B[row][col][d]);
						float leftDif = G[row][col][d - 1] - G[row][col][d];
						float rightDif = G[row][col][d + 1] - G[row][col][d];
						denom2 = (max((leftDif + rightDif), (float)1)) / 16;
						interpFunction = (leftDif - rightDif + denom2) / (denom2 * 2);
						if (interpFunction >= 0) {
							disparity1 = Groupnumber * d + B[row][col][d] + interpFunction;
						}
						else {
							disparity1 = Groupnumber * d + B[row][col][d];
						}

					}
					else {
						disparity1 = Groupnumber * d + B[row][col][d];
					}
				}
			}
			//cout << "disp_afterinterpolation" << disparity1 << endl;
			disparityMap.at<float>(row, col) = int(disparity1 * 8) / 8;
			disparity1 = 0;
			//printf("disp:");
			//printf("%p\n",disparity1);
		}
	}
	printf("disparity finish\n");
}
void computeDisparity3(float*** S, int rows, int cols, int disparityRange, Mat& disparityMap, float*** B)
{
	float disparity = 0, minCost, cost_1, cost_2, denom;
	float disparity1 = 0;
	float numer = 0; float deno = 1;
	for (int row = 0; row < rows; ++row)
	{
		for (int col = 0; col < cols; ++col)
		{
			if (row == 1 && col == 1) {
				int dpc = 1;
			}
			minCost = MAX_SHORT;
			for (int d = disparityRange / Groupnumber - 1; d >= 0; --d)
			{
				//printf("S:");
				//printf("%p\n", S[row][col][d]);
				if (minCost > S[row][col][d])
				{
					minCost = S[row][col][d];
					//disparity = Groupnumber * d;
					disparity = Groupnumber * d + B[row][col][d];
					float x = 0;
					float interpFunction = 0;

					int th = disparityRange / Groupnumber - 1;
					if (0 < d && d < th) {
						//float leftDif = (4*S[row][col][d - 1]+ B[row][col][d-1]) - (4 * S[row][col][d] + B[row][col][d]);
						//float rightDif = (4 * S[row][col][d + 1] + B[row][col][d + 1]) - (4 * S[row][col][d] + B[row][col][d]);
						float leftDif = S[row][col][d - 1] - S[row][col][d];
						float rightDif = S[row][col][d + 1] - S[row][col][d];
						if (leftDif == 0) {
							x = 0;
						}
						else
						{
							x = rightDif / leftDif;
						}

						interpFunction = 0.5 - 0.5 * cos(4 * atan(1) * x / 2);//atan(1)=pi/4
						if ((interpFunction > 0) && (interpFunction < 1)) {
							if ((leftDif <= rightDif)) {
								disparity1 = Groupnumber * d + B[row][col][d] - 0.5 + interpFunction;
								//disparity1 = Groupnumber * d + B[row][col][d];
							}
							else
							{
								disparity1 = Groupnumber * d + B[row][col][d] + 0.5 - interpFunction;
								//disparity1 = Groupnumber * d + B[row][col][d];
							}

						}
						else
						{
							disparity1 = Groupnumber * d + B[row][col][d];

						}
					}
					else {
						disparity1 = Groupnumber * d + B[row][col][d];

					}
				}
			}
			disparityMap.at<float>(row, col) = disparity1;
			//printf("disp:");
			//printf("%p\n",disparity1);
		}
	}
	printf("disparity finish\n");
}

void compress_guideimage(cv::Mat guide, cv::Mat guide_compress)
{
	int hei = guide.rows;
	int wid = guide.cols;

	int hei_temp = 3 - fmod(hei, 3) + hei;
	int wid_temp = 3 - fmod(wid, 3) + wid;

	//Mat window;

	double max_window;
	double min_window;
	Point a, b;

	Mat guide_compress_temp = cv::Mat::zeros(hei_temp, wid_temp, CV_8U);
	for (int i = 0; i < hei; i++) {
		for (int j = 0; j < wid; j++) {
			guide_compress_temp.at<uchar>(i, j) = guide.at<uchar>(i, j);
		}
	}

	//guide_compress_temp(cv::Rect(0, 0, wid, hei)) = guide;

	for (int i = 0; i < hei_temp - 2; i = i + 3) {
		for (int j = 0; j < wid_temp - 2; j = j + 3) {
			vector<int> window = { guide_compress_temp.at<uchar>(i, j), guide_compress_temp.at<uchar>(i, j), guide_compress_temp.at<uchar>(i, j),
				 guide_compress_temp.at<uchar>(i, j), guide_compress_temp.at<uchar>(i, j), guide_compress_temp.at<uchar>(i, j),
				 guide_compress_temp.at<uchar>(i, j), guide_compress_temp.at<uchar>(i, j), guide_compress_temp.at<uchar>(i, j) };
			max_window = *max_element(window.begin(), window.end());
			min_window = *min_element(window.begin(), window.end());
			for (int x = 0; x < 3; x++) {
				for (int y = 0; y < 3; y++) {
					if (guide_compress_temp.at<uchar>(i + x, j + y) > (max_window / 2 + min_window / 2)) {
						guide_compress_temp.at<uchar>(i + x, j + y) = 1;
					}
					else {
						guide_compress_temp.at<uchar>(i + x, j + y) = 0;
					}
				}
			}
		}
	}

	for (int i = 0; i < hei; i++) {
		for (int j = 0; j < wid; j++) {
			guide_compress.at<uchar>(i, j) = guide_compress_temp.at<uchar>(i, j);
			//cout << "guide_compress_temp.at<uchar>(i, j)=" << int(guide_compress_temp.at<uchar>(i, j)) << endl;
		}
	}

	//guide_compress = guide_compress_temp(cv::Rect(0, 0, wid,hei));



}


cv::Mat GuidedFilter(cv::Mat I, cv::Mat p, int r, double eps)
{
	/*
	% GUIDEDFILTER   O(N) time implementation of guided filter.
	%
	%   - guidance image: I (should be a gray-scale/single channel image)
	%   - filtering input image: p (should be a gray-scale/single channel image)
	%   - local window radius: r
	%   - regularization parameter: eps
	*/

	cv::Mat _I;
	I.convertTo(_I, CV_32FC1, 1);
	I = _I;

	cv::Mat _p;
	p.convertTo(_p, CV_32FC1, 1);
	p = _p;

	//[hei, wid] = size(I);  
	int hei = I.rows;
	int wid = I.cols;

	r = 2 * r + 1;//因为opencv自带的boxFilter（）中的Size,比如9x9,我们说半径为4 
	int N;
	N = r * r;

	//mean_I = boxfilter(I, r) ./ N;  
	cv::Mat mean_I;
	cv::boxFilter(I, mean_I, CV_32FC1, cv::Size(r, r));

	cv::Mat sum_I;
	cv::boxFilter(I, sum_I, CV_32FC1, cv::Size(r, r), cv::Point(-1, -1), 0);


	//mean_p = boxfilter(p, r) ./ N;  
	cv::Mat mean_p;
	cv::boxFilter(p, mean_p, CV_32FC1, cv::Size(r, r));

	cv::Mat sum_p;
	cv::boxFilter(p, sum_p, CV_32FC1, cv::Size(r, r), cv::Point(-1, -1), 0);


	//mean_Ip = boxfilter(I.*p, r) ./ N;  
	cv::Mat mean_Ip;
	cv::boxFilter(I.mul(p), mean_Ip, CV_32FC1, cv::Size(r, r));


	cv::Mat sum_Ip;
	cv::boxFilter(I.mul(p), sum_Ip, CV_32FC1, cv::Size(r, r), cv::Point(-1, -1), 0);



	//cov_Ip = mean_Ip - mean_I .* mean_p; % this is the covariance of (I, p) in each local patch.  
	cv::Mat cov_Ip = mean_Ip - mean_I.mul(mean_p);

	cv::Mat cov_Ip_n2 = N * sum_Ip - sum_I.mul(sum_p);

	//mean_II = boxfilter(I.*I, r) ./ N;  
	cv::Mat mean_II;
	cv::boxFilter(I.mul(I), mean_II, CV_32FC1, cv::Size(r, r));

	cv::Mat sum_II;
	cv::boxFilter(I.mul(I), sum_II, CV_32FC1, cv::Size(r, r), cv::Point(-1, -1), 0);

	//var_I = mean_II - mean_I .* mean_I;  
	cv::Mat var_I = mean_II - mean_I.mul(mean_I);

	cv::Mat var_I_n2 = N * sum_II - sum_I.mul(sum_I);

	//a = cov_Ip ./ (var_I + eps); % Eqn. (5) in the paper;     
	cv::Mat a = cov_Ip / (var_I + eps);

	//cv::Mat a_n2 = cov_Ip_n2 /(var_I_n2 + eps*N*N);


	//0.5703125  2^-0.81 7bit二进制小数
	cv::Mat a_n2_temp = cv::Mat::zeros(hei, wid, CV_32F);
	cv::Mat a_n2 = cv::Mat::zeros(hei, wid, CV_32F);
	for (int i = 0; i < hei; i++) {
		for (int j = 0; j < wid; j++) {
			//a_n2.at<float>(i, j) = cov_Ip_n2.at<float>(i, j) * pow(2,-(var_I_n2.at<float>(i, j) + eps * N * N));

			//a_n2.at<float>(i, j) = cov_Ip_n2.at<float>(i, j) / (var_I_n2.at<float>(i, j) + eps * N * N);

			a_n2_temp.at<float>(i, j) = cov_Ip_n2.at<float>(i, j) * 0.5703125 * (pow(2, -(var_I_n2.at<float>(i, j))));

			//cost[0] = float(int(cost_temp[0] * (2 ^ (bit)))) / (2 ^ (bit));

			a_n2.at<float>(i, j) = float(int(a_n2_temp.at<float>(i, j)));

			/*cout << "a_n2_temp.at<float>(i, j)" << a_n2_temp.at<float>(i, j) << endl;
			cout << "a_n2.at<float>(i,j)=" << a_n2.at<float>(i, j) << endl;*/


			/*if ( int(var_I_n2.at<float>(i, j)) = 0) {
				a_n2.at<float>(i, j) = cov_Ip_n2.at<float>(i, j) / (var_I_n2.at<float>(i, j) + eps * N * N);
			}*/

			/*if (int(var_I_n2.at<float>(i, j)) == 14) {
				cout << "a_n2.at<float>(i, j)" << a_n2.at<float>(i, j) << endl;
			}*/
			//cout << "sum_I " << sum_I.at<float>(i, j)  << endl;
			//cout << "sum_p " << sum_p.at<float>(i, j) << endl;




		}
	}


	//b = mean_p - a .* mean_I; % Eqn. (6) in the paper;  
	cv::Mat b = mean_p - a.mul(mean_I);

	cv::Mat b_n3 = N * N * sum_p - a_n2.mul(sum_I);
	cv::Mat b_n3_temp = N * N * sum_p;

	/*for (int i = 0; i < hei; i++) {
		for (int j = 1; j < wid - 1; j++) {
			if (a_n2_temp.at<float>(i, j) >0.001) {
				cout << " sum_p=" << sum_p.at<float>(i, j) << endl;
				cout << " sum_I=" << sum_I.at<float>(i, j) << endl;
				cout << " a_n2_temp=" << a_n2_temp.at<float>(i, j) << endl;
				cout << " a_n2=" << a_n2.at<float>(i, j) << endl;
				cout << setiosflags" b_n3_temp=" << b_n3_temp.at<float>(i, j) << endl;
				cout << " b_n3=" << b_n3.at<float>(i, j) << endl;
			}
		}
	}*/


	int kk = b_n3.type();

	//mean_a = boxfilter(a, r) ./ N;  
	cv::Mat mean_a;
	cv::boxFilter(a, mean_a, CV_32FC1, cv::Size(r, r));

	//cv::Mat sum_a_n2;
	cv::Mat sum_a_n2 = cv::Mat::zeros(hei, wid, CV_32F);
	//cv::boxFilter(a_n2, sum_a_n2, CV_32FC1, cv::Size(r, r),cv::Point(-1, -1), 0);

	//mean_b = boxfilter(b, r) ./ N;  
	cv::Mat mean_b;
	cv::boxFilter(b, mean_b, CV_32FC1, cv::Size(r, r));

	//cv::Mat sum_b_n3;
	cv::Mat sum_b_n3 = cv::Mat::zeros(hei, wid, CV_32F);
	//cv::boxFilter(b_n3, sum_b_n3, CV_32FC1, cv::Size(r, r),cv::Point(-1, -1), 0);

	//q = mean_a .* I + mean_b; % Eqn. (8) in the paper;  
	for (int i = 0; i < hei; i++) {
		for (int j = 1; j < wid - 1; j++) {
			sum_a_n2.at<float>(i, j) = a_n2.at<float>(i, j - 1) + a_n2.at<float>(i, j) + a_n2.at<float>(i, j + 1);
		}
	}

	for (int i = 0; i < hei; i++) {
		for (int j = 1; j < wid - 1; j++) {
			sum_b_n3.at<float>(i, j) = b_n3.at<float>(i, j - 1) + b_n3.at<float>(i, j) + b_n3.at<float>(i, j + 1);
		}
	}

	cv::Mat q = mean_a.mul(I) + mean_b;

	cv::Mat Q = N * sum_a_n2.mul(I) + sum_b_n3;

	cv::Mat Q_temp = cv::Mat::zeros(hei, wid, CV_32F);
	for (int i = 0; i < hei; i++) {
		for (int j = 1; j < wid - 1; j++) {
			Q_temp.at<float>(i, j) = float(int(Q.at<float>(i, j) * (2 ^ (3)))) / (2 ^ (3));
			//cout << "Q.at<float>(i, j)=" << Q.at<float>(i, j) << endl;
			//cout << "Q_temp.at<float>(i, j)=" << Q_temp.at<float>(i, j) << endl;
		}
	}




	return Q_temp;
}

////////////////////////////guidedfilterfilter cost agg/////////////////////////////////
void AdaptiveWindowGuidedCostFilter(Mat& img_guided, float*** S, float*** C, int disparityRange)
{
	int ImgHeight = img_guided.rows;
	int ImgWidth = img_guided.cols;
	uchar center = 0;
	Mat img = img_guided;

	Mat img_filter = Mat(Size(ImgWidth, ImgHeight), CV_32FC1, Scalar::all(255));
	Mat img_out = Mat(Size(ImgWidth, ImgHeight), CV_32FC1, Scalar::all(255));

	for (int k = 0; k < disparityRange; k++) {


		for (int i = 0; i < ImgHeight; i++) {
			for (int j = 0; j < ImgWidth; j++) {
				img_filter.at<float>(i, j) = C[i][j][k];
			}
		}
		//cv::ximgproc::guidedFilter(img_guided, img_filter, img_out, step, 0.01, -1);
		img_out = GuidedFilter(img_guided, img_filter, 1, 0.01);

		for (int i = 0; i < ImgHeight; i++) {
			for (int j = 0; j < ImgWidth; j++) {
				S[i][j][k] = img_out.at<float>(i, j);
			}
		}
	}

}

void computeDisparity(float*** S, int rows, int cols, int disparityRange, Mat& disparityMap, float*** B)
{
	float disparity = 0, minCost, cost_1, cost_2, denom;
	float disparity1 = 0;
	float numer = 0; float deno = 1;
	for (int row = 0; row < rows; ++row)
	{
		for (int col = 0; col < cols; ++col)
		{
			if (row == 251 && col == 2) {
				int dpc = 1;
			}
			minCost = MAX_SHORT;
			for (int d = disparityRange / Groupnumber - 1; d >= 0; --d)
			{

				if (minCost > S[row][col][d])
				{
					minCost = S[row][col][d];
					//disparity = Groupnumber * d;
					disparity = Groupnumber * d + B[row][col][d];
					float x = 0;
					float interpFunction = 0;

					int th = disparityRange / Groupnumber - 1;
					if (0 < d && d < th) {
						float leftDif = S[row][col][d - 1] - S[row][col][d];
						float rightDif = S[row][col][d + 1] - S[row][col][d];
						if (leftDif == 0) {
							x = 0;
						}
						else
						{
							x = rightDif / leftDif;
						}

						interpFunction = (x * x + x) / 4;
						if ((interpFunction > 0) && (interpFunction < 1)) {
							if ((leftDif <= rightDif)) {

								disparity1 = Groupnumber * d + B[row][col][d] - 0.5 + interpFunction;
							}
							else
							{
								disparity1 = Groupnumber * d + B[row][col][d] + 0.5 - interpFunction;
							}
						}
						else
						{
							disparity1 = Groupnumber * d + B[row][col][d];
						}


					}
					else {
						disparity1 = Groupnumber * d + B[row][col][d];
					}




				}
			}
			disparityMap.at<float>(row, col) = disparity1 * 2;
		}
	}
	printf("disparity finish\n");
}

void computeDisparity2(float*** S, int rows, int cols, int disparityRange, Mat& disparityMap, float*** B)
{
	float disparity = 0, minCost, cost_1, cost_2, denom;
	float disparity1 = 0;
	float numer = 0; float deno = 1;
	for (int row = 0; row < rows; ++row)
	{
		for (int col = 0; col < cols; ++col)
		{
			if (row == 251 && col == 2) {
				int dpc = 1;
			}
			minCost = MAX_SHORT;
			for (int d = disparityRange / Groupnumber - 1; d >= 0; --d)
			{

				if (minCost > S[row][col][d])
				{
					minCost = S[row][col][d];
					int th = disparityRange / Groupnumber - 2;
					if ((col - 2 >= 0) && (col + 2 <= cols - 1)) {
						if (1 < d && d < th) {
							if ((2 * (S[row][col][d - 1] + S[row][col][d + 1]) - 4 * S[row][col][d]) != 0 && (S[row][col][d - 1] - S[row][col][d + 1] + S[row][col][d + 2] - S[row][col][d]) != 0 && (S[row][col][d + 1] - S[row][col][d - 1] + S[row][col][d - 2] - S[row][col][d]) != 0) {
								float left = (S[row][col][d - 1] - S[row][col][d + 1]) / (2 * (S[row][col][d - 1] + S[row][col][d + 1]) - 4 * S[row][col][d]);
								float dsubpixel = 0;
								float lamda = 0;
								if (S[row][col][d - 1] > S[row][col][d + 1]) {
									lamda = (S[row][col][d - 1] - S[row][col][d + 1]) / (S[row][col][d - 1] - S[row][col][d + 1] + S[row][col][d + 2] - S[row][col][d]);
									dsubpixel = 0.5 * (left + lamda);
								}
								else
								{
									lamda = (S[row][col][d - 1] - S[row][col][d + 1]) / (S[row][col][d + 1] - S[row][col][d - 1] + S[row][col][d - 2] - S[row][col][d]);
									dsubpixel = 0.5 * (left + lamda);
								}
								disparity1 = Groupnumber * d + B[row][col][d] + dsubpixel;
							}
							else
							{
								disparity1 = Groupnumber * d + B[row][col][d];
							}

						}
						else {
							disparity1 = Groupnumber * d + B[row][col][d];
						}
					}
					else
					{
						disparity1 = Groupnumber * d + B[row][col][d];
					}
				}
			}
			disparityMap.at<float>(row, col) = float(int(disparity1 * 2) / 2);
		}
	}
	printf("disparity finish\n");
}


void left_right_check(Mat& disparityMap, Mat& disparityMap_R, Mat& disparityMap_hole, Mat& occlusion)
{
	int row = disparityMap.rows;
	int col = disparityMap.cols;
	printf("disparity left_right_check start\n");
	for (int j = 0; j < row; j++) {
		for (int i = 0; i < col; i++) {
			if (j == 1 && i == 696) {
				int dpc = 0;
			}

			//printf("i:%d j:%d\n",i,j);
			float dL = disparityMap.at<float>(j, i);
			if ((i - dL) >= 0) {
				float dR = disparityMap_R.at<float>(j, i - dL);
				float value;
				if ((dL - dR) > 0) {
					value = dL - dR;
				}
				else {
					value = dR - dL;
				}
				if (value > 3) {
					occlusion.at<float>(j, i) = 0;
					disparityMap_hole.at<float>(j, i) = 0;
				}
				else {
					occlusion.at<float>(j, i) = 255;
					disparityMap_hole.at<float>(j, i) = disparityMap.at<float>(j, i);
				}
			}
			else
			{
				occlusion.at<float>(j, i) = 0;
			}

		}
	}

	printf("disparity left_right_check finish\n");
}
void left_right_check2(Mat& disparityMap, Mat& disparityMap_R, Mat& disparityMap_hole, Mat& occlusion, Mat& mismatches)
{
	int row = disparityMap.rows;
	int col = disparityMap.cols;
	for (int j = 0; j < row; j++) {
		for (int i = 0; i < col; i++) {
			float dL = disparityMap.at<float>(j, i);
			if ((i - dL) >= 0 && (i - dL) < col) {
				float dR = disparityMap_R.at<float>(j, i - dL);
				float value;
				if ((dL - dR) > 0) {
					value = dL - dR;
				}
				else {
					value = dR - dL;
				}
				if (value > 3) {
					if ((dL - dR) > 0) {
						occlusion.at<float>(j, i) = 0;
					}
					else {
						mismatches.at<float>(j, i) = 0;
					}
					disparityMap_hole.at<float>(j, i) = 0;
				}
				else {
					occlusion.at<float>(j, i) = 255;
					mismatches.at<float>(j, i) = 255;
					disparityMap_hole.at<float>(j, i) = disparityMap.at<float>(j, i) * 2;      //*2 for newfilling
				}
			}
			else
			{
				mismatches.at<float>(j, i) = 0;
				//disparityMap_hole.at<float>(j, i) = 0;
			}

		}
	}
	printf("disparity left_right_check finish\n");
}
void saveGradientImage(const std::string& filename, const Mat& gradientImage) {
	// 将梯度图转换为可视化格式
	double minVal, maxVal;
	minMaxLoc(gradientImage, &minVal, &maxVal); // 查找梯度图中的最小和最大值
	Mat draw;
	gradientImage.convertTo(draw, CV_8U, 255.0 / (maxVal - minVal), -minVal * 255.0 / (maxVal - minVal));
	imwrite(filename, draw);
}
void medianFilterOnGradientDifference1(Mat& disparityMap, Mat& disparityMap_R,
	const Mat& originalMap1, const Mat& originalMap2,
	int kernelSize, double threshold)
{
	// 计算四个图像的x和y方向梯度
	Mat gradXDisparity1, gradYDisparity1, gradXDisparity2, gradYDisparity2;
	Mat gradXOriginal1, gradYOriginal1, gradXOriginal2, gradYOriginal2;

	// 计算x方向梯度
	Sobel(disparityMap, gradXDisparity1, CV_32F, 1, 0);
	Sobel(disparityMap_R, gradXDisparity2, CV_32F, 1, 0);
	Sobel(originalMap1, gradXOriginal1, CV_32F, 1, 0);
	Sobel(originalMap2, gradXOriginal2, CV_32F, 1, 0);

	// 计算y方向梯度
	Sobel(disparityMap, gradYDisparity1, CV_32F, 0, 1);
	Sobel(disparityMap_R, gradYDisparity2, CV_32F, 0, 1);
	Sobel(originalMap1, gradYOriginal1, CV_32F, 0, 1);
	Sobel(originalMap2, gradYOriginal2, CV_32F, 0, 1);

	// 合并梯度
	Mat gradDisparity1, gradDisparity2, gradOriginal1, gradOriginal2;
	magnitude(gradXDisparity1, gradYDisparity1, gradDisparity1);
	magnitude(gradXDisparity2, gradYDisparity2, gradDisparity2);
	magnitude(gradXOriginal1, gradYOriginal1, gradOriginal1);
	magnitude(gradXOriginal2, gradYOriginal2, gradOriginal2);
	saveGradientImage("gradDisparity1.png", gradDisparity1);
	saveGradientImage("gradDisparity2.png", gradDisparity2);
	saveGradientImage("gradOriginal1.png", gradOriginal1);
	saveGradientImage("gradOriginal2.png", gradOriginal2);
	// 计算梯度差异
	Mat diff1 = abs(gradDisparity1 - gradOriginal1) > threshold;
	Mat diff2 = abs(gradDisparity2 - gradOriginal2) > threshold;

	// 中值滤波整个图像
	Mat filtered1, filtered2;
	medianBlur(disparityMap, filtered1, kernelSize);
	medianBlur(disparityMap_R, filtered2, kernelSize);

	// 只更新梯度差异较大的点
	for (int j = 0; j < disparityMap.rows; ++j) {
		for (int i = 0; i < disparityMap.cols; ++i) {
			if (diff1.at<uchar>(j, i)) {
				disparityMap.at<float>(j, i) = filtered1.at<float>(j, i);
			}
			if (diff2.at<uchar>(j, i)) {
				disparityMap_R.at<float>(j, i) = filtered2.at<float>(j, i);
			}
		}
	}
}



void medianFilterOnGradientDifference2(Mat& disparityMap, Mat& disparityMap_R,
	const Mat& originalMap1, const Mat& originalMap2,
	int kernelSize, int kernelSize2, double threshold, double threshold2)
{// 计算四个图像的x和y方向梯度
	Mat gradXDisparity1, gradYDisparity1, gradXDisparity2, gradYDisparity2;
	Mat gradXOriginal1, gradYOriginal1, gradXOriginal2, gradYOriginal2;

	// 计算x方向梯度
	Sobel(disparityMap, gradXDisparity1, CV_32F, 1, 0);
	Sobel(disparityMap_R, gradXDisparity2, CV_32F, 1, 0);
	Sobel(originalMap1, gradXOriginal1, CV_32F, 1, 0);
	Sobel(originalMap2, gradXOriginal2, CV_32F, 1, 0);

	// 计算y方向梯度
	Sobel(disparityMap, gradYDisparity1, CV_32F, 0, 1);
	Sobel(disparityMap_R, gradYDisparity2, CV_32F, 0, 1);
	Sobel(originalMap1, gradYOriginal1, CV_32F, 0, 1);
	Sobel(originalMap2, gradYOriginal2, CV_32F, 0, 1);

	// 合并梯度
	Mat gradDisparity1, gradDisparity2, gradOriginal1, gradOriginal2;
	magnitude(gradXDisparity1, gradYDisparity1, gradDisparity1);
	magnitude(gradXDisparity2, gradYDisparity2, gradDisparity2);
	magnitude(gradXOriginal1, gradYOriginal1, gradOriginal1);
	magnitude(gradXOriginal2, gradYOriginal2, gradOriginal2);
	saveGradientImage("gradDisparity1.png", gradDisparity1);
	saveGradientImage("gradDisparity2.png", gradDisparity2);
	saveGradientImage("gradOriginal1.png", gradOriginal1);
	saveGradientImage("gradOriginal2.png", gradOriginal2);
	// 计算梯度差异
	Mat diff1 = abs(gradDisparity1 - gradOriginal1) > threshold;
	Mat diff2 = abs(gradDisparity2 - gradOriginal2) > threshold;

	// 中值滤波整个图像
	Mat filtered1, filtered2;
	medianBlur(disparityMap, filtered1, kernelSize);
	medianBlur(disparityMap_R, filtered2, kernelSize);

	int halfSize = kernelSize2 / 2;

	for (int j = 0; j < disparityMap.rows; ++j) {
		for (int i = 0; i < disparityMap.cols; ++i) {
			float localGradDiff1 = 0.0f;
			float localGradDiff2 = 0.0f;

			// 计算当前像素与邻域内像素的梯度差异和
			for (int y = -halfSize; y <= halfSize; ++y) {
				for (int x = -halfSize; x <= halfSize; ++x) {
					int ny = j + y;
					int nx = i + x;
					if (ny >= 0 && ny < disparityMap.rows && nx >= 0 && nx < disparityMap.cols) {
						localGradDiff1 += abs(disparityMap.at<float>(j, i) - disparityMap.at<float>(ny, nx));
						localGradDiff2 += abs(disparityMap_R.at<float>(j, i) - disparityMap_R.at<float>(ny, nx));
					}
				}
			}
			// 检查是否超过阈值，并更新
			if (diff1.at<uchar>(j, i) && localGradDiff1 > threshold2 * (kernelSize2 * kernelSize2)) {
				disparityMap.at<float>(j, i) = filtered1.at<float>(j, i);
			}
			if (diff2.at<uchar>(j, i) && localGradDiff2 > threshold2 * (kernelSize2 * kernelSize2)) {
				disparityMap_R.at<float>(j, i) = filtered2.at<float>(j, i);
			}
		}
	}
}


void left_right_check_window(Mat& disparityMap, Mat& disparityMap_R, Mat& disparityMap_hole, Mat& occlusion, Mat& mismatches, int windowSize1, int windowSize2)
{
	int row = disparityMap.rows;
	int col = disparityMap.cols;
	for (int j = 0; j < row; j++) {
		for (int i = 0; i < col; i++) {
			float dL = disparityMap.at<float>(j, i);
			int windowStart1 = max(i - windowSize1 / 2, 0);
			int windowEnd1 = min(i + windowSize1 / 2, col - 1);
			int windowStart2 = max((int)(i - dL) - windowSize2 / 2, 0);
			int windowEnd2 = min((int)(i - dL) + windowSize2 / 2, col - 1);
			float value1 = disparityMap.at<float>(j, i);
			if (windowStart2 >= 0 && windowEnd2 < col) {
				float sum = 0;
				float value = 0;

				// 对窗口内的元素进行比较并计算总和
				for (int k = windowStart1; k <= windowEnd1; k++) {
					for (int l = windowStart2; l <= windowEnd2; l++) {
						//float value1 = disparityMap.at<float>(j, k);
						float value2 = disparityMap_R.at<float>(j, l);
						sum += abs(value1 - value2);
						value += value1 - value2;
					}
				}

				// 根据总和是否超过阈值来更新输出矩阵
				if (sum / (windowSize1 * windowSize2) > 8) {
					if ((value / (windowSize1 * windowSize2)) > 0) {
						occlusion.at<float>(j, i) = 0;
					}
					else {
						mismatches.at<float>(j, i) = 0;
					}
					disparityMap_hole.at<float>(j, i) = 0;
				}
				else {
					occlusion.at<float>(j, i) = 255;
					mismatches.at<float>(j, i) = 255;
					disparityMap_hole.at<float>(j, i) = disparityMap.at<float>(j, i) * 2; // *2 for new filling
				}
			}
			else {
				mismatches.at<float>(j, i) = 0;
			}
		}
	}
	printf("disparity left_right_check finish\n");
}
void left_right_check3(Mat& disparityMap, Mat& disparityMap_R, Mat& disparityMap_hole, Mat& occlusion, Mat& mismatches)
{
	int row = disparityMap.rows;
	int col = disparityMap.cols;
	for (int j = 0; j < row; j++) {
		for (int i = 0; i < col; i++) {
			float dL = disparityMap.at<float>(j, i);
			if ((i - dL / 2) >= 0 && (i - dL / 2) < col) {
				float dR = disparityMap_R.at<float>(j, (i - dL / 2));
				float value;
				if ((dL - dR) > 0) {
					value = dL - dR;
				}
				else {
					value = dR - dL;
				}
				if (value > 3) {
					if ((dL - dR) > 0) {
						occlusion.at<float>(j, i) = 0;
					}
					else {
						mismatches.at<float>(j, i) = 0;
					}
					disparityMap_hole.at<float>(j, i) = 0;
				}
				else {
					occlusion.at<float>(j, i) = 255;
					mismatches.at<float>(j, i) = 255;
					if (disparityMap.at<float>(j, i) * 2 > 255) {
						disparityMap_hole.at<float>(j, i) = 255;      //*2 for newfilling
					}
					else {
						disparityMap_hole.at<float>(j, i) = disparityMap.at<float>(j, i) * 2;
					}
				}
			}
			else
			{
				mismatches.at<float>(j, i) = 0;
				disparityMap_hole.at<float>(j, i) = 0;
			}

		}
	}
	printf("disparity left_right_check finish\n");
}

void filling(Mat& disparityMap, Mat& disparityMap_R, Mat& occlusion, Mat& disparity_final) {
	int row = disparityMap.rows;
	int col = disparityMap.cols;
	int th = 0;
	printf("filling start\n");
	for (int j = 0; j < row; j++) {
		for (int i = 0; i < col; i++) {
			if (occlusion.at<float>(j, i) == 0) {
				int dpc = 0;
				float L = 255;
				float R = 255;
				for (int m = i; m >= 0; m--) {
					if (occlusion.at<float>(j, m) != 0) {
						L = disparityMap.at<float>(j, m);
						break;
					}
				}
				for (int n = i; n < col; n++) {
					if (occlusion.at<float>(j, n) != 0) {
						R = disparityMap.at<float>(j, n);
						break;
					}
				}
				disparity_final.at<float>(j, i) = min(L, R);
			}
			else {
				disparity_final.at<float>(j, i) = disparityMap.at<float>(j, i);
			}
		}
	}

	printf("filling finish\n");
}

// filling method 1: 5 directional filling
void filling2(Mat& disparityMap, Mat& disparityMap_R, Mat& occlusion, Mat& mismatches, Mat& disparity_final) {

	int row = disparityMap.rows;
	int col = disparityMap.cols;


	for (int j = 0; j < row; j++) {
		for (int i = 0; i < col; i++) {
			//先找误匹配，取中值


			if (occlusion.at < float >(j, i) == 0) {
				//再找遮挡，取次最小值

				float a = 0;//0
				float b = 0;//45
				float c = 0;//90
				float d = 0;//135
				float e = 0;//180
				float f = 0;//225
				float g = 0;//270
				float h = 0;//315

				for (int m = i; m >= 0; m--) {
					if (mismatches.at<float>(j, m) != 0 && occlusion.at<float>(j, m) != 0) {
						a = disparityMap.at<float>(j, m);
						break;
					}
				}

				//for (int m = 0; m <= min(j, i); m++) {
				for (int m = 0; m <= min(j, i); m++) {
					if (mismatches.at<float>(j - m, i - m) != 0 && occlusion.at<float>(j - m, i - m) != 0) {
						b = disparityMap.at<float>(j - m, i - m);
						break;
					}

				}
				for (int n = j; n >= 0; n--) {
					if (mismatches.at<float>(n, i) != 0 && occlusion.at<float>(n, i) != 0) {
						c = disparityMap.at<float>(n, i);
						break;
					}

				}

				for (int m = 0; m <= min(j, col - i - 1); m++) {
					if (mismatches.at<float>(j - m, i + m) != 0 && occlusion.at<float>(j - m, i + m) != 0) {
						d = disparityMap.at<float>(j - m, i + m);
						break;
					}
				}

				for (int m = i; m < col; m++) {
					if (mismatches.at<float>(j, m) != 0 && occlusion.at<float>(j, m) != 0) {
						e = disparityMap.at<float>(j, m);
						break;
					}
				}

				float min1 = a; float min2 = a;
				if (b < min1) {
					min2 = min1;
					min1 = b;
				}
				if (c < min1) {
					min2 = min1;
					min1 = c;
				}
				if (d < min1) {
					min2 = min1;
					min1 = d;
				}
				if (e < min1) {
					min2 = min1;
					min1 = e;
				}

				disparity_final.at<float>(j, i) = min2;

			}

			if (mismatches.at<float>(j, i) == 0) {
				float a = 0;//0
				float b = 0;//45
				float c = 0;//90
				float d = 0;//135
				float e = 0;//180

				for (int m = i; m >= 0; m--) {
					if (mismatches.at<float>(j, m) != 0 && occlusion.at<float>(j, m) != 0) {
						a = disparityMap.at<float>(j, m);
						break;
					}
				}

				for (int m = 0; m <= min(j, i); m++) {
					if (mismatches.at<float>(j - m, i - m) != 0 && occlusion.at<float>(j - m, i - m) != 0) {
						b = disparityMap.at<float>(j - m, i - m);
						break;
					}

				}
				for (int n = j; n >= 0; n--) {
					if (mismatches.at<float>(n, i) != 0 && occlusion.at<float>(n, i) != 0) {
						c = disparityMap.at<float>(n, i);
						break;
					}

				}

				for (int m = 0; m <= min(j, col - i - 1); m++) {
					if (mismatches.at<float>(j - m, i + m) != 0 && occlusion.at<float>(j - m, i + m) != 0) {
						d = disparityMap.at<float>(j - m, i + m);
						break;
					}
				}
				for (int m = i; m < col; m++) {
					if (mismatches.at<float>(j, m) != 0 && occlusion.at<float>(j, m) != 0) {
						e = disparityMap.at<float>(j, m);
						break;
					}
				}

				float n[5];
				n[0] = a; n[1] = b; n[2] = c; n[3] = d; n[4] = e;
				int temp;
				for (int p = 0; p < 5; p++) {                  //共进行9步
					for (int q = 0; q < 4 - p; q++) {      //在每一步进行10-i次两两比较
						if (n[q] > n[q + 1]) {
							temp = n[q];
							n[q] = n[q + 1];
							n[q + 1] = temp;
						}
					}
				}

				disparity_final.at<float>(j, i) = n[2];
			}

			if (occlusion.at<float>(j, i) != 0 && mismatches.at<float>(j, i) != 0) {
				disparity_final.at<float>(j, i) = disparityMap.at<float>(j, i);
			}

		}
	}
	printf("filling finish\n");
}


void filling3(Mat& disparityMap, Mat& occlusion, Mat& mismatches, Mat& disparity_final, int th_cnt = 25, float th_ratio = 0.0625) {
	int row = disparityMap.rows;
	int col = disparityMap.cols;
	int radius = 7;
	int win_cnt = (2 * radius + 1) * (2 * radius + 1);
	float* V;  // valid disp array
	V = new float[Disparity_Range * 2];
	int disp = 0;
	int disp_sum = 0;

	for (int j = 0; j < row; j = j + 1) {
		for (int i = 0; i < col; i = i + 1) {
			//initialization
			for (int k = 0; k < Disparity_Range * 2; k++) {
				V[k] = 0;
			}
			disp_sum = 0;

			if (disparityMap.at < float >(j, i) == 0) {
				//if (occlusion.at < float >(j, i) == 0 || mismatches.at < float >(j, i) == 0) {


					// find 121 valid disps
				for (int m = j - radius; m <= j + radius; m++) {
					for (int n = i - radius; n <= i + radius; n++) {
						if (m >= 0 && m < row && n >= 0 && n < col) {
							if (disparityMap.at<float>(m, n) != 0) {
								disp = int(disparityMap.at<float>(m, n));
								disp_sum += 1;
								V[disp] += 1;

							}
						}
					}
				}



				float max_value = V[0];
				int max_index = 0;
				// 遍历数组以找到最大值及其索引
				for (int k = 1; k < Disparity_Range * 2; k++) {
					if (V[k] > max_value) {
						max_value = V[k];
						max_index = k;
					}
				}

				if (disp_sum > th_cnt && max_value > th_ratio * disp_sum) {
					/*cout << "disp_sum = " << disp_sum << endl;
					cout << "max_value = " << max_value << endl;*/
					disparity_final.at<float>(j, i) = max_index;
					//cout << max_index << endl;
				}

				else {
					disparity_final.at<float>(j, i) = disparityMap.at<float>(j, i);
				}

			}


			else {
				disparity_final.at<float>(j, i) = disparityMap.at<float>(j, i);
			}


		}
	}

}

void saveSegmented(const Mat& labels, const string& filename) {
	// 创建颜色表
	vector<Vec3b> colors;
	for (int label = 0; label < labels.rows * labels.cols; label++) {
		colors.push_back(Vec3b((uchar)(rand() & 255), (uchar)(rand() & 255), (uchar)(rand() & 255)));
	}

	// 分配彩色图像
	Mat dst(labels.size(), CV_8UC3);
	for (int r = 0; r < labels.rows; ++r) {
		for (int c = 0; c < labels.cols; ++c) {
			int label = labels.at<int>(r, c);
			Vec3b& pixel = dst.at<Vec3b>(r, c);
			pixel = colors[label];
		}
	}

	// 保存图像
	imwrite(filename, dst);
}

void segmentImage(const Mat& src, Mat& dst, int K) {
	// 确保源图像是灰度图
	Mat graySrc;
	if (src.channels() == 3) {
		cvtColor(src, graySrc, COLOR_BGR2GRAY);
	}
	else {
		graySrc = src.clone();
	}

	// 将图像转换为 k-means 能处理的格式
	Mat samples(graySrc.rows * graySrc.cols, 1, CV_32F);
	for (int y = 0; y < graySrc.rows; y++) {
		for (int x = 0; x < graySrc.cols; x++) {
			samples.at<float>(y + x * graySrc.rows, 0) = graySrc.at<uchar>(y, x);
		}
	}

	// 应用 k-means 聚类
	Mat labels;
	Mat centers;
	kmeans(samples, K, labels, TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 10, 1.0), 3, KMEANS_PP_CENTERS, centers);

	// 创建聚类结果图像
	dst = Mat(graySrc.size(), graySrc.type());
	for (int y = 0; y < graySrc.rows; y++) {
		for (int x = 0; x < graySrc.cols; x++) {
			int cluster_idx = labels.at<int>(y + x * graySrc.rows);
			dst.at<uchar>(y, x) = static_cast<uchar>(centers.at<float>(cluster_idx, 0));
		}
	}
}


void filling_window2_dominant_segment_only(Mat& disparityMap, Mat& originMap, Mat& occlusion, Mat& mismatches, Mat& disparity_final, int window_size) {
	if (window_size % 2 == 0) {
		window_size += 1;
	}

	int row = disparityMap.rows;
	int col = disparityMap.cols;
	int half_window = window_size / 2;


	Mat segmentMap;

	int K = 3; // 分割类别数量
	segmentImage(originMap, segmentMap, K);

	// 保存分割后的图像
	imwrite("segmented_first.png", segmentMap);

	for (int j = 0; j < row; j++) {
		for (int i = 0; i < col; i++) {
			if (occlusion.at<float>(j, i) == 0 && mismatches.at<float>(j, i) == 0) {
				unordered_map<int, int> segmentCounts;
				vector<float> disparitiesOfDominantSegment;

				for (int wj = -half_window; wj <= half_window; ++wj) {
					for (int wi = -half_window; wi <= half_window; ++wi) {
						int yy = j + wj;
						int xx = i + wi;
						if (yy >= 0 && yy < row && xx >= 0 && xx < col) {
							int segment = segmentMap.at<uchar>(yy, xx);
							segmentCounts[segment]++;
						}
					}
				}

				int dominantSegment = max_element(segmentCounts.begin(), segmentCounts.end(),
					[](const auto& a, const auto& b) { return a.second < b.second; })->first;

				for (int wj = -half_window; wj <= half_window; ++wj) {
					for (int wi = -half_window; wi <= half_window; ++wi) {
						int yy = j + wj;
						int xx = i + wi;
						if (yy >= 0 && yy < row && xx >= 0 && xx < col) {
							if (segmentMap.at<uchar>(yy, xx) == dominantSegment) {
								disparitiesOfDominantSegment.push_back(disparityMap.at<float>(yy, xx));
							}
						}
					}
				}

				float disparityValue = 0.0f;
				if (!disparitiesOfDominantSegment.empty()) {
					nth_element(disparitiesOfDominantSegment.begin(),
						disparitiesOfDominantSegment.begin() + disparitiesOfDominantSegment.size() / 2,
						disparitiesOfDominantSegment.end());
					disparityValue = disparitiesOfDominantSegment[disparitiesOfDominantSegment.size() / 2];
				}
				else {
					disparityValue = 0; // 或者选择一个默认值
				}

				disparity_final.at<float>(j, i) = disparityValue;
			}
			else {
				disparity_final.at<float>(j, i) = disparityMap.at<float>(j, i);
			}
		}
	}
	printf("dominant segment only filling finish\n");
}


void filling_window2(Mat& disparityMap, Mat& disparityMap_R, Mat& occlusion, Mat& mismatches, Mat& disparity_final, int window_size) {
	// 确保window_size是奇数
	if (window_size % 2 == 0) {
		window_size += 1;
	}

	int row = disparityMap.rows;
	int col = disparityMap.cols;
	int half_window = window_size / 2;

	for (int j = 0; j < row; j++) {
		for (int i = 0; i < col; i++) {
			if (j < half_window || i < half_window || j >= row - half_window || i >= col - half_window) {
				disparity_final.at<float>(j, i) = disparityMap.at<float>(j, i);
			}
			else {
				// 为当前点定义一个窗口
				std::vector<float> valid_disparities;
				for (int wj = -half_window; wj <= half_window; ++wj) {
					for (int wi = -half_window; wi <= half_window; ++wi) {
						if (mismatches.at<float>(j + wj, i + wi) != 0 && occlusion.at<float>(j + wj, i + wi) != 0) {
							valid_disparities.push_back(disparityMap.at<float>(j + wj, i + wi));
						}
					}
				}

				// 计算中值
				float median = 0.0f;
				if (!valid_disparities.empty()) {
					size_t n = valid_disparities.size() / 2;
					std::nth_element(valid_disparities.begin(), valid_disparities.begin() + n, valid_disparities.end());
					median = valid_disparities[n];
					if (valid_disparities.size() % 2 == 0) {
						std::nth_element(valid_disparities.begin(), valid_disparities.begin() + n - 1, valid_disparities.end());
						median = 0.5f * (median + valid_disparities[n - 1]);
					}
				}

				// 根据情况设置最终的视差值
				if (occlusion.at<float>(j, i) == 0) {
					disparity_final.at<float>(j, i) = median;
					occlusion.at<float>(j, i) == 255;
				}
				else if (mismatches.at<float>(j, i) == 0) {
					disparity_final.at<float>(j, i) = median;
					mismatches.at<float>(j, i) == 255;
				}
				else {
					disparity_final.at<float>(j, i) = disparityMap.at<float>(j, i);
				}
			}
		}
	}
	printf("filling finish\n");
}


void filling_window(Mat& disparityMap, Mat& disparityMap_R, Mat& occlusion, Mat& mismatches, Mat& disparity_final) {

	int row = disparityMap.rows;
	int col = disparityMap.cols;

	for (int j = 0; j < row; j++) {
		for (int i = 0; i < col; i++) {
			//先找误匹配，取中值

			float a;//0
			float b;//45
			float c;//90
			float d;//135
			float e;//180
			float f;//225
			float g;//270
			float h;//315

			if (j == 0 || i == 0 || j == row - 1 || i == col - 1) {
				disparity_final.at<float>(j, i) = disparityMap.at<float>(j, i);
			}
			else {
				a = disparityMap.at<float>(j, i - 1);
				b = disparityMap.at<float>(j - 1, i - 1);
				c = disparityMap.at<float>(j - 1, i);
				d = disparityMap.at<float>(j - 1, i + 1);
				e = disparityMap.at<float>(j, i);

				int cnt_a, cnt_b, cnt_c, cnt_d, cnt_e;

				if (mismatches.at<float>(j, i - 1) != 0 && occlusion.at<float>(j, i - 1) != 0)
				{
					cnt_a = 1;
				}
				else { cnt_a = 0; }


				if (mismatches.at<float>(j - 1, i - 1) != 0 && occlusion.at<float>(j - 1, i - 1) != 0)
				{
					cnt_b = 1;
				}
				else { cnt_b = 0; }



				if (mismatches.at<float>(j - 1, i) != 0 && occlusion.at<float>(j - 1, i) != 0)
				{
					cnt_c = 1;
				}
				else { cnt_c = 0; }



				if (mismatches.at<float>(j - 1, i + 1) != 0 && occlusion.at<float>(j - 1, i + 1) != 0)
				{
					cnt_d = 1;
				}
				else { cnt_d = 0; }



				if (mismatches.at<float>(j, i) != 0 && occlusion.at<float>(j, i) != 0)
				{
					cnt_e = 1;
				}
				else { cnt_e = 0; }


				int cnt = cnt_a + cnt_b + cnt_c + cnt_d + cnt_e;

				float n[5];
				n[0] = a; n[1] = b; n[2] = c; n[3] = d; n[4] = e;
				int temp;
				for (int p = 0; p < 5; p++) {                  //共进行9步
					for (int q = 0; q < 4 - p; q++) {      //在每一步进行10-i次两两比较
						if (n[q] > n[q + 1]) {
							temp = n[q];
							n[q] = n[q + 1];
							n[q + 1] = temp;
						}
					}
				}
				// n[0]~n[4] 由小到大

				float median;
				float min2;

				if (cnt == 1)
				{
					median = n[4]; min2 = n[4];
				}
				else if (cnt == 2) {
					median = n[4]; min2 = n[4];
				}
				else if (cnt == 3) {
					median = n[3]; min2 = n[3];
				}
				else if (cnt == 4) {
					median = n[3]; min2 = n[2];
				}
				else if (cnt == 5) {
					median = n[2]; min2 = n[1];
				}
				else {
					median = n[0]; min2 = n[0];
				}


				if (occlusion.at < float >(j, i) == 0) { disparity_final.at<float>(j, i) = min2; }

				if (mismatches.at<float>(j, i) == 0) { disparity_final.at<float>(j, i) = median; }

				if (occlusion.at<float>(j, i) != 0 && mismatches.at<float>(j, i) != 0) {
					disparity_final.at<float>(j, i) = disparityMap.at<float>(j, i);
				}

			}
		}
	}
	printf("filling finish\n");
}

void saveDisparityMap(cv::Mat& disparityMap, int disparityRange, char* outputFile)
{
	//double factor = 256.0 / disparityRange;
	double factor = 1;
	for (int row = 0; row < disparityMap.rows; ++row)
	{
		for (int col = 0; col < disparityMap.cols; ++col)
		{
			disparityMap.at<float>(row, col) *= factor;
		}
	}
	Mat temp;
	//cv::medianBlur(disparityMap, temp, 3);
	//grayscaleGaussianBlur(temp, temp, 3);
	imwrite(outputFile, temp);
}

void writePFM1(Mat pfm, string path, float scale = -1 / 255.0)
{
	ofstream out(path + "/disp0censgm.pfm", ios_base::binary);

	out << "Pf" << endl << pfm.cols << " " << pfm.rows << endl << scale << endl;
	for (int i = pfm.rows - 1; i >= 0; i--)
		for (int j = 0; j < pfm.cols; j++)
			out.write((const char*)(&pfm.at<float>(i, j)), sizeof(float));

	out.close();

}

void filling1(Mat& disparityMap, Mat& disparity_out, int th_cnt) {
	int row = disparityMap.rows;
	int col = disparityMap.cols;
	for (int j = 0; j < row; j++) {
		for (int i = 0; i < col; i++) {
			//先找误匹配，取中值
			if (disparityMap.at < float >(j, i) == 0) {
				//再找遮挡，取次最小值

				float a = 0;//0
				float b = 0;//45
				float c = 0;//90
				float d = 0;//135
				float e = 0;//180
				float f = 0;//225
				float g = 0;//270
				float h = 0;//315

				for (int m = i; m >= 0; m--) {
					if (disparityMap.at<float>(j, m) != 0) {
						a = disparityMap.at<float>(j, m);
						break;
					}
				}

				//for (int m = 0; m <= min(j, i); m++) {
				for (int m = 0; m <= min(j, i); m++) {
					if (disparityMap.at<float>(j - m, i - m) != 0) {
						b = disparityMap.at<float>(j - m, i - m);
						break;
					}

				}
				for (int n = j; n >= 0; n--) {
					if (disparityMap.at<float>(n, i) != 0) {
						c = disparityMap.at<float>(n, i);
						break;
					}

				}

				for (int m = 0; m <= min(j, col - i - 1); m++) {
					if (disparityMap.at<float>(j - m, i + m) != 0) {
						d = disparityMap.at<float>(j - m, i + m);
						break;
					}
				}

				for (int m = i; m < col; m++) {
					if (disparityMap.at<float>(j, m) != 0) {
						e = disparityMap.at<float>(j, m);
						break;
					}
				}

				float x[5];
				x[0] = a;
				x[1] = b;
				x[2] = c;
				x[3] = d;
				x[4] = e;
				for (int mm = 0; mm < 4; mm++)
				{
					// 具体冒泡的方式：用相邻的两个元素进行比较，前一个大于后一个元素时，交换着两个数据，依次直到数组的末尾
					for (int nn = 1; nn < 5 - mm; nn++)
					{
						if (x[nn - 1] > x[nn])
						{
							float temp = x[nn - 1];
							x[nn - 1] = x[nn];
							x[nn] = temp;
						}
					}
				}


				//float median = (a + b + c + d + e) / 5;
				float median = x[2];


				// 找最小值
				float min1 = a; float min2 = a;
				if (b < min1) {
					min2 = min1;
					min1 = b;
				}
				if (c < min1) {
					min2 = min1;
					min1 = c;
				}
				if (d < min1) {
					min2 = min1;
					min1 = d;
				}
				if (e < min1) {
					min2 = min1;
					min1 = e;
				}



				if (abs(x[0] - x[2]) < th_cnt) {
					disparity_out.at<float>(j, i) = median;
				}
				else
				{
					disparity_out.at<float>(j, i) = disparityMap.at<float>(j, i);
				}

			}
			else {
				disparity_out.at<float>(j, i) = disparityMap.at<float>(j, i);
			}

		}

	}

}

void filling_lr(Mat& disparityMap, Mat& disparity_out, int th_cnt) {
	int row = disparityMap.rows;
	int col = disparityMap.cols;
	//medianBlur(disparityMap, disparityMap, 3);

	//disparity_out = disparityMap;
	for (int j = 0; j < row; j++) {
		for (int i = 0; i < col; i++) {
			//先找误匹配，取中值
			if (disparityMap.at < float >(j, i) == 0) {
				//再找遮挡，取次最小值

				float a = 0;//0
				float b = 0;//45
				float c = 0;//90
				float d = 0;//135
				float e = 0;//180
				float f = 0;//225
				float g = 0;//270
				float h = 0;//315

				for (int m = i; m >= 0; m--) {
					if (disparityMap.at<float>(j, m) != 0) {
						a = disparityMap.at<float>(j, m);
						break;
					}
				}

				//for (int m = 0; m <= min(j, i); m++) {
				for (int m = 0; m <= min(j, i); m++) {
					if (disparityMap.at<float>(j - m, i - m) != 0) {
						b = disparityMap.at<float>(j - m, i - m);
						break;
					}
				}

				if (abs(a - b) < th_cnt) {
					disparity_out.at<float>(j, i) = min(a, b);
				}
				else
				{
					disparity_out.at<float>(j, i) = 0;
				}

			}
			else {
				disparity_out.at<float>(j, i) = disparityMap.at<float>(j, i);
			}

		}

	}

}

string getFileName(const string& path) {
	// Find the last slash in the path
	size_t lastSlash = path.find_last_of("/\\");
	// If there is no slash, the whole path is the filename
	if (lastSlash == string::npos) return path;
	// Return the substring from the character after the last slash to the end of the string
	return path.substr(lastSlash + 1);
}

cv::Mat median_filter(const cv::Mat& input, int window_size) {

	std::cout << "Generate Point Cloud" << std::endl;
	CV_Assert(window_size % 2 == 1);     // 确保窗口大小为奇数

	int pad = window_size / 2;
	cv::Mat output = cv::Mat::zeros(input.size(), input.type());

	// 处理每一个像素点
	for (int i = pad; i < input.rows - pad; i++) {
		for (int j = pad; j < input.cols - pad; j++) {
			std::vector<float> window;

			// 收集窗口内的所有像素值
			for (int wi = -pad; wi <= pad; wi++) {
				for (int wj = -pad; wj <= pad; wj++) {
					float val = input.at<float>(i + wi, j + wj);
					window.push_back(val);
				}
			}

			// 计算中值
			std::nth_element(window.begin(), window.begin() + window.size() / 2, window.end());
			float median = window[window.size() / 2];

			// 设置输出像素值
			output.at<float>(i, j) = median;
		}
	}

	return output;
}

cv::Mat stereo_sgm::main(std::string img_L_paths, std::string img_R_paths, const std::string& outputDir) {


		Mat firstImage = imread(img_L_paths, IMREAD_GRAYSCALE);
		Mat secondImage = imread(img_R_paths, IMREAD_GRAYSCALE);
		// Mat lrcImage = imread(lrcImgPaths[i], IMREAD_GRAYSCALE);
		// lrcImage.convertTo(lrcImage, CV_32FC1);
		if (!firstImage.data || !secondImage.data)
		{
			cerr << "Could not open or find one of the images!" << endl;
			return secondImage;
		}

		int col_len = firstImage.cols;
		int row_len = firstImage.rows;
		unsigned int disparityRange = Disparity_Range;

		float*** C;  // pixel cost array W x H x D
		float*** G;  // pixel cost array W x H x D
		float*** S;  // aggregated cost array W x H x D
		float**** A; // single path cost array 2 x W x H x D
		float*** B; // to determin the final disparity position

		//For right
		float*** C_R;  // pixel cost array W x H x D
		float*** G_R;  // pixel cost array W x H x D
		float*** S_R;  // aggregated cost array W x H x D
		float**** A_R; // single path cost array 2 x W x H x D
		float*** B_R; // to determin the final disparity position
		clock_t begin = clock();

		cout << "Allocating space..." << endl;

		// allocate cost arrays for left
		C = new float** [row_len];
		G = new float** [row_len];
		S = new float** [row_len];
		B = new float** [(row_len)];

		for (int row = 0; row < row_len; ++row)
		{
			C[row] = new float* [col_len];
			S[row] = new float* [col_len]();
			G[row] = new float* [col_len]();
			B[row] = new float* [(col_len)]();

			for (int col = 0; col < col_len; ++col)
			{
				C[row][col] = new float[disparityRange / Groupnumber];
				S[row][col] = new float[disparityRange / Groupnumber](); // initialize to 0
				G[row][col] = new float[disparityRange / Groupnumber](); // initialize to 0
				B[row][col] = new float[(disparityRange / Groupnumber)]();
			}
		}
		// allocate cost arrays for right
		C_R = new float** [row_len];
		S_R = new float** [row_len];
		G_R = new float** [row_len];
		B_R = new float** [(row_len)];

		for (int row = 0; row < row_len; ++row)
		{
			C_R[row] = new float* [col_len];
			S_R[row] = new float* [col_len]();
			G_R[row] = new float* [col_len]();
			B_R[row] = new float* [(col_len)]();

			for (int col = 0; col < col_len; ++col)
			{
				C_R[row][col] = new float[disparityRange / Groupnumber];
				S_R[row][col] = new float[disparityRange / Groupnumber](); // initialize to 0
				G_R[row][col] = new float[disparityRange / Groupnumber](); // initialize to 0
				B_R[row][col] = new float[(disparityRange / Groupnumber)]();
			}
		}
		A = new float*** [PATHS_PER_SCAN];
		for (int path = 0; path < PATHS_PER_SCAN; ++path)
		{
			A[path] = new float** [row_len];
			for (int row = 0; row < row_len; ++row)
			{
				A[path][row] = new float* [col_len];
				for (int col = 0; col < col_len; ++col)
				{
					A[path][row][col] = new float[disparityRange / Groupnumber];
					for (unsigned int d = 0; d < disparityRange / Groupnumber; ++d)
					{
						A[path][row][col][d] = 0;
					}
				}
			}
		}
		A_R = new float*** [PATHS_PER_SCAN];
		for (int path = 0; path < PATHS_PER_SCAN; ++path)
		{
			A_R[path] = new float** [row_len];
			for (int row = 0; row < row_len; ++row)
			{
				A_R[path][row] = new float* [col_len];
				for (int col = 0; col < col_len; ++col)
				{
					A_R[path][row][col] = new float[disparityRange / Groupnumber];
					for (unsigned int d = 0; d < disparityRange / Groupnumber; ++d)
					{
						A_R[path][row][col][d] = 0;
					}
				}
			}
		}
		cout << "Smoothing images..." << endl;
		cout << "left_clahe and right_clahe......" << endl;
		Mat left_clahe = firstImage.clone();
		Mat right_clahe = secondImage.clone();

		blur(left_clahe, left_clahe, Size(3, 3));

		blur(right_clahe, right_clahe, Size(3, 3));

		Mat left_grad_x, left_grad_y, right_grad_x, right_grad_y;

		////利用sobel算子计算x方向和y方向梯度           scharr 可理解为加强的sobel算子
		Sobel(left_clahe, left_grad_x, CV_16S, 1, 0);
		Sobel(left_clahe, left_grad_y, CV_16S, 0, 1);

		Sobel(right_clahe, right_grad_x, CV_16S, 1, 0);
		Sobel(right_clahe, right_grad_y, CV_16S, 0, 1);

		Mat left_grad_x_abs, left_grad_y_abs, right_grad_x_abs, right_grad_y_abs;
		left_grad_x_abs = abs(left_grad_x);
		left_grad_y_abs = abs(left_grad_y);
		right_grad_x_abs = abs(right_grad_x);
		right_grad_y_abs = abs(right_grad_y);

		Mat left_grad, right_grad;

		left_grad = left_grad_x + left_grad_y;
		right_grad = right_grad_x + right_grad_y;
		int Grad = GRAD;
		int Ctg = CTg;
		cout << "calculatePixelCost for left......" << endl;
		calculatePixelCost(left_grad, right_grad, firstImage, secondImage, left_grad_x, left_grad_y, right_grad_x, right_grad_y, B, disparityRange, C, Grad, Ctg);
		cout << "calculatePixelCost for right......" << endl;
		calculatePixelCost_R(left_grad, right_grad, firstImage, secondImage, left_grad_x, left_grad_y, right_grad_x, right_grad_y, B_R, disparityRange, C_R, Grad, Ctg);

		cout << "Aggregating costs..." << endl;

		aggregateCosts(row_len, col_len, disparityRange, C, A, S);

		Mat disparityMap = Mat(Size(col_len, row_len), CV_32FC1, Scalar::all(255));

		aggregateCosts(row_len, col_len, disparityRange, C_R, A_R, S_R);

		Mat disparityMap_R = Mat(Size(col_len, row_len), CV_32FC1, Scalar::all(255));
		Mat disparityMap_hole = Mat(Size(col_len, row_len), CV_32FC1, Scalar::all(255));

		cout << "Computing disparity..." << endl;
		//agg
		computeDisparity_subpixel_4(S, row_len, col_len, disparityRange, disparityMap, B);
		computeDisparity_subpixel_4(S_R, row_len, col_len, disparityRange, disparityMap_R, B_R);
		clock_t end = clock();
		float elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
		Mat occlusion = Mat(Size(col_len, row_len), CV_32FC1, Scalar::all(255));
		Mat mismatches = Mat(Size(col_len, row_len), CV_32FC1, Scalar::all(255));
		left_right_check_window(disparityMap, disparityMap_R, disparityMap_hole, occlusion, mismatches, 1, 5);

		Mat disparity_final_s = Mat(Size(col_len, row_len), CV_32FC1, Scalar::all(255));
		Mat disparity_final_s0 = Mat(Size(col_len, row_len), CV_32FC1, Scalar::all(255));
		Mat disparity_final_int = Mat(Size(col_len, row_len), CV_16UC1, Scalar::all(255));
		Mat disparity_final_int0 = Mat(Size(col_len, row_len), CV_16UC1, Scalar::all(255));
		Mat disparity_final_0 = Mat(Size(col_len, row_len), CV_32FC1, Scalar::all(255));

		// filling
		filling2(disparityMap_hole, disparityMap_R, occlusion, mismatches, disparity_final_s);
		filling_window2(disparityMap_hole, disparityMap_R, occlusion, mismatches, disparity_final_s0, 3);

		for (int row = 0; row < disparity_final_s.rows; ++row)
		{
			for (int col = 0; col < disparity_final_s.cols; ++col)
			{
				int disp = disparity_final_s.at<float>(row, col);
				disparity_final_int.at<ushort>(row, col) = disp / 2;
				disparity_final_0.at<float>(row, col) = disparity_final_s.at<float>(row, col)/2;
				int disp0 = disparity_final_s0.at<float>(row, col);
				disparity_final_int0.at<ushort>(row, col) = disp0 / 2;
			}
		}
		convertScaleAbs(disparity_final_int, disparity_final_int);
		convertScaleAbs(disparity_final_int0, disparity_final_int0);
		string base_filename1 = getFileName(img_L_paths);

		string outputFilePath0 = outputDir + "disp_wo_mf_11_filling0" + base_filename1;
		string outputFilePath1 = outputDir + "disp_wo_mf_11_filling1" + base_filename1;

		imwrite(outputFilePath0, disparity_final_int0);
		imwrite(outputFilePath1, disparity_final_int);

		cv::medianBlur(disparity_final_int, disparity_final_int, 11);
		cv::medianBlur(disparity_final_int0, disparity_final_int0, 11);
		disparity_final_0 =  median_filter(disparity_final_0, 11);

		string outputFilePath_mf0 = outputDir + "disp_w_mf_11_filling0" + base_filename1;
		string outputFilePath_mf1 = outputDir + "disp_w_mf_11_filling1" + base_filename1;
		imwrite(outputFilePath_mf0, disparity_final_int0);
		//imwrite(outputFilePath_mf1, disparity_final_int);
		imwrite(outputDir + "first" + base_filename1, firstImage);
		imwrite(outputDir + "second"+ base_filename1, secondImage);

		cout << base_filename1 << " saved" << endl;
		// Free memory for left
		cout << "destroy space..." << endl;
		for (int path = 0; path < PATHS_PER_SCAN; ++path) {
			for (int row = 0; row < row_len; ++row) {
				for (int col = 0; col < col_len; ++col) {
					delete[] A[path][row][col];
				}
				delete[] A[path][row];
			}
			delete[] A[path];
		}
		delete[] A;

		for (int row = 0; row < row_len; ++row) {
			for (int col = 0; col < col_len; ++col) {
				delete[] C[row][col];
				delete[] S[row][col];
				delete[] G[row][col];
				delete[] B[row][col];
			}
			delete[] C[row];
			delete[] S[row];
			delete[] G[row];
			delete[] B[row];
		}
		delete[] C;
		delete[] S;
		delete[] G;
		delete[] B;

		// Free memory for right
		for (int path = 0; path < PATHS_PER_SCAN; ++path) {
			for (int row = 0; row < row_len; ++row) {
				for (int col = 0; col < col_len; ++col) {
					delete[] A_R[path][row][col];
				}
				delete[] A_R[path][row];
			}
			delete[] A_R[path];
		}
		delete[] A_R;

		for (int row = 0; row < row_len; ++row) {
			for (int col = 0; col < col_len; ++col) {
				delete[] C_R[row][col];
				delete[] S_R[row][col];
				delete[] G_R[row][col];
				delete[] B_R[row][col];
			}
			delete[] C_R[row];
			delete[] S_R[row];
			delete[] G_R[row];
			delete[] B_R[row];
		}
		delete[] C_R;
		delete[] S_R;
		delete[] G_R;
		delete[] B_R;

	
	return disparity_final_int;
}

cv::Mat stereo_sgm::sgbm_main(std::string img_L_paths, std::string img_R_paths)
{
	std::cout << "StereoSGBM" << std::endl;

	cv::Mat img1_rectified_LC = cv::imread(img_L_paths);
	cv::Mat img2_rectified_LC = cv::imread(img_R_paths);
	// 将矫正后的图像转换为灰度格式
	cv::Mat img1_rectified_gray, img2_rectified_gray;
	cv::cvtColor(img1_rectified_LC, img1_rectified_gray, cv::COLOR_BGR2GRAY);
	cv::cvtColor(img2_rectified_LC, img2_rectified_gray, cv::COLOR_BGR2GRAY);

	// 创建立体匹配对象
	int minDisparity = 0;
	int numDisparities = 16 * 16; // 必须是 16 的整数倍
	int blockSize = 7; // 调整以适应您的需求
	int P1 = 600;
	int P2 = 2400;
	int disp12MaxDiff = 2;
	int preFilterCap = 0;
	int uniquenessRatio = 1;
	int speckleWindowSize = 3;
	int speckleRange = 3;
	int mode = StereoSGBM::MODE_SGBM;
	cv::Ptr<cv::StereoSGBM> sgbm = cv::StereoSGBM::create(minDisparity, numDisparities, blockSize, P1, P2, disp12MaxDiff, preFilterCap, uniquenessRatio, speckleWindowSize = 0, speckleRange, mode);

	// 设置其他参数，根据需要调整
	// ...

	// 计算视差图
	cv::Mat disparity_sgbm;
	sgbm->compute(img1_rectified_gray, img2_rectified_gray, disparity_sgbm);

	// 将视差图转换为 0 到 255 的范围
	cv::Mat disp_sgbm_norm;
	double minVal, maxVal;
	std::cout << "minVal = " << minVal << " maxVal = " << maxVal << std::endl;
	cv::minMaxLoc(disparity_sgbm, &minVal, &maxVal);
	disparity_sgbm.convertTo(disp_sgbm_norm, CV_8UC1, 255 / (maxVal - minVal));

	// 显示视差图
	//cv::imshow("Disparity SGBM", disp_sgbm_norm);

	//cv::imshow("Original Images_LC", combinedImage_origin_LC);
	//cv::imshow("Rectified Images_LC", combinedImage_LC);

	//cv::waitKey(0);
	//cv::destroyAllWindows();
	
	return disp_sgbm_norm;

}

cv::Mat stereo_sgm::Disp2PCloud(const cv::Mat& disparityMap, float focalLength, float baseline, float cx, float cy) {
	Mat PointCloud = Mat(Size(disparityMap.cols, disparityMap.rows), CV_32FC3, Scalar::all(0));
	if (disparityMap.empty()) {
		std::cerr << "Error: disparity map is empty." << std::endl;
		return PointCloud;
	}
	std::cout << "Generate Point Cloud" << std::endl;
	double minVal, maxVal;
	cv::minMaxLoc(disparityMap, &minVal, &maxVal);
	std::cout << "disparity: minVal = " << minVal << " maxVal = " << maxVal << std::endl;
	for (int i = 0; i < disparityMap.rows; i++) {
		for (int j = 0; j < disparityMap.cols; j++) {
			float disp = static_cast<float>(disparityMap.at<float>(i, j));
			if (disp == 0) continue;
			float Z = ((focalLength * baseline) / disp) / 1000.0f ;
			if (abs(Z) > 20)
				Z = 0;
			float X = (j - cx ) * Z / focalLength;
			float Y = (cy - i ) * Z / focalLength;

			PointCloud.at<Vec3f>(i, j) = Vec3f(X, Y, -Z);
		}
	}
	//performMLS(PointCloud);
	std::cout << "focalLength = " << focalLength << " baseline = " << baseline << std::endl;
	std::cout << "cx = " << cx << " cy = " << cy << std::endl;
	return PointCloud;
}

void stereo_sgm::Mat2PCL(const cv::Mat& image, cv::Mat& pointCloud, const std::string& outputFilename) {
	// Assuming pointCloud is a CV_32FC3 Mat
	if (image.empty()) {
		std::cerr << "Error: image is empty." << std::endl;
		return;
	}
	std::cout << "Save Point Cloud from Mat" << std::endl;
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZRGB>);

	for (int i = 0; i < pointCloud.rows; i++) {
		for (int j = 0; j < pointCloud.cols; j++) {
			const cv::Vec3f& pt = pointCloud.at<cv::Vec3f>(i,j);
			if (pt == cv::Vec3f(0, 0, 0)) continue;
			pcl::PointXYZRGB point;
			point.x = pt[0];
			point.y = pt[1];
			point.z = pt[2];
			cv::Vec3b color = image.at<cv::Vec3b>(i, j);
			uint32_t rgb = (static_cast<uint32_t>(color[2]) << 16 |
				static_cast<uint32_t>(color[1]) << 8 | static_cast<uint32_t>(color[0]));
			point.rgb = *reinterpret_cast<float*>(&rgb);

			cloud->points.push_back(point);
		}
	}

	cloud->width = cloud->points.size();
	cloud->height = 1;
	cloud->is_dense = false;
	string outdir = "E:\\Studyproj\\24Proj\\meinong\\scene0421\\jvyuan_jvli\\100cm\\";
	pcl::io::savePCDFileASCII(outputFilename, *cloud);
	std::cout << "Saved " << cloud->points.size() << " data points to " << outputFilename << std::endl;


	////MLS algorithm
	//pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_Gray(new pcl::PointCloud<pcl::PointXYZ>);
	//pcl::copyPointCloud(*cloud, *cloud_Gray);
	//pcl::io::savePCDFileASCII(outdir+"cloud_Gray.pcd", *cloud_Gray);
	//// 创建一个KD树
	//pcl::search::KdTree<pcl::PointXYZ>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZ>);
	//// 输出文件中有PointNormal类型，用来存储移动最小二乘法算出的法线
	//pcl::PointCloud<pcl::PointNormal> mls_points;
	//// 定义对象 (第二种定义类型是为了存储法线, 即使用不到也需要定义出来)
	//pcl::MovingLeastSquares<pcl::PointXYZ, pcl::PointNormal> mls;
	//mls.setComputeNormals(true);
	////设置参数
	//mls.setInputCloud(cloud_Gray);
	//mls.setPolynomialOrder(2);
	////mls.setPolynomialFit(true);//设置为true则在平滑过程中采用多项式拟合来提高精度
	//mls.setSearchMethod(tree);
	//mls.setSearchRadius(0.03);
	//// 曲面重建
	//mls.process(mls_points);
	//// 保存结果
	//pcl::io::savePCDFileASCII("bun0-mls.pcd", mls_points);
	//std::cout << "Saved Kdtree" << std::endl;
	//pcl::PCLPointCloud2 blob;
	//pcl::io::loadPCDFile("E:\\Studyproj\\24Proj\\meinong\\scene0421\\jvyuan_jvli\\100cm\\cloud_Gray.pcd", blob);

}

void stereo_sgm::performMLS(cv::Mat& pointCloud) {

	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);

	for (int i = 0; i < pointCloud.rows; i++) {
		for (int j = 0; j < pointCloud.cols; j++) {
			const cv::Vec3f& pt = pointCloud.at<cv::Vec3f>(i, j);
			if (pt == cv::Vec3f(0, 0, 0)) continue;
			pcl::PointXYZ point;
			point.x = pt[0];
			point.y = pt[1];
			point.z = pt[2];

			cloud->points.push_back(point);
		}
	}
	//pcl::visualization::PCLVisualizer::Ptr viewer(new pcl::visualization::PCLVisualizer("Viewer"));
	//viewer->setBackgroundColor(0, 0, 0);
	//// 原始点云
	//viewer->addPointCloud<pcl::PointXYZ>(cloud, "original cloud");
	//viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_COLOR, 1, 0, 0, "original cloud");
	//// 平滑点云
	//while (!viewer->wasStopped()) {
	//	viewer->spinOnce(100);
	//}
	//MLS algorithm
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_Gray(new pcl::PointCloud<pcl::PointXYZ>);
	pcl::copyPointCloud(*cloud, *cloud_Gray);
	pcl::io::savePCDFileASCII("cloud_Gray.pcd", *cloud_Gray);
	// 创建一个KD树
	pcl::search::KdTree<pcl::PointXYZ>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZ>);
	// 输出文件中有PointNormal类型，用来存储移动最小二乘法算出的法线
	pcl::PointCloud<pcl::PointNormal> mls_points;
	// 定义对象 (第二种定义类型是为了存储法线, 即使用不到也需要定义出来)
	pcl::MovingLeastSquares<pcl::PointXYZ, pcl::PointNormal> mls;
	mls.setComputeNormals(true);
	//设置参数
	mls.setInputCloud(cloud_Gray);
	mls.setPolynomialOrder(2);
	//mls.setPolynomialFit(true);//设置为true则在平滑过程中采用多项式拟合来提高精度
	mls.setSearchMethod(tree);
	mls.setSearchRadius(0.03);
	// 曲面重建
	mls.process(mls_points);
	// 保存结果
	pcl::io::savePCDFileASCII("bun0-mls.pcd", mls_points);
	std::cout << "Saved Kdtree" << std::endl;
	 
	 
	// 
	//std::cout << "performMLS" << std::endl;
	//std::cout << "pointCloud.dim= " << pointCloud.dims<<std::endl;
	//if ( pointCloud.type() != CV_32FC3) {
	//	std::cout << "convert data type of pointCloud " << std::endl;
	//	pointCloud.convertTo(pointCloud, CV_32FC3); // 转换数据类型
	//}
	////if (pointCloud.rows > 1 && pointCloud.channels() == 3) {
	////	std::cout << "reshape the data sequence of pointCloud " << std::endl;
	////	pointCloud = pointCloud.reshape(3, pointCloud.rows * pointCloud.cols);
	////}
	//std::cout << "pointCloud.dim= " << pointCloud.dims << std::endl;
	//cv::Mat flatPointCloud(pointCloud.rows * pointCloud.cols,3, CV_32FC1);
	//std::cout << "flatPointCloud.cols= " << flatPointCloud.cols << std::endl;
	//if (1) {
	//	// 将每个点的 x, y, z 值拆分为单独的行
	//	
	//	int idx = 0;
	//	for (int i = 0; i < pointCloud.rows; i++) {
	//		for (int j = 0; j < pointCloud.cols; j++) {
	//			cv::Vec3f point = pointCloud.at<cv::Vec3f>(i, j);
	//			flatPointCloud.at<float>(idx,0) = point[0];
	//			flatPointCloud.at<float>(idx,1) = point[1];
	//			flatPointCloud.at<float>(idx,2) = point[2];
	//			idx++;
	//		}
	//	}
	//}
	//else {
	//	flatPointCloud = pointCloud;
	//}
	//// Assuming pointCloud is a CV_32FC3 Mat
	//cv::flann::KDTreeIndexParams indexParams;
	//cv::flann::Index kdtree(flatPointCloud, indexParams);
	//std::cout << "flatPointCloud.dim= " << flatPointCloud.dims << std::endl;

	//float searchRadius = 300.0; // Search radius for neighbors
	//int maxNeighbors = 100;     // Maximum number of neighbors to consider

	//// Output smoothed point cloud
	//cv::Mat newPointCloud(pointCloud.rows, pointCloud.cols, CV_32FC3);

	//for (int i = 0; i < newPointCloud.rows; i++) {
	//	for (int j = 0; j < newPointCloud.cols; j++){
	//		std::vector<float> query(3);
	//		query[0] = flatPointCloud.at<float>(i * newPointCloud.rows + j, 0);
	//		query[1] = flatPointCloud.at<float>(i * newPointCloud.rows + j, 1);
	//		query[2] = flatPointCloud.at<float>(i * newPointCloud.rows + j, 2);

	//		// Vector to store indices of the nearest neighbors
	//		std::vector<int> indices;
	//		// Vector to store distances to the nearest neighbors
	//		std::vector<float> dists;

	//		kdtree.radiusSearch(query, indices, dists, searchRadius, maxNeighbors);

	//		// Calculate the mean of the neighbors
	//		cv::Vec3f mean(0, 0, 0);
	//		for (int idx : indices) {
	//			mean += pointCloud.at<cv::Vec3f>(idx);
	//		}
	//		if (!indices.empty()) {  // 防止除以零的情况
	//			mean[0] /= indices.size();  // 分别处理x, y, z分量
	//			mean[1] /= indices.size();
	//			mean[2] /= indices.size();
	//		}

	//		// Place the mean in the new point cloud
	//		newPointCloud.at<cv::Vec3f>(i,j) = mean;
	//	}
	//}

	//// Copy the smoothed cloud back to the original
	////pointCloud = newPointCloud.reshape(3, pointCloud.rows);
	//std::cout << "newPointCloud.cols= " << newPointCloud.cols << std::endl;

	//int res = pcl::io::loadPCDFile("E:\\Studyproj\\24Proj\\meinong\\scene0421\\jvyuan_jvli\\100cm\\cloud_Gray.pcd", blob, cloud.sensor_origin_, cloud.sensor_orientation_, pcd_version, 0);
	// Convert PointCloud from PointXYZRGB to PointXYZ
}


void stereo_sgm::performBF(cv::Mat& pointCloud, cv::Mat& image) {

	//PointCloud<PointXYZ>::Ptr cloud(new PointCloud<PointXYZ>);
	//PointCloud<PointXYZ>::Ptr cloud_filtered(new PointCloud<PointXYZ>);

	//for (int i = 0; i < pointCloud.rows; i++) {
	//	for (int j = 0; j < pointCloud.cols; j++) {
	//		const cv::Vec3f& pt = pointCloud.at<cv::Vec3f>(i, j);
	//		if (pt == cv::Vec3f(0, 0, 0)) continue;
	//		//pcl::PointXYZRGB point;
	//		pcl::PointXYZ point;
	//		point.x = pt[0];
	//		point.y = pt[1];
	//		point.z = pt[2];
	//		//cv::Vec3b color = image.at<cv::Vec3b>(i, j);
	//		//uint32_t rgb = (static_cast<uint32_t>(color[2]) << 16 |
	//		//	static_cast<uint32_t>(color[1]) << 8 | static_cast<uint32_t>(color[0]));
	//		//point.rgb = *reinterpret_cast<float*>(&rgb);

	//		cloud->points.push_back(point);
	//	}
	//}
	//BilateralFilter<PointXYZ> bilateral_filter;
	//bilateral_filter.setInputCloud(cloud);

	//// 设置双边滤波的参数
	//bilateral_filter.setHalfSize(0.03);  // 设置滤波窗口的大小
	//bilateral_filter.setStdDev(0.05);    // 设置标准差

	//// 进行滤波处理
	//bilateral_filter.applyFilter(*cloud_filtered);

	//// 保存滤波后的点云数据
	//io::savePCDFile("filtered_output.pcd", *cloud_filtered);

	//cout << "Filtered point cloud data saved to filtered_output.pcd" << endl;

}

void stereo_sgm::performGF(cv::Mat& pointCloud, cv::Mat& image) {

	PointCloud<PointXYZ>::Ptr cloud(new PointCloud<PointXYZ>);
	PointCloud<PointXYZ>::Ptr cloud_filtered(new PointCloud<PointXYZ>);

	for (int i = 0; i < pointCloud.rows; i++) {
		for (int j = 0; j < pointCloud.cols; j++) {
			const cv::Vec3f& pt = pointCloud.at<cv::Vec3f>(i, j);
			if (pt == cv::Vec3f(0, 0, 0)) continue;
			//pcl::PointXYZRGB point;
			pcl::PointXYZ point;
			point.x = pt[0];
			point.y = pt[1];
			point.z = pt[2];
			//cv::Vec3b color = image.at<cv::Vec3b>(i, j);
			//uint32_t rgb = (static_cast<uint32_t>(color[2]) << 16 |
			//	static_cast<uint32_t>(color[1]) << 8 | static_cast<uint32_t>(color[0]));
			//point.rgb = *reinterpret_cast<float*>(&rgb);

			cloud->points.push_back(point);
		}
	}
	cloud->width = pointCloud.cols;
	cloud->height = pointCloud.rows;
	cloud->is_dense = false;
	// 打印点云数据的信息
	cout << "Loaded " << cloud->width * cloud->height << " data points from input.pcd" << endl;


	//// 生成随机点云
	//cloud->width = 100;
	//cloud->height = 10;
	//cloud->is_dense = false;
	//cloud->points.resize(cloud->width * cloud->height);

	//for (auto& point : cloud->points)
	//{
	//	point.x = 1024 * rand() / (RAND_MAX + 1.0f);
	//	point.y = 1024 * rand() / (RAND_MAX + 1.0f);
	//	point.z = 1024 * rand() / (RAND_MAX + 1.0f);
	//}

	//// 保存随机生成的点云
	//pcl::io::savePCDFileASCII("random_cloud.pcd", *cloud);

	// 创建 GaussianKernel 对象并设置参数
	pcl::filters::GaussianKernel<PointXYZ, PointXYZ> gaussian_kernel;
	gaussian_kernel.setSigma(6); // 设置高斯核的标准差
	gaussian_kernel.setThresholdRelativeToSigma(4); // 设置相对标准差的阈值

	//pcl::search::KdTree<pcl::PointXYZ>::Ptr kdtree(new pcl::search::KdTree<pcl::PointXYZ>);
	//(*kdtree).setInputCloud(cloud);
	//std::cout << "KdTree made" << std::endl;

	// 创建 Convolution3D 对象
	pcl::filters::Convolution3D<PointXYZ, PointXYZ, pcl::filters::GaussianKernel<PointXYZ, PointXYZ>> convolution;
	convolution.setInputCloud(cloud);
	convolution.setKernel(gaussian_kernel);
	//convolution.setSearchMethod(kdtree);
	convolution.setRadiusSearch(0.3);
	convolution.setNumberOfThreads(10);//important! Set Thread number for openMP
	// 进行滤波处理
	std::cout << "Convolution Start" << std::endl;
	convolution.convolve(*cloud_filtered);
	std::cout << "Convoluted" << std::endl;

	// 保存滤波后的点云数据
	io::savePCDFileASCII("GF_output.pcd", *cloud_filtered);

	cout << "Filtered point cloud data saved to filtered_output.pcd" << endl;

}