//2019/3/5
//utils.h主要用来重建，读取二维、深度和颜色信息完成重建

#pragma once
#include "process_points.h"
#include "camera.h"

Mat get_rgbMat(int num) {
	stringstream ss;
	ss << num;
	string num_str = ss.str();
	string rgb_path = "./data/rgb/" + num_str + ".png";
	Mat img = imread(rgb_path); //原始rgb图，获取颜色信息
	return img;
	//namedWindow("window");
	//imshow("window", img);
}

MyPoint vec2myp(glm::vec3 v) { return MyPoint(v.x, v.y, v.z); }

void get_3D_param(glm::mat4 inverse_projection, int num, vector<MyPoint>& m_pos, vector<MyPoint>& m_color) {
	//vector<point_3D> frame_points;
	stringstream ss;
	ss << num;
	string num_str = ss.str();
	string rgb_path = "./data/rgb/" + num_str + ".png";
	string depth_path = "./data/depth/" + num_str + ".png";
	Mat img = imread(rgb_path); // 原始rgb图，获取颜色信息

	Mat dpt_use = imread(depth_path); // 获取深度数据
	cvtColor(dpt_use, dpt_use, CV_RGB2GRAY);
	int channel = img.channels();
	int rows = img.rows;
	int cols = img.cols;

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			glm::vec3 tmp_pos;
			glm::vec3 tmp_color;
			// 在深度图中获取深度
			int depth = dpt_use.at<uchar>(i, j);
			double w_clip = -depth / 25.5; //（-z_e）
			// 获取rgb的值
			int b = img.at<Vec3b>(i, j)[0];
			int g = img.at<Vec3b>(i, j)[1];
			int r = img.at<Vec3b>(i, j)[2];
			// 将x, y, z变换到（-1.0，1.0), opencv中的原点左上角，opengl中的图像原点在左下角，需要进行对称变换
			// float x_ndc = (float)(cols - 1 - j) / (float)cols * 2.0 - 1.0; 
			double x_ndc = (double)(j) / (double)cols * 2.0 - 1.0;
			double y_ndc = (double)(i) / (double)rows * 2.0 - 1.0;
			double z_ndc = depth / 255.0 * 2.0 - 1.0;
			if (g > 0.0 && r > 0.0 && b > 0.0) {
				// 获取点的颜色
				tmp_color = glm::vec3(r / 255.0, g / 255.0, b / 255.0);
				// 计算点的位置
				glm::vec4 p_c = glm::vec4(x_ndc * w_clip, y_ndc * w_clip, z_ndc * w_clip, w_clip);//裁剪坐标系下的齐次坐标（w_c = -z_e）
				glm::vec4 p_e = inverse_projection * p_c;
				tmp_pos = glm::vec3(p_e); //取出x, y, z作为当前点的位置
				// cout << g << " " << b << " " << r << " " << depth << endl;
				m_pos.push_back(vec2myp(tmp_pos));
				m_color.push_back(vec2myp(tmp_color));
			}
		}
	}
}

//相机围绕中心的旋转获取新的位置
glm::vec3 getCameraRotatePoint(glm::vec3 camPos, float theta, glm::vec3 center) {
	float x = camPos.x, z = camPos.z;
	// 围绕中心点旋转，仅旋转x、z即可, 旋转PI
	float x1 = (x - center.x) * cos(theta) - (z - center.z) * sin(theta) + center.x;
	float z1 = (x - center.x) * sin(theta) + (z - center.z) * cos(theta) + center.z;
	return glm::vec3(x1, camPos.y, z1);
}

// 更新相机位置
void updateCamera(Camera& camera) {
	// 相机实时旋转
	glm::vec3 center = glm::vec3(0.0, 0.0, 5.0); // 设置旋转中心
	float theta = 0.1; //实时旋转的角度
	camera.Position = getCameraRotatePoint(camera.Position, theta, center);
	// camera.Yaw -= theta;
	// 根据lookat更新相机的三个轴
	camera.Front = glm::normalize(center - camera.Position);
	camera.Right = glm::normalize(glm::cross(camera.Front, camera.WorldUp));
	camera.Up = glm::normalize(glm::cross(camera.Right, camera.Front));
}


