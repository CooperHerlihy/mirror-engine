#pragma once

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
