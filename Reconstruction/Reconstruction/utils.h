//2019/3/5
//utils.h主要用来重建，读取二维、深度和颜色信息完成重建

#pragma once
#include "process_points.h"
#include "camera.h"
#include <thread>
#include <FreeImage/FreeImage.h>
#include <direct.h>
#include <io.h>

// 查看重建效果需要修改的一些参数
string shape = "e_shape1";
bool is_Interpolation = true;
bool is_Screenshot = false;
bool is_SavePointsToTxt = false;
const int insert_num = 3;// 插值点数
glm::vec3 point_scale(0.1f, 0.1f, 0.1f); // 对点进行缩放

Mat get_rgbMat(int num) {
	stringstream ss;
	ss << num;
	string num_str = ss.str();
	string rgb_path = "./data/" + shape +"/rgb/" + num_str + ".png";
	Mat img = imread(rgb_path); //原始rgb图，获取颜色信息
	return img;
	//namedWindow("window");
	//imshow("window", img);
}
// 一、开始处理原始数据，获取位置和颜色
void get_3D_param(glm::mat4 inverse_projection, int num, vector<glm::vec3>& m_pos, vector<glm::vec3>& m_color) {
	//vector<point_3D> frame_points;
	stringstream ss;
	ss << num;
	string num_str = ss.str();
	string rgb_path = "./data/" + shape + "/rgb/" + num_str + ".png";
	string depth_path = "./data/" + shape + "/depth_L/" + num_str + ".png";
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
			// 获取rgb的值
			int b = img.at<Vec3b>(i, j)[0];
			int g = img.at<Vec3b>(i, j)[1];
			int r = img.at<Vec3b>(i, j)[2];	
			if (g > 0.0 && r > 0.0 && b > 0.0 && depth > 30) {
				double w_clip = -depth / 25.5; //（-z_e）
				// 将x, y, z变换到（-1.0，1.0), opencv中的原点左上角，opengl中的图像原点在左下角，需要进行对称变换
				// float x_ndc = (float)(cols - 1 - j) / (float)cols * 2.0 - 1.0; 
				double x_ndc = (double)(j) / (double)cols * 2.0 - 1.0;
				double y_ndc = (double)(i) / (double)rows * 2.0 - 1.0;
				double z_ndc = depth / 255.0 * 2.0 - 1.0;
				// 获取点的颜色
				tmp_color = glm::vec3(r / 255.0, g / 255.0, b / 255.0);
				// 计算点的位置
				glm::vec4 p_c = glm::vec4(x_ndc * w_clip, y_ndc * w_clip, z_ndc * w_clip, w_clip);//裁剪坐标系下的齐次坐标（w_c = -z_e）
				glm::vec4 p_e = inverse_projection * p_c;
				tmp_pos = glm::vec3(p_e); //取出x, y, z作为当前点的位置
				// cout << g << " " << b << " " << r << " " << depth << endl;
				m_pos.push_back(tmp_pos);
				m_color.push_back(tmp_color);
			}
		}
	}
}

// 插值每一帧
vector<glm::vec3> Interpolation(vector<vector<int>>& insert_idx, vector<glm::vec3>& frame_pos) {
	int num = insert_idx.size();
	vector<glm::vec3> insert;
	for (int i = 0; i < num; ++i) {
		for (int j = 0; j < insert_idx[i].size(); ++j) {
			int idx = insert_idx[i][j];
			glm::vec3 cur_pos = frame_pos[i];
			glm::vec3 insert_pos = frame_pos[idx];
			glm::vec3 all = insert_pos - cur_pos;
			glm::vec3 pre = glm::vec3(all[0] / (insert_num + 1), all[1] / (insert_num + 1), all[2] / (insert_num + 1));
			for (int k = 1; k <= insert_num; k++) {
				glm::vec3 tmp = cur_pos + glm::vec3(pre[0] * k, pre[1] * k, pre[2] * k);
				insert.push_back(tmp);
			}
		}
	}
	return insert;
}

// 二、获取所有帧的插值点
void Interpolation(vector<ProcessPoints>& frames_pps, 
	vector<vector<glm::vec3>>& insert_pos, vector<vector<glm::vec3>>& insert_color) {
	int total_frames = frames_pps.size();
	for (int i = 0; i < total_frames; ++i) {
		cout << "插值中:" << i << "/" << total_frames << endl;
		vector<vector<int>> insert_idx = frames_pps[i].insert_idx(); // 获取当前帧的插值点索引
		vector<glm::vec3> frame_pos = frames_pps[i].frame_pos();
		vector<glm::vec3> frame_color = frames_pps[i].frame_color();
		//获取插值点位置
		vector<glm::vec3> frame_insert_pos = Interpolation(insert_idx, frame_pos);
		insert_pos.push_back(frame_insert_pos);
		//获取插值点颜色
		vector<glm::vec3> frame_insert_color = Interpolation(insert_idx, frame_color);
		insert_color.push_back(frame_insert_color);
	}
}

// 三、将点信息保存至文件
void SavePointsToTxt(vector<ProcessPoints>& frames_pps, vector<vector<glm::vec3>>& insert_pos, vector<vector<glm::vec3>>& insert_color, int start_idx) {

	int n = frames_pps.size(); // 共有多少帧
	for (int i = 0; i < n; ++i) {
		cout << "写入文件中：" << i << "/" << n << endl;
		string filename = "points_txt/" + shape + "/frame_" + to_string(i + start_idx) + ".txt";
		ofstream fout(filename.c_str(), ios::trunc); // 将点的信息写入文件, 先将文件清空
		if (!fout) {
			cout << "文件打开失败" << endl;
		}
		// 获取当前帧的位置和颜色
		ProcessPoints pps0 = frames_pps[i];
		vector<glm::vec3> pos0 = pps0.frame_pos(); 
		vector<glm::vec3> color0 = pps0.frame_color();
		// 获取当前帧插值部分的位置和颜色
		vector<glm::vec3> insert_pos0;
		vector<glm::vec3> insert_color0;
		if (insert_pos.size() > 0) {
			insert_pos0 = insert_pos[i];
			insert_color0 = insert_color[i];
		}
		// 先向文件中写入总共点的数量
		int point_amount = pos0.size() + insert_pos0.size();
		fout << point_amount << endl;
		// 向文件中写入原始点
		for (int j = 0; j < pos0.size(); ++j) {
			glm::vec3 p = pos0[j], c = color0[j];
			fout << p[0] << " " << p[1] << " " << p[2] << " " << c[0] << " " << c[1] << " " << c[2] <<endl;
		}
		// 向文件中写入插值点
		for (int j = 0; j < insert_pos0.size(); ++j) {
			glm::vec3 p = insert_pos0[j], c = insert_color0[j];
			fout << p[0] << " " << p[1] << " " << p[2] << " " << c[0] << " " << c[1] << " " << c[2] << endl;
		}
		fout.close();
	}
	cout << "写文件完成" << endl;
	return;
}

// 四、形成buffer数据
void GenBufferData(vector<ProcessPoints>& frames_pps, vector<vector<glm::vec3>>& insert_pos, vector<vector<glm::vec3>>& insert_color,
	vector<glm::mat4*>& frames_modelMatrices, vector<glm::vec3*>& frames_colors, vector<int>& frames_amount) {
	int total_frames = frames_pps.size();
	for (int i = 0; i < frames_pps.size(); ++i) {
		cout << "形成buffer：" << i << "/" << total_frames << endl;
		// 获取第一帧的原始数据
		ProcessPoints pps0 = frames_pps[i];
		vector<glm::vec3> pos0 = pps0.frame_pos(); // 获取当前帧全部位置
		vector<glm::vec3> color0 = pps0.frame_color(); // 获取当前帧全部颜色
		// 获取第一帧插值数据
		vector<glm::vec3> insert_pos0;
		vector<glm::vec3> insert_color0;
		if (insert_pos.size() > 0) {
			insert_pos0 = insert_pos[i];
			insert_color0 = insert_color[i];
		}
		// insert_pos0.clear(); // 是否需要插值
		// 总共需要实例化的数量
		int amount = pos0.size() + insert_pos0.size();
		glm::mat4* modelMatrices = new glm::mat4[amount];
		glm::vec3* colors = new glm::vec3[amount];
		// 原始数据
		for (int i = 0; i < pos0.size(); i++) {
			// 位置信息
			glm::mat4 tmp_model;
			tmp_model = glm::translate(tmp_model, pos0[i]);
			tmp_model = glm::scale(tmp_model, point_scale);
			modelMatrices[i] = tmp_model;
			// 颜色信息
			colors[i] = color0[i];
		}
		// 插值数据
		for (int i = pos0.size(); i < amount; ++i) {
			int j = i - pos0.size();
			// 插值位置信息
			glm::mat4 tmp_model;
			tmp_model = glm::translate(tmp_model, insert_pos0[j]);
			tmp_model = glm::scale(tmp_model, point_scale);
			modelMatrices[i] = tmp_model;
			// 插值颜色信息
			colors[i] = insert_color0[j];
		}
		// 保存
		frames_modelMatrices.push_back(modelMatrices);
		frames_colors.push_back(colors);
		frames_amount.push_back(amount);
	}
}

// 五、截图保存每一帧
void SaveScreenshot(int idx, int width = 900, int height = 720) {
	stringstream ss;
	ss << idx;
	string dir_path = "Screenshot/" + shape;
	if (_access(dir_path.c_str(), 0) == -1) {
		int isSuccess = _mkdir(dir_path.c_str());
		if (isSuccess == -1) cout << "未能成功创建" << dir_path << endl;
	}
	string file_path = dir_path + "/" + ss.str() + ".png";
	unsigned char *mpixels = new unsigned char[width * height * 4];
	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, mpixels);
	glReadBuffer(GL_BACK);
	for (int i = 0; i < (int)width * height * 4; i += 4) {
		mpixels[i] ^= mpixels[i + 2] ^= mpixels[i] ^= mpixels[i + 2];
	}
	FIBITMAP* bitmap = FreeImage_Allocate(width, height, 32, 8, 8, 8);
	for (int y = 0; y < FreeImage_GetHeight(bitmap); y++)
	{
		BYTE *bits = FreeImage_GetScanLine(bitmap, y);
		for (int x = 0; x < FreeImage_GetWidth(bitmap); x++)
		{
			bits[0] = mpixels[(y * width + x) * 4 + 0];
			bits[1] = mpixels[(y * width + x) * 4 + 1];
			bits[2] = mpixels[(y * width + x) * 4 + 2];
			bits[3] = 255;
			bits += 4;
		}
	}
	bool bSuccess = FreeImage_Save(FIF_PNG, bitmap, file_path.c_str(), PNG_Z_DEFAULT_COMPRESSION);
	if (bSuccess == false)
		cout << "保存失败" << endl;
	FreeImage_Unload(bitmap);
	delete[] mpixels;
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
float total_angle = PI / 12;
float cur_angle = 0.0;
float flag = 1.0;
void updateCamera(Camera& camera) {
	// 相机实时旋转
	glm::vec3 center = glm::vec3(0.0, 0.0, 5.0); // 设置旋转中心
	
	//float theta = 0.0; //不旋转
	float theta = 0.04; // 之前是查看45度效果
	cur_angle += flag * theta;
	if (cur_angle > total_angle || cur_angle < -total_angle) {
		flag = -1.0 * flag;
	}
	
	camera.Position = getCameraRotatePoint(camera.Position, flag*theta, center);
	// camera.Yaw -= theta;
	// 根据lookat更新相机的三个轴
	camera.Front = glm::normalize(center - camera.Position);
	camera.Right = glm::normalize(glm::cross(camera.Front, camera.WorldUp));
	camera.Up = glm::normalize(glm::cross(camera.Right, camera.Front));
}
