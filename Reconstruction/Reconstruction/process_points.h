#pragma once
#define PI 3.141592653589793238462643383279502

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
class MyPoint : public std::array<double, 3>{
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

static double Radius = 0.05;
const double MinAngle = 30.0;
const double MaxAngle = 150.0;


class ProcessPoints {
public:
	ProcessPoints() {}
	ProcessPoints(vector<glm::vec3> points_pos, vector<glm::vec3> points_color, bool is_Interpolation) : glm_points_pos_(points_pos), points_color_(points_color){
		InitData();
		// 判断是否需要获取插值的索引
		if (is_Interpolation) {
			//PointsInsertIdx();
			EasyPointsInsertIdx();
		}
	}
	~ProcessPoints(){}
	vector<vector<int>> insert_idx() { return points_insert_idx_; }
	int points_num() { return glm_points_pos_.size(); }
	vector<glm::vec3> frame_pos() { return glm_points_pos_; }
	vector<glm::vec3> frame_color() { return points_color_; }

private:
	vector<MyPoint> my_points_pos_; // 每一帧的点位置信息
	vector<glm::vec3> glm_points_pos_;
	vector<glm::vec3> points_color_; // 每一帧的点颜色信息
	vector<vector<int>> points_insert_idx_; // 每一帧的点对应插值索引
	int min_distance_idx_ = 0; //记录一个据当前距离最小点的索引

	// 初始化操作，将数据格式转换成可用
	void InitData();
	// 计算一帧的插值点索引
	void PointsInsertIdx();
	void EasyPointsInsertIdx();
	// 在半径范围内的点进一步根据角度信息筛选，同时设置是否遍历
	vector<int> AngleSearch(const vector<point_3D>& frame_points, int point_idx, vector<int>& point_radius_idx);
	// 计算两个位置间的距离
	double TwoPosDistance(const glm::vec3& p1, const glm::vec3& p2);
	// 计算两个向量的夹角,返回角度值
	double TwoVecAngle(const glm::dvec3& v1, const glm::dvec3& v2);
	
	glm::vec3 myp2vec(MyPoint p) { return glm::vec3(p[0], p[1], p[2]); }
	MyPoint vec2myp(glm::vec3 v) { return MyPoint(v.x, v.y, v.z); }
};