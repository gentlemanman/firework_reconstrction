#pragma once
#define PI 3.1415927f

#include <iostream>
#include <array>
#include <unordered_map>
#include <set>
#include <math.h>
using namespace std;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include<opencv2/core/core.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/highgui/highgui.hpp>
using namespace cv;

#include "kdtree.h"
class MyPoint : public std::array<double, 3>
{
public:

	// dimension of space (or "k" of k-d tree)
	// KDTree class accesses this member
	static const int DIM = 3;

	// the constructors
	MyPoint() {}
	MyPoint(double x, double y)
	{
		(*this)[0] = x;
		(*this)[1] = y;
	}
	MyPoint(double x, double y, double z)
	{
		(*this)[0] = x;
		(*this)[1] = y;
		(*this)[2] = z;
	}

	// conversion to OpenCV Point2d
	operator cv::Point2d() const { return cv::Point2d((*this)[0], (*this)[1]); }
	operator cv::Point3d() const { return cv::Point3d((*this)[0], (*this)[1], (*this)[2]); }
};

struct point_3D {
	glm::vec3 pos = glm::vec3(0.0, 0.0, 0.0);
	glm::vec3 color = glm::vec3(0.0, 0.0, 0.0);
	bool isValid = false;
};

class ProcessPoints {
public:
	ProcessPoints() {}
	ProcessPoints(vector<MyPoint> points_pos, vector<MyPoint> points_color) : points_pos_(points_pos), points_color_(points_color) {
		PointsInsertIdx();
	}
	~ProcessPoints(){}
	vector<vector<int>> insert_idx() { return points_insert_idx_; }
	int points_num() { return points_pos_.size(); }
	glm::vec3 pos(int i) { return myp2vec(points_pos_[i]); }
	glm::vec3 color(int i) { return myp2vec(points_color_[i]); }

private:
	vector<MyPoint> points_pos_; // ÿһ֡�ĵ�λ����Ϣ
	vector<MyPoint> points_color_; // ÿһ֡�ĵ���ɫ��Ϣ
	vector<vector<int>> points_insert_idx_; // ÿһ֡�ĵ��Ӧ��ֵ����
	const double radius_ = 0.2; // ��Ѱ�뾶

	int min_distance_idx_ = 0; //��¼һ���ݵ�ǰ������С�������
	const int num_insert = 5; // ��ֵ������
	
	const float angle_min_ = 30.0;
	const float angle_max_ = 150.0;

	// ����һ֡�Ĳ�ֵ������
	void PointsInsertIdx();
	// �ڰ뾶��Χ�ڵĵ��һ�����ݽǶ���Ϣɸѡ��ͬʱ�����Ƿ����
	vector<int> AngleSearch(const vector<point_3D>& frame_points, int point_idx, vector<int>& point_radius_idx);
	// ��������λ�ü�ľ���
	float TwoPosDistance(const glm::vec3& p1, const glm::vec3& p2);
	// �������������ļн�
	float TwoVecAngle(const glm::vec3& v1, const glm::vec3& v2);
	glm::vec3 myp2vec(MyPoint p) { return glm::vec3(p[0], p[1], p[2]); }
};