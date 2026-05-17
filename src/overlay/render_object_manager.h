#pragma once
#include "render_object.h"
#include <vector>
#include <memory>

class render_object_manager {
public:
	void add(std::shared_ptr<render_object> obj) {
		objects_.push_back(std::move(obj));
	}

	void render_all(ImVec2 display_size, ImVec2 cursor_pos, bool mouse_released) {
		for (auto& obj : objects_)
			obj->render(display_size, cursor_pos, mouse_released);
	}

	void clear() { objects_.clear(); }

private:
	std::vector<std::shared_ptr<render_object>> objects_;
};