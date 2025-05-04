#pragma once

#include <cstdint>
#include <exception>
#include <format>
#include <iostream>
#include <string_view>
#include <chrono>

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using iptr = intptr_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using uptr = uintptr_t;
using usize = size_t;

using f32 = float;
using f64 = double;

template <typename F> struct PrivateDefer {
	F f;
	PrivateDefer(F f) : f(f) {}
	~PrivateDefer() {
		f();
	}
};

template <typename F> PrivateDefer<F> defer_function(F f) {
	return PrivateDefer<F>(f);
}

#define DEFER_INTERMEDIATE_1(x, y) x##y
#define DEFER_INTERMEDIATE_2(x, y) DEFER_INTERMEDIATE_1(x, y)
#define DEFER_INTERMEDIATE_3(x) DEFER_INTERMEDIATE_2(x, __COUNTER__)
#define defer(code)                                                                                                    \
	auto DEFER_INTERMEDIATE_3(_defer_) = defer_function([&] {                                                          \
		code;                                                                                                          \
	})

#ifdef NDEBUG
#define debug_assert(condition) ((void)0)
#else
#define debug_assert(condition)                                                                                        \
	if (!(condition)) {                                                                                                \
		std::cerr << std::format("Failed debug assert: {}\n    File {}, Line {}\n", #condition, __FILE__, __LINE__);   \
		std::terminate();                                                                                              \
	}
#endif

#define release_assert(condition)                                                                                      \
	if (!(condition)) {                                                                                                \
		std::cerr << std::format("Failed assert: {}\n    File {}, Line {}\n", #condition, __FILE__, __LINE__);         \
		throw std::runtime_error{std::format("Failed assert: {}\n", #condition)};                                      \
	}

#define critical_assert(condition)                                                                                     \
	if (!(condition)) {                                                                                                \
		std::cerr << std::format("Failed critical assert: {}\n    File {}, Line {}\n", #condition, __FILE__,           \
								 __LINE__);                                                                            \
		std::abort();                                                                                                  \
	}

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
		std::cout << std::format("{}: {}ms", message, (f64)(end - m_begin).count() / 1'000'000.0);
	}

private:
	std::chrono::high_resolution_clock::time_point m_begin = std::chrono::high_resolution_clock::now();
};

} // namespace Mirror
