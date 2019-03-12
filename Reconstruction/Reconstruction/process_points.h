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
	vector<MyPoint> points_pos_; // 每一帧的点位置信息
	vector<MyPoint> points_color_; // 每一帧的点颜色信息
	vector<vector<int>> points_insert_idx_; // 每一帧的点对应插值索引
	const double radius_ = 0.2; // 搜寻半径

	int min_distance_idx_ = 0; //记录一个据当前距离最小点的索引
	const int num_insert = 5; // 插值的数量
	
	const float angle_min_ = 30.0;
	const float angle_max_ = 150.0;

	// 计算一帧的插值点索引
	void PointsInsertIdx();
	// 在半径范围内的点进一步根据角度信息筛选，同时设置是否遍历
	vector<int> AngleSearch(const vector<point_3D>& frame_points, int point_idx, vector<int>& point_radius_idx);
	// 计算两个位置间的距离
	float TwoPosDistance(const glm::vec3& p1, const glm::vec3& p2);
	// 计算两个向量的夹角
	float TwoVecAngle(const glm::vec3& v1, const glm::vec3& v2);
	glm::vec3 myp2vec(MyPoint p) { return glm::vec3(p[0], p[1], p[2]); }
};