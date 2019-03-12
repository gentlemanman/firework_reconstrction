//2019/3/5
//utils.h��Ҫ�����ؽ�����ȡ��ά����Ⱥ���ɫ��Ϣ����ؽ�

#pragma once
#include "process_points.h"
#include "camera.h"

Mat get_rgbMat(int num) {
	stringstream ss;
	ss << num;
	string num_str = ss.str();
	string rgb_path = "./data/rgb/" + num_str + ".png";
	Mat img = imread(rgb_path); //ԭʼrgbͼ����ȡ��ɫ��Ϣ
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
	Mat img = imread(rgb_path); // ԭʼrgbͼ����ȡ��ɫ��Ϣ

	Mat dpt_use = imread(depth_path); // ��ȡ�������
	cvtColor(dpt_use, dpt_use, CV_RGB2GRAY);
	int channel = img.channels();
	int rows = img.rows;
	int cols = img.cols;

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			glm::vec3 tmp_pos;
			glm::vec3 tmp_color;
			// �����ͼ�л�ȡ���
			int depth = dpt_use.at<uchar>(i, j);
			double w_clip = -depth / 25.5; //��-z_e��
			// ��ȡrgb��ֵ
			int b = img.at<Vec3b>(i, j)[0];
			int g = img.at<Vec3b>(i, j)[1];
			int r = img.at<Vec3b>(i, j)[2];
			// ��x, y, z�任����-1.0��1.0), opencv�е�ԭ�����Ͻǣ�opengl�е�ͼ��ԭ�������½ǣ���Ҫ���жԳƱ任
			// float x_ndc = (float)(cols - 1 - j) / (float)cols * 2.0 - 1.0; 
			double x_ndc = (double)(j) / (double)cols * 2.0 - 1.0;
			double y_ndc = (double)(i) / (double)rows * 2.0 - 1.0;
			double z_ndc = depth / 255.0 * 2.0 - 1.0;
			if (g > 0.0 && r > 0.0 && b > 0.0) {
				// ��ȡ�����ɫ
				tmp_color = glm::vec3(r / 255.0, g / 255.0, b / 255.0);
				// ������λ��
				glm::vec4 p_c = glm::vec4(x_ndc * w_clip, y_ndc * w_clip, z_ndc * w_clip, w_clip);//�ü�����ϵ�µ�������꣨w_c = -z_e��
				glm::vec4 p_e = inverse_projection * p_c;
				tmp_pos = glm::vec3(p_e); //ȡ��x, y, z��Ϊ��ǰ���λ��
				// cout << g << " " << b << " " << r << " " << depth << endl;
				m_pos.push_back(vec2myp(tmp_pos));
				m_color.push_back(vec2myp(tmp_color));
			}
		}
	}
}

//���Χ�����ĵ���ת��ȡ�µ�λ��
glm::vec3 getCameraRotatePoint(glm::vec3 camPos, float theta, glm::vec3 center) {
	float x = camPos.x, z = camPos.z;
	// Χ�����ĵ���ת������תx��z����, ��תPI
	float x1 = (x - center.x) * cos(theta) - (z - center.z) * sin(theta) + center.x;
	float z1 = (x - center.x) * sin(theta) + (z - center.z) * cos(theta) + center.z;
	return glm::vec3(x1, camPos.y, z1);
}

// �������λ��
void updateCamera(Camera& camera) {
	// ���ʵʱ��ת
	glm::vec3 center = glm::vec3(0.0, 0.0, 5.0); // ������ת����
	float theta = 0.1; //ʵʱ��ת�ĽǶ�
	camera.Position = getCameraRotatePoint(camera.Position, theta, center);
	// camera.Yaw -= theta;
	// ����lookat���������������
	camera.Front = glm::normalize(center - camera.Position);
	camera.Right = glm::normalize(glm::cross(camera.Front, camera.WorldUp));
	camera.Up = glm::normalize(glm::cross(camera.Right, camera.Front));
}


