#pragma once
#include "vector.h"

struct matrix3x4_t {
	float m[3][4];

	vector3 origin() const {
		return { m[0][3], m[1][3], m[2][3] };
	}
};

struct view_matrix_t {
	float m[4][4];

	// world to screen projection
	bool world_to_screen(const vector3& world, vector3& screen, int screen_w, int screen_h) const;
};