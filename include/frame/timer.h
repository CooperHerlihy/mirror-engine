#pragma once

#include <chrono>
#include <print>

#include "frame/types.h"

namespace Mirror {

struct Timer {
	std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();

	void start() noexcept {
		begin = std::chrono::high_resolution_clock::now();
	}
	
	void stop(const char* message) const noexcept {
		auto end = std::chrono::high_resolution_clock::now();
		std::println("{}: {}ms", message, (f64)(end - begin).count() / 1'000'000.0);
	}
};

}
