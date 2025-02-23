#pragma once

#include "frame/frame.h"
#include "reflect/renderer.h"

namespace Mirror {

class Mirror {
public:
	Mirror(const Vec2<i32> window_size, const std::string_view window_name) : 
		renderer_(window_size, window_name) {}

	void update() {
		renderer_.update();
	}

private:
	Reflect::Renderer renderer_;
};

}

