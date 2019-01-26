//2019/1/25
//理解_ndc和_clip,实现变换关系

#pragma once
#include <iostream>
using namespace std;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include<opencv2/core/core.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/highgui/highgui.hpp>
using namespace cv;

vector<vector<float>> get_3D_param() {
	vector<vector<float>> ans;
	Mat img = imread("./data/rgb.png"); //原始rgb图，获取颜色信息
	//Mat dpt = imread("./data/dpt.png"); //深度图的可视化效果
	namedWindow("window");
	imshow("window", img);
	Mat dpt_use = imread("./data/dpt_use.png"); //获取深度数据
	cvtColor(dpt_use, dpt_use, CV_RGB2GRAY);
	int channel = img.channels();
	int rows = img.rows;
	int cols = img.cols;
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			vector<float> tmp;
			// 在深度图中获取深度
			int depth = dpt_use.at<uchar>(i, j);
			float w_clip = -depth / 25.5;
			// 获取rgb的值
			int b = img.at<Vec3b>(i, j)[0];
			int g = img.at<Vec3b>(i, j)[1];
			int r = img.at<Vec3b>(i, j)[2];
			//将x, y, z变换到（-1.0，1.0), opencv中的原点左上角，opengl中的图像原点在左下角，需要进行对称变换
			float x_ndc = (float)(cols - 1 - j) / (float)cols * 2.0 - 1.0; 
			float y_ndc = (float)(i) / (float)rows * 2.0 - 1.0;
			float z_ndc = depth / 255.0 * 2.0 - 1.0;
			if (g > 0 && r > 0 && b > 0) {
				tmp.push_back(r / 255.0);
				tmp.push_back(g / 255.0);
				tmp.push_back(b / 255.0);
				tmp.push_back(w_clip * x_ndc); // 放入x_clip  
				tmp.push_back(w_clip * y_ndc);
				tmp.push_back(w_clip * z_ndc);
				ans.push_back(tmp);
				//cout << g << " " << b << " " << r << " " << depth << endl;
			}
		}
	}
	return ans;
}
