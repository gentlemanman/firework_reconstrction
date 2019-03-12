#include "process_points.h"

void ProcessPoints::PointsInsertIdx() {
	kdt::KDTree<MyPoint> kdtree(points_pos_); // 构建kdtree 
	int num_points = points_pos_.size();
	vector<bool> visited(num_points, false);
	// 遍历所有点
	for (size_t i = 0; i < num_points; ++i) {
		vector<int> insert_idx;
		MyPoint query = points_pos_[i];
		// radius search
		vector<int> radius_idx = kdtree.radiusSearch(query, radius_);

		for (auto idx : radius_idx) {
			if (visited[idx] == true) continue;
			insert_idx.push_back(idx);
		}
		// 处理完当前点
		visited[i] = true;
		points_insert_idx_.push_back(insert_idx);
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
		if (angle_min_ < angle && angle < angle_max_) {
			vector_idx.push_back(i);
		}
	}
	return vector_idx;
}

float ProcessPoints::TwoPosDistance(const glm::vec3& p1, const glm::vec3& p2) {
	glm::vec3 p = p1 - p2;
	return sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
}
float ProcessPoints::TwoVecAngle(const glm::vec3& v1, const glm::vec3& v2) {
	glm::vec2 a = glm::vec2(v1.y, v1.z), b = glm::vec2(v2.y, v2.z); //投影成二维向量
	float abs_a = sqrt(a.x * a.x + a.y * a.y), abs_b = sqrt(b.x * b.x + b.y * b.y);
	float a_b = a.x * b.x + a.y * b.y;
	return acos(a_b / abs_a / abs_b);
}