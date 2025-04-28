#pragma once

#include <chrono>
#include <print>
#include <string_view>

#include "frame/types.h"

namespace Mirror {

class Clock {
public:
	void update() noexcept {
		auto now = std::chrono::high_resolution_clock::now();
		m_delta = now - m_previous;
		m_previous = now;
	}

	[[nodiscard]] constexpr f64 delta_sec() const noexcept {
		return static_cast<f64>(m_delta.count()) / 1'000'000'000.0;
	}

private:
	std::chrono::high_resolution_clock::time_point m_previous = std::chrono::high_resolution_clock::now();
	std::chrono::nanoseconds m_delta{0};
};

class Timer {
public:
	void start() noexcept {
		m_begin = std::chrono::high_resolution_clock::now();
	}

	void stop(const std::string_view message) const noexcept {
		auto end = std::chrono::high_resolution_clock::now();
		std::println("{}: {}ms", message, (f64)(end - m_begin).count() / 1'000'000.0);
	}

private:
	std::chrono::high_resolution_clock::time_point m_begin = std::chrono::high_resolution_clock::now();
};

} // namespace Mirror
