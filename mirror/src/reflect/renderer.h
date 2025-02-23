#pragma once

#include "frame/frame.h"
#include "window.h"
#include "vk/vk_resources.h"

namespace Mirror::Reflect {

class Renderer {
public:
	Renderer(const Vec2<i32> window_size, const std::string_view window_name = "Mirror") :
		window(window_size, window_name) {};

	void update() {
		render();
	}

private:
	Window window;

	void render();
};

}
