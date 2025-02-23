#pragma once

#include "types.h"

#include <chrono>

namespace Mirror {

struct Clock {
	std::chrono::high_resolution_clock::time_point previous = std::chrono::high_resolution_clock::now();
	std::chrono::nanoseconds delta{ 0 };

	void update() noexcept {
		auto now = std::chrono::high_resolution_clock::now();
		delta = now - previous;
		previous = now;	
	}

	[[nodiscard]] constexpr f64 deltaSec() const noexcept {
		return (f64)delta.count() / 1'000'000'000.0;
	}
};

}
