#pragma once

#include <exception>
#include <format>
#include <iostream>
#include <print>

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
