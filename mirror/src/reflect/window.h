#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

namespace Mirror::Reflect {

class Window {
public:
	Window(const Vec2<i32> size, const std::string_view name) {
		assert(size.x > 1 && size.y > 1);
		window_ = SDL_CreateWindow(name.data(), size.x, size.y, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
		if (window_ == nullptr) throw Error::SDL;
	};
	~Window() noexcept {
		if (window_) SDL_DestroyWindow(window_);
	}

	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
	constexpr Window(Window&& other) noexcept : window_(other.window_) {
		other.window_ = nullptr;
	}
	constexpr Window& operator=(Window&& other) noexcept {
		if (this == &other) return *this;
		window_ = other.window_;
		other.window_ = nullptr;
		return *this;
	}

	[[nodiscard]] constexpr SDL_Window* window() const noexcept {
		assert(window_ != nullptr);
		return window_;
	}
	[[nodiscard]] constexpr operator SDL_Window*() const noexcept {
		assert(window_ != nullptr);
		return window_;
	}

	[[nodiscard]] Vec2<i32> getSize() const {
		assert(window_ != nullptr);
		Vec2<i32> size;
		if (!SDL_GetWindowSize(window_, &size.x, &size.x)) throw Error::SDL;
		return size;
	}

private:
	SDL_Window* window_;
};

}

