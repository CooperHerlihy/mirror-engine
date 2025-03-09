#pragma once

#include <cstdint>
#include <expected>

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

namespace Mirror {

enum struct Err : u8 {
	Success,
	Unknown,
	NoResult,
	OutOfMemory,
	WindowTooSmall,
	CouldNotOpenFile,
	Timeout,
	Vulkan,
	SDL,
};

template<typename T>
using Result = std::expected<T, Err>;
using Error = std::unexpected<Err>;

}
