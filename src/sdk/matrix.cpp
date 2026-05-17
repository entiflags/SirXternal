#include "sdk/matrix.h"

bool view_matrix_t::world_to_screen(const vector3& world, vector3& screen, int screen_w, int screen_h) const {
    float w = m[3][0] * world.x + m[3][1] * world.y + m[3][2] * world.z + m[3][3];
    if (w < 0.001f)
        return false;

    float inv_w = 1.0f / w;
    float sx = m[0][0] * world.x + m[0][1] * world.y + m[0][2] * world.z + m[0][3];
    float sy = m[1][0] * world.x + m[1][1] * world.y + m[1][2] * world.z + m[1][3];

    screen.x = (screen_w * 0.5f) + (sx * inv_w) * (screen_w * 0.5f);
    screen.y = (screen_h * 0.5f) - (sy * inv_w) * (screen_h * 0.5f);
    screen.z = 0.0f;
    return true;
}
