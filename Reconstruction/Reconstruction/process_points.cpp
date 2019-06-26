#include "process_points.h"

void ProcessPoints::PointsInsertIdx() {
	kdt::KDTree<MyPoint> kdtree(my_points_pos_); // 构建kdtree 
	int num_points = my_points_pos_.size();
	vector<bool> visited(num_points, false);
	// 遍历所有点
	for (size_t i = 0; i < num_points; ++i) {
		vector<int> insert_idx;
		MyPoint query = my_points_pos_[i];
		// radius search
		vector<int> radius_idx = kdtree.radiusSearch(query, Radius);
		// 最近点索引
		vector<int> knn_idx = kdtree.knnSearch(query, 2);
		int nn_idx = knn_idx[1];
		//double minDistance = TwoPosDistance(glm_points_pos_[nn_idx], glm_points_pos_[i]);
		//cout << "minDistance:" << minDistance << endl; // 最近点距离大概0.05
		for (auto idx : radius_idx) {
			// 当前点已经被插过值
			if (idx == i || visited[idx] == true) continue;
			// TODO 可计算满足的角度条件
			glm::dvec3 v1 = glm_points_pos_[nn_idx] - glm_points_pos_[i];
			glm::dvec3 v2 = glm_points_pos_[idx] - glm_points_pos_[i];
			double angle = TwoVecAngle(v1, v2);
			if (angle > MinAngle && angle < MaxAngle) {
				insert_idx.push_back(idx);
			}
		}
		// 处理完当前点
		visited[i] = true;
		points_insert_idx_.push_back(insert_idx);
	}
}

void ProcessPoints::EasyPointsInsertIdx() {
	kdt::KDTree<MyPoint> kdtree(my_points_pos_);
	int num_points = my_points_pos_.size();
	vector<bool> visited(num_points, false);
	for (size_t i = 0; i < num_points; ++i) {
		vector<int> insert_idx;
		MyPoint query = my_points_pos_[i];
		// vector<int> knn_idx = kdtree.knnSearch(query, 10);
		// double r = 0.048;
		vector<int> knn_idx = kdtree.radiusSearch(query, Radius);
		if(knn_idx.size() > 5)
			knn_idx.erase(knn_idx.begin(), knn_idx.begin() + 5);
		/*
		for (auto idx : knn_idx) {
			if (idx == i || visited[idx] == true) continue;
			insert_idx.push_back(idx);
		}
		visited[i] = true;
		*/
		points_insert_idx_.push_back(knn_idx);
	}
}

void ProcessPoints::InitData() {
	for (auto pos : glm_points_pos_) {
		my_points_pos_.push_back(vec2myp(pos)); //glm::vec3->MyPoint
	}
}

vector<int> ProcessPoints::AngleSearch(const vector<point_3D>& frame_points, int point_idx, vector<int>& point_radius_idx) {
	glm::vec3 cur_pos = frame_points[point_idx].pos;	
	vector<int> vector_idx;
	int size = point_radius_idx.size();
	for (int i = 0; i < size; ++i) {
		glm::vec3 p1 = frame_points[min_distance_idx_].pos, p2 = frame_points[i].pos;
		// 生成向量
		glm::vec3 v1 = p1 - cur_pos, v2 = p2 - cur_pos;
		// 计算投影后的向量夹角
		float angle = TwoVecAngle(v1, v2);
		if (MinAngle < angle && angle < MaxAngle) {
			vector_idx.push_back(i);
		}
	}
	return vector_idx;
}

double ProcessPoints::TwoPosDistance(const glm::vec3& p1, const glm::vec3& p2) {
	glm::dvec3 p = p1 - p2;
	return sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
}
double ProcessPoints::TwoVecAngle(const glm::dvec3& v1, const glm::dvec3& v2) {
	glm::dvec2 a = glm::vec2(v1.y, v1.z), b = glm::vec2(v2.y, v2.z); //投影成二维向量
	double abs_a = sqrt(a.x * a.x + a.y * a.y), abs_b = sqrt(b.x * b.x + b.y * b.y);
	double a_b = a.x * b.x + a.y * b.y;
	return acos(a_b / abs_a / abs_b) / (double)PI * (double)180.0;
}