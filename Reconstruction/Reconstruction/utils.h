//2019/3/5
//utils.h��Ҫ�����ؽ�����ȡ��ά����Ⱥ���ɫ��Ϣ����ؽ�

#pragma once
#include "process_points.h"
#include "camera.h"
#include <thread>
#include <FreeImage/FreeImage.h>
#include <direct.h>
#include <io.h>

// �鿴�ؽ�Ч����Ҫ�޸ĵ�һЩ����
string shape = "e_shape1";
bool is_Interpolation = true;
bool is_Screenshot = false;
bool is_SavePointsToTxt = false;
const int insert_num = 3;// ��ֵ����
glm::vec3 point_scale(0.1f, 0.1f, 0.1f); // �Ե��������

Mat get_rgbMat(int num) {
	stringstream ss;
	ss << num;
	string num_str = ss.str();
	string rgb_path = "./data/" + shape +"/rgb/" + num_str + ".png";
	Mat img = imread(rgb_path); //ԭʼrgbͼ����ȡ��ɫ��Ϣ
	return img;
	//namedWindow("window");
	//imshow("window", img);
}
// һ����ʼ����ԭʼ���ݣ���ȡλ�ú���ɫ
void get_3D_param(glm::mat4 inverse_projection, int num, vector<glm::vec3>& m_pos, vector<glm::vec3>& m_color) {
	//vector<point_3D> frame_points;
	stringstream ss;
	ss << num;
	string num_str = ss.str();
	string rgb_path = "./data/" + shape + "/rgb/" + num_str + ".png";
	string depth_path = "./data/" + shape + "/depth_L/" + num_str + ".png";
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
			// ��ȡrgb��ֵ
			int b = img.at<Vec3b>(i, j)[0];
			int g = img.at<Vec3b>(i, j)[1];
			int r = img.at<Vec3b>(i, j)[2];	
			if (g > 0.0 && r > 0.0 && b > 0.0 && depth > 30) {
				double w_clip = -depth / 25.5; //��-z_e��
				// ��x, y, z�任����-1.0��1.0), opencv�е�ԭ�����Ͻǣ�opengl�е�ͼ��ԭ�������½ǣ���Ҫ���жԳƱ任
				// float x_ndc = (float)(cols - 1 - j) / (float)cols * 2.0 - 1.0; 
				double x_ndc = (double)(j) / (double)cols * 2.0 - 1.0;
				double y_ndc = (double)(i) / (double)rows * 2.0 - 1.0;
				double z_ndc = depth / 255.0 * 2.0 - 1.0;
				// ��ȡ�����ɫ
				tmp_color = glm::vec3(r / 255.0, g / 255.0, b / 255.0);
				// ������λ��
				glm::vec4 p_c = glm::vec4(x_ndc * w_clip, y_ndc * w_clip, z_ndc * w_clip, w_clip);//�ü�����ϵ�µ�������꣨w_c = -z_e��
				glm::vec4 p_e = inverse_projection * p_c;
				tmp_pos = glm::vec3(p_e); //ȡ��x, y, z��Ϊ��ǰ���λ��
				// cout << g << " " << b << " " << r << " " << depth << endl;
				m_pos.push_back(tmp_pos);
				m_color.push_back(tmp_color);
			}
		}
	}
}

// ��ֵÿһ֡
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

// ������ȡ����֡�Ĳ�ֵ��
void Interpolation(vector<ProcessPoints>& frames_pps, 
	vector<vector<glm::vec3>>& insert_pos, vector<vector<glm::vec3>>& insert_color) {
	int total_frames = frames_pps.size();
	for (int i = 0; i < total_frames; ++i) {
		cout << "��ֵ��:" << i << "/" << total_frames << endl;
		vector<vector<int>> insert_idx = frames_pps[i].insert_idx(); // ��ȡ��ǰ֡�Ĳ�ֵ������
		vector<glm::vec3> frame_pos = frames_pps[i].frame_pos();
		vector<glm::vec3> frame_color = frames_pps[i].frame_color();
		//��ȡ��ֵ��λ��
		vector<glm::vec3> frame_insert_pos = Interpolation(insert_idx, frame_pos);
		insert_pos.push_back(frame_insert_pos);
		//��ȡ��ֵ����ɫ
		vector<glm::vec3> frame_insert_color = Interpolation(insert_idx, frame_color);
		insert_color.push_back(frame_insert_color);
	}
}

// ����������Ϣ�������ļ�
void SavePointsToTxt(vector<ProcessPoints>& frames_pps, vector<vector<glm::vec3>>& insert_pos, vector<vector<glm::vec3>>& insert_color, int start_idx) {

	int n = frames_pps.size(); // ���ж���֡
	for (int i = 0; i < n; ++i) {
		cout << "д���ļ��У�" << i << "/" << n << endl;
		string filename = "points_txt/" + shape + "/frame_" + to_string(i + start_idx) + ".txt";
		ofstream fout(filename.c_str(), ios::trunc); // �������Ϣд���ļ�, �Ƚ��ļ����
		if (!fout) {
			cout << "�ļ���ʧ��" << endl;
		}
		// ��ȡ��ǰ֡��λ�ú���ɫ
		ProcessPoints pps0 = frames_pps[i];
		vector<glm::vec3> pos0 = pps0.frame_pos(); 
		vector<glm::vec3> color0 = pps0.frame_color();
		// ��ȡ��ǰ֡��ֵ���ֵ�λ�ú���ɫ
		vector<glm::vec3> insert_pos0;
		vector<glm::vec3> insert_color0;
		if (insert_pos.size() > 0) {
			insert_pos0 = insert_pos[i];
			insert_color0 = insert_color[i];
		}
		// �����ļ���д���ܹ��������
		int point_amount = pos0.size() + insert_pos0.size();
		fout << point_amount << endl;
		// ���ļ���д��ԭʼ��
		for (int j = 0; j < pos0.size(); ++j) {
			glm::vec3 p = pos0[j], c = color0[j];
			fout << p[0] << " " << p[1] << " " << p[2] << " " << c[0] << " " << c[1] << " " << c[2] <<endl;
		}
		// ���ļ���д���ֵ��
		for (int j = 0; j < insert_pos0.size(); ++j) {
			glm::vec3 p = insert_pos0[j], c = insert_color0[j];
			fout << p[0] << " " << p[1] << " " << p[2] << " " << c[0] << " " << c[1] << " " << c[2] << endl;
		}
		fout.close();
	}
	cout << "д�ļ����" << endl;
	return;
}

// �ġ��γ�buffer����
void GenBufferData(vector<ProcessPoints>& frames_pps, vector<vector<glm::vec3>>& insert_pos, vector<vector<glm::vec3>>& insert_color,
	vector<glm::mat4*>& frames_modelMatrices, vector<glm::vec3*>& frames_colors, vector<int>& frames_amount) {
	int total_frames = frames_pps.size();
	for (int i = 0; i < frames_pps.size(); ++i) {
		cout << "�γ�buffer��" << i << "/" << total_frames << endl;
		// ��ȡ��һ֡��ԭʼ����
		ProcessPoints pps0 = frames_pps[i];
		vector<glm::vec3> pos0 = pps0.frame_pos(); // ��ȡ��ǰ֡ȫ��λ��
		vector<glm::vec3> color0 = pps0.frame_color(); // ��ȡ��ǰ֡ȫ����ɫ
		// ��ȡ��һ֡��ֵ����
		vector<glm::vec3> insert_pos0;
		vector<glm::vec3> insert_color0;
		if (insert_pos.size() > 0) {
			insert_pos0 = insert_pos[i];
			insert_color0 = insert_color[i];
		}
		// insert_pos0.clear(); // �Ƿ���Ҫ��ֵ
		// �ܹ���Ҫʵ����������
		int amount = pos0.size() + insert_pos0.size();
		glm::mat4* modelMatrices = new glm::mat4[amount];
		glm::vec3* colors = new glm::vec3[amount];
		// ԭʼ����
		for (int i = 0; i < pos0.size(); i++) {
			// λ����Ϣ
			glm::mat4 tmp_model;
			tmp_model = glm::translate(tmp_model, pos0[i]);
			tmp_model = glm::scale(tmp_model, point_scale);
			modelMatrices[i] = tmp_model;
			// ��ɫ��Ϣ
			colors[i] = color0[i];
		}
		// ��ֵ����
		for (int i = pos0.size(); i < amount; ++i) {
			int j = i - pos0.size();
			// ��ֵλ����Ϣ
			glm::mat4 tmp_model;
			tmp_model = glm::translate(tmp_model, insert_pos0[j]);
			tmp_model = glm::scale(tmp_model, point_scale);
			modelMatrices[i] = tmp_model;
			// ��ֵ��ɫ��Ϣ
			colors[i] = insert_color0[j];
		}
		// ����
		frames_modelMatrices.push_back(modelMatrices);
		frames_colors.push_back(colors);
		frames_amount.push_back(amount);
	}
}

// �塢��ͼ����ÿһ֡
void SaveScreenshot(int idx, int width = 900, int height = 720) {
	stringstream ss;
	ss << idx;
	string dir_path = "Screenshot/" + shape;
	if (_access(dir_path.c_str(), 0) == -1) {
		int isSuccess = _mkdir(dir_path.c_str());
		if (isSuccess == -1) cout << "δ�ܳɹ�����" << dir_path << endl;
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
		cout << "����ʧ��" << endl;
	FreeImage_Unload(bitmap);
	delete[] mpixels;
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
float total_angle = PI / 12;
float cur_angle = 0.0;
float flag = 1.0;
void updateCamera(Camera& camera) {
	// ���ʵʱ��ת
	glm::vec3 center = glm::vec3(0.0, 0.0, 5.0); // ������ת����
	
	//float theta = 0.0; //����ת
	float theta = 0.04; // ֮ǰ�ǲ鿴45��Ч��
	cur_angle += flag * theta;
	if (cur_angle > total_angle || cur_angle < -total_angle) {
		flag = -1.0 * flag;
	}
	
	camera.Position = getCameraRotatePoint(camera.Position, flag*theta, center);
	// camera.Yaw -= theta;
	// ����lookat���������������
	camera.Front = glm::normalize(center - camera.Position);
	camera.Right = glm::normalize(glm::cross(camera.Front, camera.WorldUp));
	camera.Up = glm::normalize(glm::cross(camera.Right, camera.Front));
}
