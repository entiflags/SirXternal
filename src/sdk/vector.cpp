#include "sdk/vector.h"
#include <cmath>

float vector3::length() const {
    return sqrtf(x * x + y * y + z * z);
}

float vector3::length_2d() const {
    return sqrtf(x * x + y * y);
}

vector3 vector3::normalized() const {
    float len = length();
    if (len == 0.0f) return {};
    return { x / len, y / len, z / len };
}

float vector3::dot(const vector3& other) const {
    return x * other.x + y * other.y + z * other.z;
}
