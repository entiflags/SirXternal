#pragma once
#include <imgui.h>

class render_object {
public:
    virtual ~render_object() = default;
    virtual void render(ImVec2 display_size, ImVec2 cursor_pos, bool mouse_released) = 0;
};
