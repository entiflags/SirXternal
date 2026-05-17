#pragma once

struct vector3 {
	float x, y, z;

	vector3() :x(0), y(0), z(0) {}
	vector3(float x, float y, float z) : x(x), y(y), z(z) {}

	vector3 operator+(const vector3& other) const {
		return { x + other.x, y + other.y, z + other.z };
	}

	vector3 operator-(const vector3& other) const {
		return { x - other.x, y - other.y, z - other.z };
	}

	vector3 operator*(float scalar) const {
		return { x * scalar, y * scalar, z * scalar };
	}

	float length() const;
	float length_2d() const;
	vector3 normalized() const;
	float dot(const vector3& other) const;
};

using qangle = vector3;