#pragma once

#include "frame/frame.h"
#include "reflect/reflect.h"

namespace Mirror {

struct Engine {
	Vk::FurnaceKeeper furnace_keeper;
	Reflect::Renderer renderer;

	Engine(const Vec2<i32> window_size, const std::string_view window_name, const std::string_view app_name = "Mirror Engine", const std::string_view app_version = "1.0.0") :
		furnace_keeper(app_name, app_version),
		renderer(window_name, window_size) {}

	void update() {
		renderer.update();
	}
};

}
